//---------------------------------------------------------------------------
#include <memory> // make_shared 
#include <iostream>
#include <boost/asio.hpp>
//---------------------------------------------------------------------------
#include "server.h" // наш хидер
//---------------------------------------------------------------------------
#include "include/mrpc/i_server.h"     // здесь же реализуем функцию
#include "include/mrpc/i_listen_rp.h"  // отсылаем созданные драйвера
#include "include/mrpc/i_driver_rp_own.h"  // отсылаем созданные драйвера
//---------------------------------------------------------------------------
#include "tcp_connect.h"
#include "include/sconsole/sync_console.h" // выводим логи синхронизованно
//---------------------------------------------------------------------------


// Функция конструирующая класс:
mrpc::i_server* mrpc::i_server::create(uint16_t aun_port, unsigned int aun_num_of_threads,
  mrpc::i_listen_rp& a_listener, mrpc::i_driver_rp_own& a_drv_rp)
{
  return new t_server(aun_port, aun_num_of_threads, a_listener, a_drv_rp);
};
//---------------------------------------------------------------------------



mrpc::t_server::t_server(uint16_t a_port, size_t a_num_of_threads, 
  mrpc::i_listen_rp& a_listen_rp, mrpc::i_driver_rp_own& a_drv_rp)
  : m_acceptor(m_io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), a_port)),
    m_threads{a_num_of_threads},
    m_listen_rp(a_listen_rp),
    m_drv_rp(a_drv_rp)
{
}
//---------------------------------------------------------------------------


mrpc::t_server::~t_server()
{
  clog::logout("before join() threads");
  for(auto& curr : m_threads) {
    curr.join();
  };

  clog::logout("after join() threads");
}
//---------------------------------------------------------------------------


int mrpc::t_server::start_listen()
{
  do_accept();

  return 0;
}
//---------------------------------------------------------------------------

void mrpc::t_server::do_accept()
{
  m_acceptor.async_accept(
    [this](boost::system::error_code a_error_code, boost::asio::ip::tcp::socket a_socket)
    {
      if (a_error_code) {
        clog::log_err("Error after accept()");
        return;
      }


      clog::logout("<< income connection. remote endpoint: " +
        a_socket.remote_endpoint().address().to_string() + ":" +
        std::to_string(a_socket.remote_endpoint().port()));

      auto p_driver = std::make_shared<tcp_connect>("recv_drv", std::move(a_socket), *this, m_drv_rp);

      m_listen_rp.notify_new_drv_income(p_driver);

      do_accept();

    });
}
//---------------------------------------------------------------------------


int mrpc::t_server::start_threads()
{
  clog::logout("before start working threads");

  // запускаем нити:
  for (auto& curr : m_threads) {
    curr = std::thread([this] {this->asio_thread_work(); });
  };

  clog::logout("after all working threads has been started");

  return 0;
};
//---------------------------------------------------------------------------


int mrpc::t_server::stop()
{
  clog::logout("before stop() io_context");
  m_io_context.stop();
  clog::logout("after stop() io_context");

  return 0;
}
//---------------------------------------------------------------------------

void mrpc::t_server::asio_thread_work()
{

  clog::logout("in start of thread work()");

  try
  {
    boost::system::error_code ec;

    clog::logout("before io_context.run()");
    m_io_context.run(ec);

    clog::logout("after io_context.run(), res code: " + std::to_string(static_cast<int>(ec.value())) + ", transl: " + ec.message());
  }
  catch (const std::exception& aexc)
  {
    clog::log_err(std::string("exception in asio_thread_work(), exception info: ") + aexc.what());
  }
}
//---------------------------------------------------------------------------


// коннектимся куда-то как клиент:
int mrpc::t_server::client_connect(const std::string& astr_host, const std::string& astr_port)
{

  b_tcp::resolver resolver(m_io_context);
  b_tcp::resolver::query query(astr_host, astr_port);
  b_tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

  b_tcp::endpoint endpoint = *endpoint_iterator;


  std::unique_ptr<b_tcp::socket> sp_socket = std::make_unique<b_tcp::socket>(m_io_context);

  sp_socket->async_connect(*endpoint_iterator,
    [this, sp_socket = std::move(sp_socket)](const boost::system::error_code& a_error)
    {
      if(a_error) {
        clog::log_err("Error after async_connect(). code: " + std::to_string(a_error.value()) + ", transl: " + a_error.message());
        return;
      }

      clog::logout("<< connection completed ok. remote endpoint: " +
        sp_socket->remote_endpoint().address().to_string() + ":" +
        std::to_string(sp_socket->remote_endpoint().port()));

      auto p_driver = std::make_shared<tcp_connect>("connect_drv", std::move(*sp_socket), *this, m_drv_rp);
      m_listen_rp.notify_new_drv_connect(p_driver);

    }
  );

  return 0;
};
//---------------------------------------------------------------------------
