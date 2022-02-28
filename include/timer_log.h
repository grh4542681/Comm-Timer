#ifndef __TIMER_LOG_H__
#define __TIMER_LOG_H__

#include "log_writer.h"

#define TIMER_OS_INFO(Args...) \
            ::xg::timer::Log::Info("OS", Args)

#endif
