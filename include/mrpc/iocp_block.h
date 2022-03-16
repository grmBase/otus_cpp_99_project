//---------------------------------------------------------------------------
#pragma once
//---------------------------------------------------------------------------
#include <boost/asio.hpp>
//---------------------------------------------------------------------------
using b_tcp = boost::asio::ip::tcp;
//---------------------------------------------------------------------------



class t_socket_block
{

  public:

  t_socket_block(boost::asio::io_service& a_io_service)
    : m_socket(a_io_service)
  {
  }

  ~t_socket_block()
  {
    m_socket.shutdown(b_tcp::socket::shutdown_receive);
    m_socket.close();
  };

  int write_buf(const uint8_t* a_pbuf, size_t asize)
  {
    size_t un_written = 0;
    while(un_written < asize)
    {
      size_t un_curr = m_socket.write_some(boost::asio::buffer(
        a_pbuf + un_written, asize - un_written));
      if(!un_curr) {
        return 33;
      }
      un_written += un_curr;
    }

    return 0;
  };


  int read_buf(uint8_t* a_pbuf, size_t asize)
  {
    size_t un_readed = 0;
    while (un_readed < asize)
    {
      size_t un_curr = m_socket.read_some(boost::asio::buffer(
        a_pbuf + un_readed, asize - un_readed));
      if (!un_curr) {
        return 33;
      }
      un_readed += un_curr;
    }

    return 0;
  };



  b_tcp::socket m_socket;
};
//---------------------------------------------------------------------------



class t_iocp_block
{
  public:
 
    t_iocp_block();
    ~t_iocp_block();


    int send_recieve_custom_msg(const std::string& astr_host, uint16_t aw_port);


    int send_wrong_header_msg(const std::string& astr_host, uint16_t aw_port);


  private:

    int connect(const std::string& astr_host, uint16_t aw_port, b_tcp::socket& a_socket);

    // ------- данные -------
    boost::asio::io_service m_io_service;
};
//---------------------------------------------------------------------------