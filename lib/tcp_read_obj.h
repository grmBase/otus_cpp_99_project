//---------------------------------------------------------------------------
#pragma once
//---------------------------------------------------------------------------
#include "appl/sync_console.h"
//---------------------------------------------------------------------------




namespace mrpc {
namespace protocol {


// вспомогательная структура, просто чтобы было проще её создать/ удалить/ пересоздать
// когда начинается, завершается чтение очередной команды из сети
struct t_read_obj
{

  // Получить "указатель" куда писать дальше. todo: поискать что там есть в стандарте для этого std::span? glc::span?
  std::pair<uint8_t*, size_t> get_curr_data()
  {

    if(m_read_header) {
      return std::pair<uint8_t*, size_t>(((uint8_t*)&m_header) + m_index, sizeof(m_header) - m_index);
    }

    return std::pair(&(m_vec_payload[m_index]), m_vec_payload.size() - m_index);
  };
  //---------------------------------------------------------------------------


  void handle_readed(size_t a_readed)
  {

    if(m_read_header) {

      // смещаемся:
      m_index += a_readed;


      clog::logout("in handle_readed (header). readed: " + 
        std::to_string(a_readed) + ", total: " + std::to_string(m_index));
      

      // ошибка?
      if(m_index > sizeof(t_msg_header)) {
        clog::log_err("somehow overrun happened");
        return;
      }

      if(m_index == sizeof(t_msg_header)) {

        clog::logout("header readed");
        uint32_t payload_size = m_header.m_dw_buf_size;
        clog::logout("payload size: " + std::to_string(payload_size));

        // направляем на чтение нагрузки:
        m_vec_payload.resize(payload_size);
        m_read_header = false;
        m_index = 0;

        return;
      }

      //clog::log_err("should not be there");

      return;
    }


    // значит читаем нагрузку:
    // 
    // смещаемся:
    m_index += a_readed;

    clog::logout("in handle_readed (payload). readed: " +
      std::to_string(a_readed) + ", total: " + std::to_string(m_index));



  }
  //---------------------------------------------------------------------------

  bool is_ready() const
  {
    if (m_read_header) {
      return false;
    }

    if (m_vec_payload.size() != m_index) {
      return false;
    }

    return true;
  }
  //---------------------------------------------------------------------------

  void reset()
  {
    m_read_header = true;
    m_vec_payload.clear();
    m_index = 0;
  }
  //---------------------------------------------------------------------------


  bool m_read_header = true;

  // Заголовок
  t_msg_header m_header = {};

  // пользовательская нагрузка
  std::vector<uint8_t> m_vec_payload;


  // смещение куда читаем (или в заголовке или в нагрузке)
  size_t m_index = 0;
};
//---------------------------------------------------------------------------


} // protocol
} // mrpc