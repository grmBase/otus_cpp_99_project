//---------------------------------------------------------------------------
#include <chrono>    // используем время
#include <vector>    //
#include <algorithm> // std::transform
//---------------------------------------------------------------------------
#include <boost/asio.hpp>
//---------------------------------------------------------------------------
#include "tst_custom_drv.h"
//---------------------------------------------------------------------------
#include "include/sconsole/sync_console.h" // логи
//---------------------------------------------------------------------------


tst::t_custom_drv::t_custom_drv(std::shared_ptr<mrpc::i_driver> ap_drv, size_t a_num_of_threads,
  bool af_send_requests)
  : mp_drv(ap_drv),
    m_threads{5},
    m_pool{a_num_of_threads}
{

  
  {
    auto p_tmp = mp_drv.lock();
    if(p_tmp) {
      p_tmp->set_drv_rp(this);
    }
    else {
      clog::log_err("driver is nullptr");
    }
  }
  

  clog::logout("in constructor of t_custom_drv()");

  clog::logout("before start thread...");

  if(af_send_requests) {

    // запускаем нити:
    for (auto& curr : m_threads) {
      curr = std::thread(
      [this]
      {
        try 
        {
          this->work();
        }
        catch (const std::exception& ex)
        {
          clog::log_err(std::string("Exception in work. details: ") + ex.what());
        }
        catch (...)
        {
          clog::log_err("Exception '...' in thread");
        }
      }
      );

    };

    /*
    m_thread = std::thread(
      [this] 
      {
        try {
          this->work(); 
        }
        catch(const std::exception& ex)
        {
          clog::log_err(std::string("Exception in work. details: ") + ex.what());
        }
        catch(...)
        {
          clog::log_err("Exception '...' in thread");
        }
      }
    );
    */
  }
  clog::logout("after threads started");
};
//---------------------------------------------------------------------------


// то, что делаем из нити
void tst::t_custom_drv::work()
{

  while(true)
  {

    std::unique_lock lock(m_mutex);
    while(!m_exit_flag) 
    {
      auto result = m_cv.wait_for(lock, std::chrono::milliseconds(3000));
      if(result == std::cv_status::timeout)  {

        //clog::logout("timeout happened. Here going to be our periodical work");
        

        for(size_t i=0; i<10; ++i)
        {
          int n_res = exec_block_call();
          if (n_res) {
            clog::log_err("error in exec_block_call()");
          }
          else {
            clog::logout("exec_block_call() passed");
          }
        }


        /*
        // закинем 3 задания подряд
        for(size_t i=0; i<5; ++i) 
        {
          int n_res = make_request();
          if (n_res) {
            clog::log_err("error in make_request()");
          }
          else {
            clog::logout("make_request() passed");
          }
        }
        */

      }
    }

    if (m_exit_flag) {
      clog::logout("Exit flag detected. Exiting from thread...");
      return;
    }
  }

};
//---------------------------------------------------------------------------


tst::t_custom_drv::~t_custom_drv()
{
  clog::logout("in destructor of t_custom_drv()");

  {
    auto p_drv = mp_drv.lock();
    if(p_drv) {
      p_drv->clear_drv_rp();
    };
  }

  clog::logout("before set_exit() flag for thread...");
  {
    {
      std::lock_guard lk(m_mutex);
      m_exit_flag = true;
    }

    m_cv.notify_all();
  }
  clog::logout("after set_exit() flag for thread");


  // Сначала удалим драйвер - он может "держать" нить на промисе
  clog::logout("before reset mrpc driver...");
  mp_drv.reset();
  clog::logout("after reset mrpc driver...");


  clog::logout("before join() threads");
  for (auto& curr : m_threads) 
  {
    if(curr.joinable()) {
      clog::logout("before thread join");
      curr.join();
      clog::logout("after thread join");
    }

  };

};
//---------------------------------------------------------------------------



