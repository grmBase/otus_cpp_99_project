//---------------------------------------------------------------------------
#include "include/mrpc/iocp_block.h"
//---------------------------------------------------------------------------
#include "include/sconsole/sync_console.h"
#include "mrpc_lib/protocol.h" // используем формат хидера для тестов
//---------------------------------------------------------------------------




t_iocp_block::t_iocp_block()
{
};
//---------------------------------------------------------------------------



t_iocp_block::~t_iocp_block()
{
};
//---------------------------------------------------------------------------



int t_iocp_block::send_recieve_custom_msg(const std::string& astr_host, uint16_t aw_port)
{

  t_socket_block block_socket{m_io_service};

  int n_res = connect(astr_host, aw_port, block_socket.m_socket);
  if (n_res) {
    clog::log_err("error in connect()");
    return n_res;
  }
  
  const size_t payload_size = 15;


  // заполним, отошлём хидер:
  mrpc::protocol::t_msg_header header_cmd;

  //header_cmd.m_b_sign = 0;
  header_cmd.m_b_cmd_id = mrpc::protocol::cmd_type::custom;
  header_cmd.m_dw_func_id = 0;
  header_cmd.m_is_request = true;
  header_cmd.m_dw_buf_size = payload_size;
  header_cmd.m_dw_task_id = 345;

  
  n_res = block_socket.write_buf((uint8_t*)&header_cmd, sizeof(header_cmd));
  if (n_res) {
    clog::log_err("error in write_buf() for header");
    return n_res;
  }

  std::vector<uint8_t> vec_payload(payload_size, 33);

  n_res = block_socket.write_buf((uint8_t*)&vec_payload[0], vec_payload.size());
  if (n_res) {
    clog::log_err("error in write_buf() for payload");
    return n_res;
  }


  // читаем ответный хидер:
  mrpc::protocol::t_msg_header header_reply = {};
  n_res = block_socket.read_buf((uint8_t*)(&header_reply), sizeof(header_reply));
  if(n_res) {
    clog::log_err("error in read_buf() for header");
    return n_res;
  }


  if(header_reply.m_dw_task_id != header_cmd.m_dw_task_id) {
    clog::log_err("incorrect task ID in reply");
    return n_res;
  }

  if(header_reply.m_dw_buf_size != header_cmd.m_dw_buf_size) {
    clog::log_err("incorrect reply header");
    return n_res;
  }



  std::vector<uint8_t> vec_payload_reply(payload_size);
  n_res = block_socket.read_buf((uint8_t*)&(vec_payload_reply[0]), vec_payload_reply.size());
  if(n_res) {
    clog::log_err("error in read_buf() for body");
    return n_res;
  }

  return 0;
};
//---------------------------------------------------------------------------



int t_iocp_block::send_wrong_header_msg(const std::string& astr_host, uint16_t aw_port)
{
  return -33;
};
//---------------------------------------------------------------------------


int t_iocp_block::connect(const std::string& astr_host, uint16_t aw_port, b_tcp::socket& a_socket)
{

  b_tcp::endpoint ep(boost::asio::ip::address::from_string(astr_host), aw_port);
  b_tcp::socket sock(m_io_service);
  sock.open(b_tcp::v4());

  try {
    sock.connect(ep);
  }
  catch (boost::system::system_error& ex) {
    clog::log_err("Exception cought when connect() on socket. Info: " + std::string(ex.what()) );
    return -33;
  }

  a_socket = std::move(sock);

  return 0;
};
//---------------------------------------------------------------------------
