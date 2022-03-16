//---------------------------------------------------------------------------
#include <iostream>
//---------------------------------------------------------------------------
#include "appl.h"
//---------------------------------------------------------------------------
#include "include/sconsole/sync_console.h"
#include "include/mrpc/iocp_block.h"
//---------------------------------------------------------------------------



int main(int argc, char* argv[])
{
  std::cout << "in main() start" << std::endl;

  if (argc != 2) {
    std::cerr << "Usage: server <listen port>" << std::endl;
    return 1;
  }

  int n_port = std::atoi(argv[1]);

  std::cout << "Detected params: tcp port: " << n_port << std::endl;



  try
  {

    t_appl appl(n_port);


    // запускаем блокирующие тесты:
    t_iocp_block iocp_block{};

    int n_res = iocp_block.send_recieve_custom_msg("127.0.0.1", n_port);
    if (n_res) {
      clog::log_err("error in send_recieve_custom_msg");
      return n_res;
    }
    clog::logout("send_receive passed OK");

  }
  catch (const std::exception& aexc)
  {
    clog::log_err(std::string("caught exeption: ") + aexc.what());
    return -33;
  }




  try
  {
    t_appl appl(n_port);

    // добавим клиентский коннект:
    {
      // чуть подождём:
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      int n_res = appl.client_connect("127.0.0.1", std::to_string(n_port));
      if (n_res) {
        clog::log_err("error in client_connect()");
        return n_res;
      }
    }

    // Подождём 10 сек, и выходим:
    std::this_thread::sleep_for(std::chrono::milliseconds(10000));
  }
  catch(const std::exception& aexc)
  {
    clog::log_err(std::string("caught exeption: ") + aexc.what());
    return -33;
  }


  return 0;
}
//---------------------------------------------------------------------------
