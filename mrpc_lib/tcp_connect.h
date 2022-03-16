//---------------------------------------------------------------------------
#pragma once
//---------------------------------------------------------------------------
#include <memory>  // unique_ptr
#include <boost/asio.hpp>
#include <mutex>
#include <queue>
//---------------------------------------------------------------------------
#include "protocol.h"
#include "tcp_read_obj.h"
#include "tcp_read_obj.h"
#include "defer_rec.h"
#include "cmd_record.h"   // храним запросы

#include "include/mrpc/i_driver.h"    // наследуемся
#include "include/mrpc/i_driver_rp.h" // отправляем результаты
#include "include/mrpc/i_driver_rp_own.h" // отправляем результаты
//---------------------------------------------------------------------------


namespace mrpc
{

class t_server;
//---------------------------------------------------------------------------


using t_payload = std::vector<std::uint8_t>;


class tcp_connect
  : public mrpc::i_driver,
    public std::enable_shared_from_this<tcp_connect>
{

  public:

    tcp_connect(const std::string& astr_drv_id,
      boost::asio::ip::tcp::socket a_socket,
      t_server& a_server, 
      mrpc::i_driver_rp_own& a_drv_rp_own);

    // чтобы отслеживать удаление:
    ~tcp_connect();

    //
    void set_drv_rp(mrpc::i_driver_rp* ap_drv_rp) override;



    // запускаем первичную работу драйвера
    int start() override;
    // Забросить задание на выполнение:
    int put_request(uint32_t adw_func_id, std::vector<uint8_t>&& a_buf, uint32_t& adw_req_id) override;
    // Забросить ответ на задание:
    int put_answer(uint32_t adw_req_id, std::vector<uint8_t>&& a_buf) override;

    // выполнить блокирующий запрос
    int exec_request(uint32_t adw_req_id, std::vector<uint8_t>&& a_buf, 
      std::vector<uint8_t>& a_res) override;


    void handle_complete_msg();

  private:

    void logout(const std::string& astr_info);
    void log_err(const std::string& astr_info);



    // кладём задачу:
    int push_task_request(uint32_t adw_func_id, std::vector<uint8_t>&& avec_data, uint32_t& adw_task_id);

    // кладём ответ:
    int push_task_answer(uint32_t adw_task_id, std::vector<uint8_t>&& avec_data);


    // кладём задачу на выполнение, инициализируем промис для ожидания:
    int push_task_4block(uint32_t adw_func_id, std::vector<uint8_t>&& avec_data, std::future<t_payload>& a_future);



    // смотрим нужно ли начинать запись (выполнять из под крит секции):
    void check_start_write();


    // отработать, то что что-то ушло в сеть:
    void handle_written(std::size_t a_written);


    // читаем что-то из сети
    void do_read();

    // пишем то, что в выходной строке
    void do_write();




    // *********************** Данные ***************************************

    boost::asio::ip::tcp::socket m_socket;

    // блок про буфер записи. TODO: возможно потом будет блок отдельно для заголовка и отдельно для payload-а
    std::unique_ptr<mrpc::protocol::t_read_obj> m_p_read_obj;


    // данные для записи. Пока считаем что владеем ими. TODO: можно подумть и получать их как buf_view
    std::vector<uint8_t> m_write_buf;
    size_t m_write_buf_pos = 0; // todo: объединить в единый объект?

  
    // ссылка на основной сервер
    t_server& m_server;

    // приёмные части драйвера:
    mrpc::i_driver_rp* mp_drv_rp;
    mrpc::i_driver_rp_own& m_drv_rp_own;
    // ---------------------------------------------------------------------------------------

    


    // ---------------------------------------------------------------------------------------
    // блок про всякие отложенные задачи. Которые отослали, но ещё не получили ответа:
    std::unordered_map<uint32_t, std::unique_ptr<mrpc::t_defer_rec>> m_map_defer_tasks;
 
    // id последней отосланной задачи. Просто инкрементируем её
    uint32_t m_dwLastTaskID = {55}; // 55 - чтобы было виднее и проще отлаживать

    // для защиты данных буфера
    mutable std::mutex m_mutex;

    std::queue<std::unique_ptr<mrpc::t_cmd_record>> m_queue;

    // идентификация для логов
    std::string m_str_drv_id;

};
//---------------------------------------------------------------------------

} //mrpc
//---------------------------------------------------------------------------