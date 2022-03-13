//---------------------------------------------------------------------------
#pragma once
//---------------------------------------------------------------------------
#include <boost/asio.hpp>
//---------------------------------------------------------------------------
#include <boost/asio/thread_pool.hpp> // для организации пула потоков
#include <boost/asio/post.hpp>
//---------------------------------------------------------------------------
#include "include/mrpc/i_server.h"   // наследуемся
//---------------------------------------------------------------------------
namespace mrpc
{
  class i_driver_rp_own;
};
//---------------------------------------------------------------------------
using b_tcp = boost::asio::ip::tcp;
//---------------------------------------------------------------------------


namespace mrpc
{

class t_server : public mrpc::i_server
{
  public:
    t_server(uint16_t a_port, size_t a_num_of_threads, 
      mrpc::i_listen_rp& a_listen_rp, mrpc::i_driver_rp_own& a_drv_rp);

    ~t_server();

    // то что вызывют нитки когда крутятся
    void asio_thread_work();


    int start_threads() override;
    // запускаем слушание порта:
    int start_listen() override;
    int stop() override;


    // коннектимся куда-то как клиент:
    int client_connect();

    // 
    //boost::asio::thread_pool& get_pool();

  private:

    void do_accept();



    // *********** данные ***************
    uint16_t m_port;

    // основной объект asio
    boost::asio::io_context m_io_context;

    // слушатель входящих запросов tcp
    b_tcp::acceptor m_acceptor;

    // здесь храним список рабочих нитей
    std::vector<std::thread> m_threads;


    // пул потоков "медленных, пользовательских" функция
    //boost::asio::thread_pool m_pool;



    // приёмная часть кто слушает новые присоединившиеся драйвера:
    mrpc::i_listen_rp& m_listen_rp;
    // приёмная часть, кто слушает запросы из сети драйверов и запросы на 
    // удаление драйверов
    mrpc::i_driver_rp_own& m_drv_rp;

};
//---------------------------------------------------------------------------

}; // mrpc

//---------------------------------------------------------------------------