// делаем сетевой запрос
int tst::t_custom_drv::make_request()
{

  std::vector<uint8_t> vec_in_buf(500000, 0);
  uint32_t dw_req_id = 0;
  auto tmp = mp_drv.lock();
  int n_res = tmp->put_request(0, std::move(vec_in_buf), dw_req_id);
  if (n_res) {
    clog::log_err("error in exec_request()");
    return n_res;
  }

  clog::logout("put_request completed. request_id assigned: " + std::to_string(dw_req_id));

  return 0;
};
//---------------------------------------------------------------------------


// делаем некий блокирующий вызов:
int tst::t_custom_drv::exec_block_call()
{
  std::vector<uint8_t> vec_in_buf(500000, 0);
  std::vector<uint8_t> vec_res_buf;


  auto tmp = mp_drv.lock();
  if (!tmp) {
    clog::log_err("!!! driver is empty");
    return -33;
  }
  int n_res = tmp->exec_request(1, std::move(vec_in_buf), vec_res_buf);
  if (n_res) {
    clog::log_err("Error in exec_request(). code: " + std::to_string(n_res));
    return n_res;
  }

  if (vec_res_buf.size() != vec_in_buf.size()) {
    clog::log_err("Result buf size not equal to request one. size: " + std::to_string(vec_res_buf.size()));
    return -33;
  }

  auto res = std::mismatch(vec_res_buf.begin(), vec_res_buf.end(), vec_in_buf.begin(), 
    [] (uint8_t elem1, uint8_t elem2)
    {
      return (elem1 - 1) == elem2;
    }
  );

  size_t pos1 = res.first - vec_res_buf.begin();
  size_t pos2 = res.second - vec_in_buf.begin();

  if (pos1 != pos2) {
    clog::log_err("buffers are incorrect");
    return -34;
  }

  if (pos1 != vec_in_buf.size()) {
    clog::log_err("buffers are incorrect check condition #2");
    return -35;
  }

  clog::logout("call passed OK");

  return n_res;
};
//---------------------------------------------------------------------------


// Запрос на отработать новую команду с другой стороны
void tst::t_custom_drv::handle_net_request(uint32_t adw_req_id, 
  uint32_t adw_func_id, std::vector<uint8_t>&& a_vec_req)
{
  clog::logout("<< received new request. req_id: " + std::to_string(adw_req_id));


  if(adw_func_id == 0) {
    std::vector<uint8_t> vec_results;
    vec_results.resize(a_vec_req.size());

    // В тестовой функции просто увеличиваем на 1-цу
    std::transform(a_vec_req.begin(), a_vec_req.end(), vec_results.begin(),
      [](uint8_t x) -> uint8_t
      {
        return x+1;
      });
    

    auto p_tmp = mp_drv.lock();
    if(p_tmp) {
      int n_res = p_tmp->put_answer(adw_req_id, std::move(vec_results));
      if(n_res) {
        clog::log_err("Error in put_answer()");
        return;
      }
    }
    else {
      clog::log_err("error in put_answer()");
    }

    return;
  }

  if(adw_func_id == 1) 
  {
    boost::asio::post(m_pool, 
      [adw_req_id, a_vec_req, this]
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        clog::logout("handling in thread completed");


        // пока просто что-нибудь вернуть, todo: реально что-то модифицировать
        std::vector<uint8_t> vec_results;
        vec_results.resize(a_vec_req.size());

        // В тестовой функции просто увеличиваем на 1-цу
        std::transform(a_vec_req.begin(), a_vec_req.end(), vec_results.begin(),
          [](uint8_t x) -> uint8_t
          {
            return x + 1;
          });



        auto tmp = mp_drv.lock();
        if (!tmp) {
          clog::log_err("error drv is already null");
          return;
        }

        int n_res = tmp->put_answer(adw_req_id, std::move(vec_results));
        if (n_res) {
          clog::log_err("Error in put_answer()");
          return;
        }

      }
    );

    return;
  }

  clog::log_err("unknown function id: " + std::to_string(adw_func_id));
};
//---------------------------------------------------------------------------


// Запрос отрабоать асинхронный ответ
void tst::t_custom_drv::handle_net_answer(uint32_t adw_req_id, std::vector<uint8_t>&& a_vec_request)
{
  clog::log_err("not implemented yet");
};
//---------------------------------------------------------------------------
