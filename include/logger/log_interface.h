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

/**
* @brief - XG-timer logger interface.
*/
typedef struct __Interface {
    /**
    * @brief - Function ptr for log write
    *
    * @param [facility] - log facility.
    * @param [priority] - log priority.
    * @param [message] - log message.
    */
    void (*write) (Facility&& facility, Priority&& priority, std::string&& message);
} Interface;

}

#endif
