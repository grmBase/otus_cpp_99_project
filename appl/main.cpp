//---------------------------------------------------------------------------
#include <iostream>
//---------------------------------------------------------------------------
//#include "server.h"
#include "appl.h"
//---------------------------------------------------------------------------
#include "sync_console.h"
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



    // добавим клиентский коннект:
    {
      // чуть подождём:
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      int n_res = appl.client_connect();
      if (n_res) {
        clog::log_err("error in client_connect()");
        return n_res;
      }
    }
    

    clog::logout("press any key to stop and exit");
    std::getchar();


    //clog::logout("before call stop()");
    //server.stop();
    //clog::logout("after stop()");
  }
  catch(const std::exception& aexc)
  {
    clog::log_err(std::string("caught exeption: ") + aexc.what());
    return -33;
  }

  return 0;
}
//---------------------------------------------------------------------------