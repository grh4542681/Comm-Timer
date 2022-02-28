#ifndef __LOG_WRITER_H__
#define __LOG_WRITER_H__

#include <map>
#include <utility>
#include <chrono>
#include <thread>
#include <sstream>
#include <ctime>
#include <iostream>
#include <iomanip>

#include "log_wapper.h"

namespace xg::timer {

class Log {
public:
    template <typename ... Args> static void Emergency(std::string&& fac_name, Args&& ... args) {
        log::Wapper::Instance().Log(log::Facility(std::move(fac_name)), log::Priority::Emergency, std::forward<Args>(args)...);
    }
};

}

#endif
