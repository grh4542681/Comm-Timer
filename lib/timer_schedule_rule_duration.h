/*******************************************************
 * Copyright (C) For free.
 * All rights reserved.
 *******************************************************
 * @author   : Ronghua Gao
 * @date     : 2022-04-22 09:25
 * @file     : timer_schedule_rule_duration.h
 * @brief    : Scheduling rules based on time duration.
 * @note     : Email - grh4542681@163.com
 * ******************************************************/
#ifndef __TIMER_SCHEDULE_RULE_DURATION_H__
#define __TIMER_SCHEDULE_RULE_DURATION_H__

#include "timer_wheel_scale.h"
#include "timer_schedule_rule.h"

namespace xg::timer {

/**
* @brief - Scheduling rules based on time duration.
*/
class ScheduleRuleDuration : public ScheduleRule {
public:
    ScheduleRuleDuration(std::chrono::nanoseconds&& nano);
    ScheduleRuleDuration(std::chrono::microseconds&& micro);
    ScheduleRuleDuration(std::chrono::milliseconds&& mill);
    ScheduleRuleDuration(std::chrono::seconds&& sec);
    ScheduleRuleDuration(std::chrono::minutes&& min);
    ScheduleRuleDuration(std::chrono::hours&& hour);
    ScheduleRuleDuration(std::chrono::days&& day);
    ScheduleRuleDuration(std::chrono::weeks&& week);
    ScheduleRuleDuration(std::chrono::months&& mon);
    ScheduleRuleDuration(std::chrono::years&& year);
    ~ScheduleRuleDuration();

    /**
    * @brief Valid - Inherited function(ScheduleRule).
    */
    bool Valid(WheelAccuracy& accuracy);
    /**
    * @brief Valid - Inherited function(ScheduleRule).
    */
    std::tuple<Return, WheelScale> GetNextExprieScale(RefTimePoint&& reftime, WheelAccuracy& accuracy);

    /**
    * @brief Valid - Inherited function(ScheduleRule).
    */
    std::tuple<Return, RefTimePoint> GetNextExprieTime(RefTimePoint&& reftime);

private:
    std::chrono::nanoseconds _duration_nano;
};

}

#endif
