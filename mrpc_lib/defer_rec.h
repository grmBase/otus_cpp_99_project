//---------------------------------------------------------------------------
#pragma once
//---------------------------------------------------------------------------
#include <future>
//---------------------------------------------------------------------------
#include "tcp_drv.h"      // t_payload - возможно todo вынести в отдельный хидер
#include "defer_result.h" // контейнер с результатом
//---------------------------------------------------------------------------


namespace mrpc
{


/*
    Запись о незавершённом вызове
*/
class t_defer_rec
{
  public:

    // запись если был промис
    t_defer_rec(std::future<t_defer_result>& a_future)
    {
      a_future = m_promise.get_future();
    };

    // запись, если прописа не было:
    t_defer_rec()
    {
    };


    void set_promise(t_defer_result&& a_result)
    {
      m_promise.set_value(a_result);
    };


    virtual ~t_defer_rec(){};

  private:

    std::promise<t_defer_result> m_promise;

};
//---------------------------------------------------------------------------



}; //mrpc