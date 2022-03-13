//---------------------------------------------------------------------------
#pragma once
//---------------------------------------------------------------------------
#include <stdint.h>
#include <unordered_map>
#include <memory>
#include <mutex>
//---------------------------------------------------------------------------
#include "include/mrpc/i_server.h"
#include "include/mrpc/i_listen_rp.h"
//---------------------------------------------------------------------------
#include "include/mrpc/i_driver.h"
#include "include/mrpc/i_driver_rp_own.h"
//---------------------------------------------------------------------------
#include "tst_custom_drv.h"  // тут храним эти драйвера
//---------------------------------------------------------------------------




class t_appl : 
  public mrpc::i_listen_rp,
  public mrpc::i_driver_rp_own
{

  public:

    t_appl(uint16_t aun_listen_port);
    ~t_appl();


    // имитируем коннект клиента:
    int client_connect();



    void notify_new_drv_income(mrpc::i_driver& a_drv) override;
    void notify_new_drv_connect(mrpc::i_driver& a_drv) override;

    // отослать запрос, полученный из сети
    //void handle_net_request(std::vector<uint8_t>& a_vec_result) override;
    
    // запрос на удаление драйвера
    void request_to_del_drv(mrpc::i_driver* ap_drv) override;

  private:

    //Храним сервер
    std::unique_ptr<mrpc::i_server> m_p_server;
    
    // указатель будет на mrpc::i_driver, а хранить будем уже наш расширенный
    std::unordered_map<uintptr_t, std::unique_ptr<tst::t_custom_drv>> m_vec_drivers;

    mutable std::mutex m_mutex;

};
//---------------------------------------------------------------------------