//---------------------------------------------------------------------------
#include <stdio.h>
#include <memory.h>
#include <iostream>
//---------------------------------------------------------------------------
#include "tcp_connect.h"
#include "appl/sync_console.h"
#include "server.h"
#include "protocol.h"
#include "cmd_record.h"
//---------------------------------------------------------------------------


mrpc::tcp_connect::tcp_connect(const std::string& astr_drv_id,
  boost::asio::ip::tcp::socket a_socket, t_server& a_server,
  mrpc::i_driver_rp_own& a_drv_rp_own)
  : m_str_drv_id(astr_drv_id),
    m_socket(std::move(a_socket)),
    m_server(a_server),
    mp_drv_rp(nullptr),
    m_drv_rp_own(a_drv_rp_own)
{
  m_p_read_obj.reset(new mrpc::protocol::t_read_obj());

  // отладка
  logout("size of msg_header: " + 
    std::to_string(sizeof(mrpc::protocol::t_msg_header)));
}
//---------------------------------------------------------------------------


mrpc::tcp_connect::~tcp_connect()
{
  logout("in destructor of tcp_connect");


  // пробежимся и "отпустим" все ожидающие задачи
  for(auto& curr : m_map_defer_tasks)
  {
    mrpc::t_payload payload;
    curr.second->set_promise(std::move(payload));
  }
}
//---------------------------------------------------------------------------


void mrpc::tcp_connect::set_drv_rp(mrpc::i_driver_rp* ap_drv_rp)
{
  mp_drv_rp = ap_drv_rp;
};
//---------------------------------------------------------------------------


int mrpc::tcp_connect::start()
{
  do_read();

  return 0;
}
//---------------------------------------------------------------------------


void mrpc::tcp_connect::tcp_connect::do_read()
{

  if(!m_p_read_obj) {
    log_err("buf is null");
    return;
  }


  // узнаем куда, сколько нужно читать:
  std::pair<uint8_t*, size_t> buf = m_p_read_obj->get_curr_data();


  //auto self(shared_from_this());
  // 
  //m_socket.async_read_some(boost::asio::buffer(m_read_buf, c_un_buf_length),

  m_socket.async_read_some(boost::asio::buffer(buf.first, buf.second),
 
    [this /*self*/](boost::system::error_code a_error, std::size_t a_readed)
    {
      if(a_error) {
        if(a_error == boost::asio::error::eof) {
          logout("<< seems that socket was shutdown normally");
        }
        else {
          log_err("error in asyn_read_some(). code: " + std::to_string(a_error.value()) + ", transl: " + a_error.message());
        };

        logout("sending request to del drv...");
        m_drv_rp_own.request_to_del_drv(this);

        return; 
      }
    

      if(!a_readed) {
        log_err("zero receied? something strange");
        m_drv_rp_own.request_to_del_drv(this);
        return;
      }


      // смещаем указатель
      m_p_read_obj->handle_readed(a_readed);

      bool f_is_ready = m_p_read_obj->is_ready();
      if (f_is_ready) {
        logout("read msg from net is ready");


        // проверим "протокол":
        if (m_p_read_obj->m_header.m_b_sign != mrpc::protocol::c_bSign) {
          log_err("Unexpected sign byte: " + std::to_string(m_p_read_obj->m_header.m_b_sign));

          //self.
          //delete this;
          // somehow close this connection

          // закроем сокет - по идее это приведёт к завершению операций
          m_socket.close();

          return;
        }

        handle_complete_msg();

        m_p_read_obj->reset();
      }


      // опять читаем:
      do_read();

      return;
      
    });
}
//---------------------------------------------------------------------------


void mrpc::tcp_connect::do_write()
{

  boost::asio::async_write(m_socket, boost::asio::buffer(&(m_write_buf[m_write_buf_pos]), m_write_buf.size() - m_write_buf_pos),    

    [this](boost::system::error_code a_error, std::size_t a_written)
    {
      if (a_error) {
        if (a_error == boost::asio::error::eof) {
          logout("<< seems that socket was shutdown normally");
        }
        else {
          log_err("error in async_write(). code: " + std::to_string(a_error.value()) + ", transl: " + a_error.message());
        };


        logout("sending request to del drv...");
        m_drv_rp_own.request_to_del_drv(this);

        return;
      }

      if(!a_written) {
        logout("nothing was written? Strange...");
        m_drv_rp_own.request_to_del_drv(this);
        return;
      }

      logout("<< num written: " + std::to_string(a_written));

      // тут всё обрабатываем, если надо ещё раз вызываем запись:
      handle_written(a_written);
      
    });
}
//---------------------------------------------------------------------------


// отработать, то что что-то ушло в сеть:
void mrpc::tcp_connect::handle_written(std::size_t a_written)
{

  { 
     // нужно из под крит секции. т.к. на размер буфера, индекс также смотрят при 
    // добавлении новой задачи на запись
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_write_buf_pos + a_written > m_write_buf.size()) {
      log_err("some overrun happened");
      return;
    }

    m_write_buf_pos += a_written;


    if(m_write_buf_pos == m_write_buf.size()) {
      logout("all buff written");

      // выталкивает то что писали (удаляем)
      m_queue.pop();

      logout("new size of queue after pop: " + std::to_string(m_queue.size()));

      check_start_write();
    }

    return;
  }

};
//---------------------------------------------------------------------------


