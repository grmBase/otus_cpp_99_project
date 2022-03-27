//---------------------------------------------------------------------------
#pragma once
//---------------------------------------------------------------------------
#include <vector>
#include <cstdint> // uint8_t
#include <string>
//---------------------------------------------------------------------------



namespace mrpc {
namespace protocol {



// Тип команды "внутренняя" типа keepalive или обычная "пользовательская"
enum class cmd_type : uint8_t
{
  undefined = 0xff,
  custom = 0,        // регулярные посылки пользовательского уровня
  rejected = 1,      // тип посылки с аварийным ответом: что-то не получилось запустить на стороне сервера
  keep_alive = 2,    // служебные посылки keep alive
};
//---------------------------------------------------------------------------


// Макрос, для максимальной упаковки структуры (далее мы на неё делаешь sizeof())
#ifdef __GNUC__
#define MRPC_PACK( __Declaration__ ) __Declaration__ __attribute__((packed));
#endif

#ifdef _MSC_VER
#define MRPC_PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__; __pragma( pack(pop))
#endif



// Значение первого байта в посылке. Константа, для проверки, что это "наш" клиент
const uint8_t c_bSign = 0x31;
//---------------------------------------------------------------------------



// Структура заголовка сообщения
MRPC_PACK(
struct t_msg_header
{

  // создаём "пользовательский" хидер
  static t_msg_header create_custom_request(bool af_is_request, uint32_t adw_req_id, 
    uint32_t adw_func_id, uint32_t adw_payload_size)
  {
    t_msg_header result;
    result.m_b_cmd_id = cmd_type::custom;
    result.m_is_request = af_is_request;
    result.m_dw_task_id = adw_req_id;

    result.m_dw_func_id = adw_func_id;
    result.m_dw_buf_size = adw_payload_size;

    return result;
  };
  //---------------------------------------------------------------------------


  uint8_t m_b_sign = c_bSign;    // просто определённое значение
  uint16_t m_w_sequence = 0x32;  // пока не используется
  uint8_t m_b_reserved = 0x33;   // пока не используется


  bool m_is_request = true;      // флаг если ли посылка запрос или ответ
  cmd_type m_b_cmd_id = cmd_type::undefined;    // тип команды если это запрос и -1 если это ответ
  uint32_t m_dw_task_id = std::numeric_limits<uint32_t>::max();  // ID запроса

  uint32_t m_dw_func_id = 0x00; // id пользовательской функции. В реальной задаче скорее всего будет строкой

  uint32_t m_dw_buf_size = std::numeric_limits<uint32_t>::max(); // размер последующей нагрузки
}
)

//---------------------------------------------------------------------------



// Сериализуем. А может и не надо так...
// std::vector<uint8_t> serialize_custom_msg(bool af_is_request, const std::vector<uint8_t>& a_custom_data);

inline static std::string print_cmd_type(cmd_type a_cmd_type)
{
  switch(a_cmd_type)
  {
    case cmd_type::undefined:
      return "undefined";
    //break;

    case cmd_type::custom:
      return "custom";
    //break;

    case cmd_type::keep_alive:
      return "keep_alive";
    //break;

    default:
      return "<! should not be there !>";
    //break;
  }
}
//---------------------------------------------------------------------------


inline static std::string print_header(const t_msg_header& a_header)
{
  std::string str_info;

  str_info = "cmdtype: " + print_cmd_type(a_header.m_b_cmd_id);

  if(a_header.m_is_request) {
    str_info += ", it is command, task_id: " + std::to_string(a_header.m_dw_task_id);
  }
  else {
    str_info += ", it is reply, task_id: " + std::to_string(a_header.m_dw_task_id);
  };


  if(a_header.m_is_request) {
    str_info += ", func_id: " + std::to_string(a_header.m_dw_func_id);
  }
  else {
    assert(a_header.m_dw_func_id == -1); // в ответе имя функции не передаём
  }

  return str_info;
};
//---------------------------------------------------------------------------


} // protocol
} // mrpc
