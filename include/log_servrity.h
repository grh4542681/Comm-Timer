#ifndef __TIMER_LOG_SERVRITY_H__
#define __TIMER_LOG_SERVRITY_H__

namespace ::comm::timer {

enum class LogServrity : int {
    Emergency = 0,
    Alert,
    Critical,
    Error,
    Warning,
    Notice,
    Info,
    Debug,
    Debug2,
    Debug3,
};

}

#endif