void mrpc::tcp_connect::handle_complete_msg()
{
  logout("in handle_complete_msg");
  logout("Cmd info: " + print_header(m_p_read_obj->m_header));

 
  // Если это запрос, то по любому отправляем на исполнение "серванту":
  if (m_p_read_obj->m_header.m_is_request) {
    mp_drv_rp->handle_net_request(m_p_read_obj->m_header.m_dw_task_id, 
      m_p_read_obj->m_header.m_dw_func_id, std::move(m_p_read_obj->m_vec_payload));
    return;
  }



  {
    // под мьютексом, т.к. удаляем из дефер буфера
    std::lock_guard<std::mutex> lock(m_mutex);

    // Если это ответ, то возможно у нас есть уже отлженная запись и подвешенный на ней клиент. Посмотрим:
    auto iter = m_map_defer_tasks.find(m_p_read_obj->m_header.m_dw_task_id);
    if (iter != m_map_defer_tasks.end()) {
      const auto& record = *iter;

      logout("before set_promise()");
      record.second->set_promise(std::move(m_p_read_obj->m_vec_payload));
      logout("after set_promise()");

      m_map_defer_tasks.erase(iter);
      logout("after erase defer record");

      return;
    }
  }

  // если здесь, то 

  //mp_drv_rp->handle_net_answer(m_p_read_obj->m_header.m_dw_task_id, std::move(m_p_read_obj->m_vec_payload));
  return;

};
//---------------------------------------------------------------------------


// Забросить задание на выполнение:
int mrpc::tcp_connect::put_request(uint32_t adw_func_id, std::vector<uint8_t>&& a_buf, uint32_t& adw_taskid)
{
  adw_taskid = 0;

  int n_res = push_task_request(adw_func_id, std::move(a_buf), adw_taskid);
  if (n_res) {
    log_err("error in push_task_request()");
    return n_res;
  }

  return 0;
};
//---------------------------------------------------------------------------



// Забросить ответ на задание:
int mrpc::tcp_connect::put_answer(uint32_t adw_req_id, std::vector<uint8_t>&& a_buf)
{

  // "-1"  - id функции. в ответе не используется
  int n_res = push_task_answer(adw_req_id, std::move(a_buf));
  if (n_res) {
    log_err("error in push_task_answer()");
    return n_res;
  }

  return 0;
};
//---------------------------------------------------------------------------


// выполнить блокирующий запрос
int mrpc::tcp_connect::exec_request(uint32_t adw_func_id, std::vector<uint8_t>&& a_buf, 
  std::vector<uint8_t>& a_res)
{
  
  uint8_t b_first = 0;
  if(a_buf.size()) {
    b_first = a_buf[0];
  }

  // кладём задачу на выполнение, инициализируем промис для ожидания:
  std::future<t_payload> a_future;
  int n_res = push_task_4block(adw_func_id, std::move(a_buf), a_future);
  if (n_res) {
    log_err("error in push_task_4block()");
    return n_res;
  }

  logout("push_task_4block() OK");


  logout("before future.wait()...");
  a_future.wait();


  const auto& result = a_future.get();

  bool f_is_res_ok = true;
  for(auto& curr : result) 
  {
    if (curr != b_first + 1) {
      log_err("wrong data in answer");
      f_is_res_ok = false;
      break;
    }
  }

  if(!f_is_res_ok) {
    log_err("received msg not corresponds to request?");
  }
  else {
    logout("future.wait() completed. msg is OK");
  }

  return 0;
};
//---------------------------------------------------------------------------



// кладём задачу на выполнение:
int mrpc::tcp_connect::push_task_request(uint32_t adw_func_id, std::vector<uint8_t>&& avec_data, 
  uint32_t& adw_task_id)
{
  // обнуляем выходные данные:
  adw_task_id = 0;


  {
    std::lock_guard<std::mutex> lock(m_mutex);


    adw_task_id = m_dwLastTaskID;
    // пока просто ++ без проверки. TODO: проверить, что такой же нет в буфере, если есть, то inc пока не найдём свободной
    m_dwLastTaskID++;


    //todo: заменить на make_unique
    auto rec = std::unique_ptr<mrpc::t_cmd_record>(
      new mrpc::t_cmd_record(true, adw_task_id, adw_func_id, std::move(avec_data)));

    m_queue.emplace(std::move(rec));

    logout("size of queue after push: " + std::to_string(m_queue.size()));


    std::unique_ptr<mrpc::t_defer_rec> tmp(std::make_unique<mrpc::t_defer_rec>());
    m_map_defer_tasks.emplace(m_dwLastTaskID, std::move(tmp));

    logout("size of defer buf after register: " + std::to_string(m_queue.size()));

    check_start_write();
  }

  return 0;
};
//---------------------------------------------------------------------------



