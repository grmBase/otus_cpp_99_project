//---------------------------------------------------------------------------
#include <iostream>
#include <thread> //this_thread
//---------------------------------------------------------------------------
#include "include/sconsole/sync_console.h"
//---------------------------------------------------------------------------



// пока включим отладку:
#define DBG_LOGGING


std::string make_time()
{
  auto timepoint = std::chrono::system_clock::now();
  std::time_t some = std::chrono::system_clock::to_time_t(timepoint);

  auto fine = std::chrono::time_point_cast<std::chrono::milliseconds>(timepoint);

  tm parts{};

#if defined(_WIN32)
  localtime_s(&parts, &some);
#else
  parts = *localtime(&some);
#endif

  auto milseconds = fine.time_since_epoch().count() % 1000;


  const size_t un_size = 32;
  char buf[un_size] = {};


  // sprintf разные хотят под разные платформы:
  #ifdef _WIN32
    sprintf_s(buf, un_size, "%02d:%02d:%02d.%03d",
      parts.tm_hour, parts.tm_min, parts.tm_sec, milseconds);
  #elif defined(__unix)
    snprintf(buf, un_size, "%02d:%02d:%02d.%03d",
      parts.tm_hour, parts.tm_min, parts.tm_sec, (int)milseconds);
  #endif



  return buf;
}
//---------------------------------------------------------------------------



void t_sync_console::log_info_inst(std::string_view astr_info)
{
  std::lock_guard lock(m_mutex);

  std::cout << make_time() << " [trk] [tid:" << std::this_thread::get_id() << "] " << astr_info << std::endl;
}
//---------------------------------------------------------------------------


void t_sync_console::logout_inst(std::string_view astr_info)
{
#ifdef DBG_LOGGING
  log_info_inst(astr_info);
#endif
}
//---------------------------------------------------------------------------


// 
void t_sync_console::log_err_inst(std::string_view astr_info)
{

#ifdef DBG_LOGGING
  std::lock_guard lock(m_mutex);

  std::cerr << make_time() << " [err] [tid:" << std::this_thread::get_id() << "] " << astr_info << std::endl;
#endif
}
//---------------------------------------------------------------------------



void clog::logout(std::string_view astr_info)
{
#ifdef DBG_LOGGING
  gp_log->logout_inst(astr_info);
#endif
}
//---------------------------------------------------------------------------

void clog::log_info(std::string_view astr_info)
{
#ifdef DBG_LOGGING
  gp_log->log_info_inst(astr_info);
#endif
}
//---------------------------------------------------------------------------

void clog::log_err(std::string_view astr_info)
{
#ifdef DBG_LOGGING
  gp_log->log_err_inst(astr_info);
#endif
}
//---------------------------------------------------------------------------


// чтобы выводилось всегда - там по заданию нужно в консоль
void clog::log_info_always(std::string_view astr_info)
{
  gp_log->log_info_inst(astr_info);
}
//---------------------------------------------------------------------------
