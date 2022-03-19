//---------------------------------------------------------------------------
#pragma once
//---------------------------------------------------------------------------
#include <vector>
//---------------------------------------------------------------------------



namespace mrpc {


//---------------------------------------------------------------------------
class i_driver_rp;
//---------------------------------------------------------------------------


// Интерфейс уведомления о подключении нового драйвера:
class i_driver
{
  public:

    // "пост" инициализация
    virtual void set_drv_rp(mrpc::i_driver_rp* ap_drv_rp) = 0;

    virtual ~i_driver() {};

    // запускаем работу драйвера
    virtual int start() = 0;

    // Забросить какое-то задание на выполнение:
    virtual int put_request(uint32_t adw_func_id, std::vector<uint8_t>&& a_buf, uint32_t& adw_req_id) = 0;

    // Забросить ответ на задание:
    virtual int put_answer(uint32_t adw_req_id, std::vector<uint8_t>&& a_buf) = 0;



    // выполнить блокирующий запрос
    virtual int exec_request(uint32_t adw_func_id, std::vector<uint8_t>&& a_buf, 
      std::vector<uint8_t>& a_res) = 0;

    virtual void clear_drv_rp() = 0;



};
//---------------------------------------------------------------------------


} // mrpc