// кладём ответ:
int mrpc::tcp_connect::push_task_answer(uint32_t adw_task_id, std::vector<uint8_t>&& avec_data)
{
  {
    std::lock_guard<std::mutex> lock(m_mutex);

    //todo: заменить на make_unique
    auto rec = std::unique_ptr<mrpc::t_cmd_record>(
      new mrpc::t_cmd_record(false, adw_task_id, -1, std::move(avec_data)));

    m_queue.emplace(std::move(rec));

    logout("size of queue after push: " + std::to_string(m_queue.size()));

    check_start_write();
  }

  return 0;
};
//---------------------------------------------------------------------------



/*
// кладём задачу на выполнение:
int mrpc::tcp_connect::push_task(bool af_is_request, uint32_t adw_task_id, uint32_t adw_func_id, std::vector<uint8_t>&& avec_data)
{

  {
    std::lock_guard<std::mutex> lock(m_mutex);

    //todo: заменить на make_unique


    uint32_t dw_task_id = 0;

    if(af_is_request) {
      dw_task_id = m_dwLastTaskID;
      // пока просто ++ без проверки. TODO: проверить, что такой же нет в буфере, если есть, то inc пока не найдём свободной
      m_dwLastTaskID++;
    }
    else {
      dw_task_id = adw_task_id;
    }


    auto rec = std::unique_ptr<mrpc::t_cmd_record>(
      new mrpc::t_cmd_record(af_is_request, dw_task_id, adw_func_id, std::move(avec_data)));

    m_queue.emplace(std::move(rec));

    logout("size of queue after push: " + std::to_string(m_queue.size()));


    if (af_is_request) {
      std::unique_ptr<mrpc::t_defer_rec> tmp(std::make_unique<mrpc::t_defer_rec>());
      m_map_defer_tasks.emplace(m_dwLastTaskID, std::move(tmp));

      logout("size of defer buf after register: " + std::to_string(m_queue.size()));
    }

    check_start_write();
  }

  return 0;
};
//---------------------------------------------------------------------------
*/



// кладём задачу на выполнение, инициализируем промис для ожидания:
int mrpc::tcp_connect::push_task_4block(uint32_t adw_func_id, std::vector<uint8_t>&& avec_data, 
  std::future<t_payload>& a_future)
{
  {
    std::lock_guard<std::mutex> lock(m_mutex);


    uint32_t dw_task_id = m_dwLastTaskID;
    m_dwLastTaskID++;

    auto rec = std::unique_ptr<mrpc::t_cmd_record>(
      new mrpc::t_cmd_record(true, dw_task_id, adw_func_id, std::move(avec_data)));
    // пока просто ++ без проверки. TODO: проверить, что такой же нет в буфере, если есть, то inc пока не найдём свободной
    
    m_queue.emplace(std::move(rec));

    logout("size of queue after push: " + std::to_string(m_queue.size()));
    std::unique_ptr<mrpc::t_defer_rec> tmp(std::make_unique<mrpc::t_defer_rec>(a_future));
    m_map_defer_tasks.emplace(dw_task_id, std::move(tmp));
    logout("size of defer buf after register: " + std::to_string(m_queue.size()));

    check_start_write();
  }

  return 0;
};
//---------------------------------------------------------------------------


// смотрим нужно ли начинать запись:
void mrpc::tcp_connect::check_start_write()
{

  if(m_write_buf.size() && (m_write_buf_pos != m_write_buf.size())) {
    logout("writing in progress. so just waiting. buf_size: " + 
      std::to_string(m_write_buf.size()) + ", pos: " + std::to_string(m_write_buf_pos) + 
      ", msg size: " + std::to_string(m_write_buf.size()));
    return;
  }

  logout("No writing right now, so let's order writing");

  if(!m_queue.size()) {
    logout("queue is empty. nothing to send");
    return;
  }

  // "сериализуем" первую в очереди запись:
  const auto& record = m_queue.front();

  // рассчитаем всю посылку:
  size_t un_total = sizeof(mrpc::protocol::t_msg_header) + record->m_vec_data.size();
  // зарезервируем сколько нужно памяти:
  m_write_buf.resize(un_total);

  // заполняем хидер данными для "пользовательской" посылки:
  mrpc::protocol::t_msg_header header = mrpc::protocol::t_msg_header::create_custom_request(
    record->m_is_request, record->m_dw_req_id, record->m_dw_func_id, record->m_vec_data.size());

  // копируем заголовок на данные:
  std::memcpy(&m_write_buf[0], &header, sizeof(header));

  std::memcpy(&(m_write_buf[0]) + sizeof(header), &record->m_vec_data[0], record->m_vec_data.size());

  // выставляем позицию "записано" в 0:
  m_write_buf_pos = 0;

  // начинаем запись. По идее нужно их под критической секции это делать, а потом мы из неё выйдем
  do_write();
};
//---------------------------------------------------------------------------

void mrpc::tcp_connect::logout(const std::string& astr_info)
{
  clog::logout("'" + m_str_drv_id + "' " + astr_info);
};
//---------------------------------------------------------------------------


void mrpc::tcp_connect::log_err(const std::string& astr_info)
{
  clog::log_err("'" + m_str_drv_id + "' " + astr_info);
};
//---------------------------------------------------------------------------