#ifndef __LOG_WRITER_HH__
#define __LOG_WRITER_HH__

#include <iostream>

#include "log_wapper.hh"

namespace xg::timer {

/**
* @brief - XG-timer log class
*/
class Log {
public:
    template <typename ... Args> static void Emergency(std::string&& fac_name, Args&& ... args) {
        log::Wapper::Instance().Log(log::Facility(std::move(fac_name)), log::Priority::Emergency, std::forward<Args>(args)...);
    }
    template <typename ... Args> static void Alert(std::string&& fac_name, Args&& ... args) {
        log::Wapper::Instance().Log(log::Facility(std::move(fac_name)), log::Priority::Alert, std::forward<Args>(args)...);
    }
    template <typename ... Args> static void Critical(std::string&& fac_name, Args&& ... args) {
        log::Wapper::Instance().Log(log::Facility(std::move(fac_name)), log::Priority::Critical, std::forward<Args>(args)...);
    }
    template <typename ... Args> static void Error(std::string&& fac_name, Args&& ... args) {
        log::Wapper::Instance().Log(log::Facility(std::move(fac_name)), log::Priority::Error, std::forward<Args>(args)...);
    }
    template <typename ... Args> static void Warning(std::string&& fac_name, Args&& ... args) {
        log::Wapper::Instance().Log(log::Facility(std::move(fac_name)), log::Priority::Warning, std::forward<Args>(args)...);
    }
    template <typename ... Args> static void Notice(std::string&& fac_name, Args&& ... args) {
        log::Wapper::Instance().Log(log::Facility(std::move(fac_name)), log::Priority::Notice, std::forward<Args>(args)...);
    }
    template <typename ... Args> static void Info(std::string&& fac_name, Args&& ... args) {
        log::Wapper::Instance().Log(log::Facility(std::move(fac_name)), log::Priority::Info, std::forward<Args>(args)...);
    }
    template <typename ... Args> static void Debug(std::string&& fac_name, Args&& ... args) {
        log::Wapper::Instance().Log(log::Facility(std::move(fac_name)), log::Priority::Debug, std::forward<Args>(args)...);
    }
    template <typename ... Args> static void Debug2(std::string&& fac_name, Args&& ... args) {
        log::Wapper::Instance().Log(log::Facility(std::move(fac_name)), log::Priority::Debug2, std::forward<Args>(args)...);
    }
    template <typename ... Args> static void Debug3(std::string&& fac_name, Args&& ... args) {
        log::Wapper::Instance().Log(log::Facility(std::move(fac_name)), log::Priority::Debug3, std::forward<Args>(args)...);
    }
};

}

#endif
