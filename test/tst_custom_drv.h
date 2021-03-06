//---------------------------------------------------------------------------
#pragma once
//---------------------------------------------------------------------------
#include <memory> // unique_ptr
#include <vector>
//---------------------------------------------------------------------------
#include <thread> // драйвер будет "живым"
#include <mutex>
#include <condition_variable>
//---------------------------------------------------------------------------
#include <boost/asio/thread_pool.hpp> // для организации пула потоков
//---------------------------------------------------------------------------
#include "include/mrpc/i_driver.h"
#include "include/mrpc/i_driver_rp.h"
//---------------------------------------------------------------------------




namespace tst {


// Тестовый "клиентский" драйвер. Выдаёт клиентские запросы через указанные интервалы
class t_custom_drv : public mrpc::i_driver_rp
{
  public:

    t_custom_drv(std::shared_ptr<mrpc::i_driver> ap_drv, size_t a_num_of_threads, bool af_send_requests);

    ~t_custom_drv();


    // то, что делаем из нити
    void work();

    // Запрос на отработать новую команду с другой стороны
    void handle_net_request(uint32_t adw_req_id, 
      uint32_t adw_func_id, std::vector<uint8_t>&& a_vec_request) override;

    // Запрос отрабоать асинхронный ответ
    void handle_net_answer(uint32_t adw_req_id, std::vector<uint8_t>&& a_vec_request) override;



  private:

    // делаем некий блокирующий вызов:
    int exec_block_call();


    // делаем сетевой запрос
    int make_request();
    

    // пул потоков для "медленных", пользовательских функций
    boost::asio::thread_pool m_pool;



    // указатель на основной "рабочий" драйвер mrpc
    std::weak_ptr<mrpc::i_driver> mp_drv;


    // нитка из которой будем посылать запросы
    //std::thread m_thread;

    // здесь храним список рабочих нитей
    std::vector<std::thread> m_threads;


    bool m_exit_flag = false;
    mutable std::mutex m_mutex;     // защита от мнгопоточного доступа
    std::condition_variable m_cv;   // сигнал наличия работы

};
//---------------------------------------------------------------------------


} // tst