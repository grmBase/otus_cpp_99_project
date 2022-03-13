//---------------------------------------------------------------------------
#include <iostream>
#include <thread>
//---------------------------------------------------------------------------
#include "appl.h"
//---------------------------------------------------------------------------
#include "sync_console.h"
//---------------------------------------------------------------------------


t_appl::t_appl(uint16_t aun_listen_port)
{

  clog::logout("in constructor of appl");

  // число потоков возьмём исходя из числа CPU:
  const auto processor_count = std::thread::hardware_concurrency();
  std::cout << "num of CPU cores detected: " << processor_count << std::endl;


  clog::logout("before creating server...");
  m_p_server.reset(mrpc::i_server::create(aun_listen_port, processor_count, *this, *this));
  clog::logout("server created");


  clog::logout("before server start_listen()...");
  m_p_server->start_listen();
  clog::logout("server start_listen() completed");

  clog::logout("before server start_threads...");
  m_p_server->start_threads();
  clog::logout("server start_threads completed");
};
//---------------------------------------------------------------------------


t_appl::~t_appl()
{
  clog::logout("in destructor of appl");


  {
    // сначала попробуем удалить все драйвера:
    std::lock_guard<std::mutex> lock(m_mutex);

    clog::logout("before clear all drivers...");
    m_vec_drivers.clear();
    clog::logout("all drivers deleted");
  }


  clog::logout("before stopping server...");
  m_p_server->stop();
  clog::logout("server stopped");

  clog::logout("before deleting server...");
  m_p_server.reset();
  clog::logout("server deleted");


};
//---------------------------------------------------------------------------


void t_appl::notify_new_drv_income(mrpc::i_driver& a_drv)
{
  std::lock_guard<std::mutex> lock(m_mutex);


  clog::logout("<< in notify_new_drv(). Num of drivers in store: " + std::to_string(m_vec_drivers.size()));

  auto tmp = std::unique_ptr<tst::t_custom_drv>(new tst::t_custom_drv(a_drv, true));

  m_vec_drivers.emplace((uintptr_t)&a_drv, std::move(tmp));

  int n_res = a_drv.start();
  if(n_res) {
    clog::log_err("Error in start() of driver");
    return;
  }
};
//---------------------------------------------------------------------------


void t_appl::notify_new_drv_connect(mrpc::i_driver& a_drv)
{

  std::lock_guard<std::mutex> lock(m_mutex);


  clog::logout("<< in notify_new_drv(). Num of drivers in store: " + std::to_string(m_vec_drivers.size()));

  auto tmp = std::unique_ptr<tst::t_custom_drv>(new tst::t_custom_drv(a_drv, true));

  m_vec_drivers.emplace((uintptr_t)&a_drv, std::move(tmp));

  int n_res = a_drv.start();
  if (n_res) {
    clog::log_err("Error in start() of driver");
    return;
  }
};
//---------------------------------------------------------------------------


// запрос на удаление драйвера
void t_appl::request_to_del_drv(mrpc::i_driver* ap_drv)
{

  std::lock_guard<std::mutex> lock(m_mutex);

  clog::logout("request_to_del_drv");

  size_t un_deleted = m_vec_drivers.erase((uintptr_t)ap_drv);
  if (un_deleted != 1) {
    clog::log_err("Error deleting driver");
    return;
  }

  clog::logout("driver deleted successfully");
  return;
};
//---------------------------------------------------------------------------



// имитируем коннект клиента:
int t_appl::client_connect()
{
  return m_p_server->client_connect();
};
//---------------------------------------------------------------------------
