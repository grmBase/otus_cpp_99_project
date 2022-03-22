//---------------------------------------------------------------------------
#pragma once
//---------------------------------------------------------------------------
#include <string>
#include <string_view>
#include <mutex>
//---------------------------------------------------------------------------
class t_sync_console;
//---------------------------------------------------------------------------

// ссылка на единый лог
extern t_sync_console* gp_log;
//---------------------------------------------------------------------------



namespace clog
{
  void logout(std::string_view astr_info);
  void log_info(std::string_view astr_info);
  void log_err(std::string_view astr_info);

  // чтобы выводилось всегда - там по заданию нужно в консоль
  void log_info_always(std::string_view astr_info);
}



class t_sync_console
{
public:

  void logout_inst(std::string_view astr_info);

  void log_err_inst(std::string_view astr_info);



  void log_info_inst(std::string_view astr_info);

private:

  // мьютекс синхронизации std::cout
  mutable std::mutex m_mutex; // оставил один на всех - иначе cout и cerr в одной консоли смешиваются
};
//---------------------------------------------------------------------------