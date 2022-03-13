//---------------------------------------------------------------------------
#pragma once
//---------------------------------------------------------------------------



namespace mrpc {


// Интерфейс "владельца" драйверов
class i_driver_rp_own
{
  public:

    // запрос на удаление драйвера:
    virtual void request_to_del_drv(i_driver* ap_drv) = 0;
};
//---------------------------------------------------------------------------


} // mrpc