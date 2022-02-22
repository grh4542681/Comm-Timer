#ifndef __LOG_INTERFACE_H__
#define __LOG_INTERFACE_H__

#include <map>
#include <utility>
#include <chrono>
#include <thread>
#include <sstream>
#include <ctime>
#include <iomanip>

#include "log_wapper.h"

namespace xg::timer {

class Log {
public:
    static void Emergency(Args&& ... args) {
        log::Wapper.Instance().Log(log::Priority::Emergency, std::forward<Args>(args)...);
    }
};

#endif
