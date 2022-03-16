//---------------------------------------------------------------------------
#pragma once
//---------------------------------------------------------------------------



namespace mrpc {


//---------------------------------------------------------------------------
class i_listen_rp;
class i_driver_rp_own;
//---------------------------------------------------------------------------


// Класс "обёртка" вокруг boost::asio ядра, с функциями запуска прослушки или
// коннекта к другому серверу
class i_server
{
  public:

    // Функция конструирующая класс:
    static i_server* create(uint16_t aun_port, unsigned int aun_num_of_threads,
      mrpc::i_listen_rp& a_listener, mrpc::i_driver_rp_own& a_drv_rp);



    virtual ~i_server() {};


    // запускаем нити
    virtual int start_threads() = 0;
    // запускаем прослушивание
    virtual int start_listen() = 0;


    // коннектимся как клиент:
    virtual int client_connect(const std::string& astr_host, const std::string& astr_port) = 0;


    // останавливает нитки контекста
    virtual int stop() = 0;

};
//---------------------------------------------------------------------------




} // mrpc