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

    std::promise<t_payload> m_promise;

};
//---------------------------------------------------------------------------



}; //mrpc