//---------------------------------------------------------------------------
#include <benchmark/benchmark.h>
#include <iostream>
//---------------------------------------------------------------------------
#include "../test/appl.h"
//---------------------------------------------------------------------------
#include "include/sconsole/sync_console.h"
#include "include/mrpc/iocp_block.h"
//---------------------------------------------------------------------------





static void BM_ClientCalls(benchmark::State& state) 
{
  int n_port = 5000;

  t_appl appl(n_port);


  // запускаем блокирующие тесты:
  t_iocp_block iocp_block{};

  for (auto _ : state) {

    int n_res = iocp_block.send_recieve_custom_msg("127.0.0.1", n_port);
    if (n_res) {
      clog::log_err("error in send_recieve_custom_msg");
    }
  }
}
// Register the function as a benchmark
BENCHMARK(BM_ClientCalls);
//---------------------------------------------------------------------------





BENCHMARK_MAIN();
//---------------------------------------------------------------------------
