//---------------------------------------------------------------------------
#include <string>
#include <iostream>
#include <gtest/gtest.h>
//---------------------------------------------------------------------------
#include "appl.h"
//---------------------------------------------------------------------------
#include "include/sconsole/sync_console.h"
#include "include/mrpc/iocp_block.h"
//---------------------------------------------------------------------------



TEST(CheckBlockCall, SomeInfo1)
{
  std::cout << "in start of test func" << std::endl;

  int n_port = 5000;

  t_appl appl(n_port);

  // запускаем блокирующие тесты:
  t_iocp_block iocp_block{};

  int n_res = iocp_block.send_recieve_custom_msg("127.0.0.1", n_port);
  EXPECT_EQ(n_res, 0) << "result is ok";
  clog::logout("send_receive passed OK");
}
//---------------------------------------------------------------------------



TEST(CheckWorkLoadEmulation, SomeInfo2)
{
  std::cout << "in start of test func" << std::endl;

  int n_port = 5000;

  t_appl appl(n_port);

  // добавим клиентский коннект:
  {
    // чуть подождём:
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    int n_res = appl.client_connect("127.0.0.1", std::to_string(n_port));
    EXPECT_EQ(n_res, 0) << "result should be ok";
  }

  // Подождём 10 сек, и выходим:
  std::this_thread::sleep_for(std::chrono::milliseconds(10000));
}
//---------------------------------------------------------------------------

