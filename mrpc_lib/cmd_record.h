//---------------------------------------------------------------------------
#pragma once
//---------------------------------------------------------------------------
#include "protocol.h" // cmd_type
//---------------------------------------------------------------------------



namespace mrpc
{


/*
    Запись об одном запросе/ ответе 
*/
struct t_cmd_record
{
  public:

    t_cmd_record(mrpc::protocol::cmd_type a_cmd_type, bool af_is_request, uint32_t adw_req_id,
       uint32_t adw_func_id, std::vector<uint8_t>&& avec_data)
    : m_cmd_type(a_cmd_type),
      m_is_request(af_is_request),
      m_dw_req_id(adw_req_id),
      m_dw_func_id(adw_func_id),
      m_vec_data(avec_data)
    {};

    virtual ~t_cmd_record(){};



  // ************ Данные *****************
    // служебное или пользовательское сообщение
    mrpc::protocol::cmd_type m_cmd_type;

    bool m_is_request;

    // id запроса
    uint32_t m_dw_req_id;

    // id функции
    uint32_t m_dw_func_id;

    // пользовательские данные
    std::vector<uint8_t> m_vec_data;

};
//---------------------------------------------------------------------------

}; //mrpc