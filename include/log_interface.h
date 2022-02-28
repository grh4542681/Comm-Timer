#ifndef __LOG_INTERFACE_H__
#define __LOG_INTERFACE_H__

#include <map>
#include <utility>
#include <chrono>
#include <thread>
#include <sstream>
#include <ctime>
#include <iomanip>

#include "log_priority.h"

namespace xg::timer::log {

typedef struct __Interface {
    void (*write) (Facility&& facility, Priority&& priority, std::string&& message);
} Interface;

}

#endif
