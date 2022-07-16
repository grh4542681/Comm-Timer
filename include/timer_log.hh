#ifndef __TIMER_LOG_HH__
#define __TIMER_LOG_HH__

#include "log_writer.hh"

#define TIMER_OS_INFO(Args...) \
            ::xg::timer::Log::Info("OS", Args)

#define TIMER_RULE_DEBUG(Args...) \
            ::xg::timer::Log::Debug("RULE", Args)

#define TIMER_RULE_INFO(Args...) \
            ::xg::timer::Log::Info("RULE", Args)

#define TIMER_RULE_ERROR(Args...) \
            ::xg::timer::Log::Error("RULE", Args)

#endif
