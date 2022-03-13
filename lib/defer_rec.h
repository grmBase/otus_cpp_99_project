//---------------------------------------------------------------------------
#pragma once
//---------------------------------------------------------------------------
#include <future>
//---------------------------------------------------------------------------
#include "tcp_connect.h" // t_payload - возможно todo вынести в отдельный хидер
//---------------------------------------------------------------------------


namespace mrpc
{


  using t_payload = std::vector<std::uint8_t>;

/*
    Запись о незавершённом вызове
*/
class t_defer_rec
{
  public:

    // запись если был промис
    t_defer_rec(std::future<t_payload>& a_future)
    {
      a_future = m_promise.get_future();
    };

    // запись, если прописа не было:
    t_defer_rec()
    {
    };


    void set_promise(t_payload&& a_payload)
    {
      m_promise.set_value(a_payload);
    };


    virtual ~t_defer_rec(){};

  private:

    // пока ерунда. Потом здесь будет что-то на чём можно ждать. future/promise?
    std::string m_str_info;

    std::promise<t_payload> m_promise;

};
//---------------------------------------------------------------------------



}; //mrpc