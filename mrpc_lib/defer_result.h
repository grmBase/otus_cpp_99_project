//---------------------------------------------------------------------------
#pragma once
//---------------------------------------------------------------------------


namespace mrpc
{

  using t_payload = std::vector<std::uint8_t>;

/*
    Структура, чтобы передавать результаты вызова при клиентских
  блокирующих запросах через future/promise
*/

class t_defer_result
{
  public:

    // запись если был промис
    t_defer_result(int a_n_res_code, mrpc::t_payload&& a_payload)
      : m_n_res_code(a_n_res_code),
        m_payload(std::move(a_payload))
    {
    };

    // нужен похоже где-то при создании промиса:
    t_defer_result()
     : m_n_res_code(-1)
    {
    };


    /*
    t_defer_result(t_defer_result&& a_second) {
      m_n_res_code = a_second.m_n_res_code;
      m_payload = std::move(a_second.m_payload);
    }

    t_defer_result& operator = (t_defer_result&& a_second) noexcept
    {
      m_n_res_code = a_second.m_n_res_code;
      m_payload = std::move(a_second.m_payload);
    };
    */

    virtual ~t_defer_result(){};

  //private:

    // запись, если промиса не было:
    //t_defer_result()
    //{
    //};


    // результат: 0 - прошло, остальные коды - если что-то не получилось на уровне "подсистемы mrpc"
    // (т.е. это не пользовательская ошибка). TODO: сделать отдельный enum, чтобы было понятно "уровень" ошибки
    int m_n_res_code;

    // Пользовательская нагрузка, если результат был положительный. Иначе "не смотрим" сюда
    t_payload m_payload;

};
//---------------------------------------------------------------------------



}; //mrpc