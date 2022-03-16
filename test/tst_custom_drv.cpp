//---------------------------------------------------------------------------
#include <chrono> // используем время
#include <vector> //
//---------------------------------------------------------------------------
#include "tst_custom_drv.h"
//---------------------------------------------------------------------------
#include "include/sconsole/sync_console.h" // логи
//---------------------------------------------------------------------------


tst::t_custom_drv::t_custom_drv(std::shared_ptr<mrpc::i_driver> ap_drv, bool af_send_requests)
  :mp_drv(ap_drv)
{

  mp_drv->set_drv_rp(this);

  clog::logout("in constructor of t_custom_drv()");

  clog::logout("before start thread...");

  if(af_send_requests) {
    m_thread = std::thread(
      [this] 
      {
        this->work(); 
      }
    );
  }
  clog::logout("after thread started");
};
//---------------------------------------------------------------------------


// то, что делаем из нити
void tst::t_custom_drv::work()
{

  while(true)
  {

    std::unique_lock<std::mutex> lock(m_mutex);
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


  clog::logout("before set_exit() flag for thread...");
  {
    {
      std::lock_guard<std::mutex> lk(m_mutex);
      m_exit_flag = true;
    }

    m_cv.notify_one();
  }
  clog::logout("after set_exit() flag for thread");


  // Сначала удалим драйвер - он может "держать" нить на промисе
  clog::logout("before reset mrpc driver...");
  mp_drv.reset();
  clog::logout("after reset mrpc driver...");


  if(m_thread.joinable()) {
    clog::logout("before thread join");
    m_thread.join();
    clog::logout("after thread join");
  };
};
//---------------------------------------------------------------------------



// делаем сетевой запрос
int tst::t_custom_drv::make_request()
{

  std::vector<uint8_t> vec_in_buf(500000, 0);
  uint32_t dw_req_id = 0;
  int n_res = mp_drv->put_request(0, std::move(vec_in_buf), dw_req_id);
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

  int n_res = mp_drv->exec_request(0, std::move(vec_in_buf), vec_res_buf);
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

    for(size_t i=0; i<a_vec_req.size(); ++i)
    {
       vec_results[i] = a_vec_req[i] + 1;
    };

    int n_res = mp_drv->put_answer(adw_req_id, std::move(vec_results));
    if (n_res) {
      clog::log_err("Error in put_answer()");
      return;
    }
    return;
  }

  clog::log_err("unknowd function id: " + std::to_string(adw_func_id));

};
//---------------------------------------------------------------------------


// Запрос отрабоать асинхронный ответ
void tst::t_custom_drv::handle_net_answer(uint32_t adw_req_id, std::vector<uint8_t>&& a_vec_request)
{
  clog::log_err("not implemented yet");
};
//---------------------------------------------------------------------------
