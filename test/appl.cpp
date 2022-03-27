//---------------------------------------------------------------------------
#include <iostream>
#include <thread>
//---------------------------------------------------------------------------
#include "appl.h"
//---------------------------------------------------------------------------
#include "include/sconsole/sync_console.h"
//---------------------------------------------------------------------------


t_appl::t_appl(uint16_t aun_listen_port)
{
  clog::logout("in constructor of appl");

  // число потоков возьмём исходя из числа CPU:
  const auto processor_count = std::thread::hardware_concurrency();
  std::cout << "num of CPU cores detected: " << processor_count << std::endl;

  clog::logout("before creating server...");

  std::unique_ptr<mrpc::i_server> tmp(mrpc::i_server::create(aun_listen_port, processor_count, *this, *this));
  m_p_server.swap(tmp);
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
    std::lock_guard lock(m_mutex);

    clog::logout("before clear all drivers... number of drivers: " + 
      std::to_string(m_vec_drivers.size()));
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


void t_appl::notify_new_drv_income(std::shared_ptr<mrpc::i_driver> ap_drv)
{
  std::lock_guard<std::mutex> lock(m_mutex);


  clog::logout("<< in notify_new_drv(). Num of drivers in store: " + std::to_string(m_vec_drivers.size()));

  // число потоков возьмём исходя из числа CPU:
  const auto processor_count = std::thread::hardware_concurrency();

  auto tmp = std::make_unique<tst::t_custom_drv>(ap_drv, processor_count, true);

  m_vec_drivers.emplace(ap_drv.get(), std::move(tmp));
  //m_vec_drivers.insert(std::make_pair<(uintptr_t)ap_drv.get(), std::move(tmp)>);

  int n_res = ap_drv->start();
  if(n_res) {
    clog::log_err("Error in start() of driver");
    return;
  }
};
//---------------------------------------------------------------------------


void t_appl::notify_new_drv_connect(std::shared_ptr<mrpc::i_driver> ap_drv)
{

  std::lock_guard<std::mutex> lock(m_mutex);


  clog::logout("<< in notify_new_drv(). Num of drivers in store: " + std::to_string(m_vec_drivers.size()));

  // число потоков возьмём исходя из числа CPU:
  const auto processor_count = std::thread::hardware_concurrency();

  auto tmp = std::make_unique<tst::t_custom_drv>(ap_drv, processor_count, true);

  m_vec_drivers.emplace(ap_drv.get(), std::move(tmp));

  int n_res = ap_drv->start();
  if (n_res) {
    clog::log_err("Error in start() of driver");
    return;
  }
};
//---------------------------------------------------------------------------




// запрос на удаление драйвера
void t_appl::request_to_del_drv(mrpc::i_driver* ap_drv)
{
  if (!ap_drv) {
    clog::log_err("drv is nullptr");
    return;
  }


  std::lock_guard lock(m_mutex);

  auto iter = m_vec_drivers.find(ap_drv);
  if (iter == m_vec_drivers.end()) {
    clog::log_err("drvier was not found, ptr: ");
    return;
  }

  m_vec_drivers.erase(iter);

  clog::logout("driver was delete from store");
  // можно будет отработать удаление сразу
};
//---------------------------------------------------------------------------



// имитируем коннект клиента:
int t_appl::client_connect(const std::string& astr_host, const std::string& astr_port)
{
  return m_p_server->client_connect(astr_host, astr_port);
};
//---------------------------------------------------------------------------
