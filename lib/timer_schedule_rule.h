/*******************************************************
 * Copyright (C) For free.
 * All rights reserved.
 *******************************************************
 * @author   : Ronghua Gao
 * @date     : 2022-04-22 09:24
 * @file     : timer_schedule_rule.h
 * @brief    : Scheduled task scheduling rule base class.
 * @note     : Email - grh4542681@163.com
 * ******************************************************/
#ifndef __TIMER_SCHEDULE_RULE_H__
#define __TIMER_SCHEDULE_RULE_H__

#include <chrono>
#include "timer_return.h"
#include "timer_wheel_scale.h"

namespace xg::timer {

/**
* @brief - Scheduled task scheduling rule base class.
*/
class ScheduleRule {
public:
    /**
    * @brief - Base time type, accurate to nanoseconds.
    */
    using RefTimePoint = std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>;
public:
    ScheduleRule() { }
    virtual ~ScheduleRule() { }

    /**
    * @brief Vaild - Whether the scheduling rules are correct under the given accuracy.
    *
    * @param [accuracy] - Timer wheel accuracy.
    *
    * @returns Bool 
    */
    virtual bool Vaild(WheelAccuracy& accuracy) = 0;

    /**
    * @brief GetNextExprieScale - Based on the given time and accuracy,
    *                             obtain the time wheel scale from the next scheduling.
    *
    * @param [reftime] - Base time.
    * @param [accuracy] - Timer wheel accuracy.
    *
    * @returns  Tuple of Return class & WheelScale.
    */
    virtual std::tuple<Return, WheelScale> GetNextExprieScale(RefTimePoint&& reftime, WheelAccuracy& accuracy) = 0;

    /**
    * @brief GetNextExprieTime - Based on the given time,
    *                            obtain the next time from the next scheduling.
    *
    * @param [reftime] - Base time.
    *
    * @returns Next time. 
    */
    virtual RefTimePoint GetNextExprieTime(RefTimePoint&& reftime) = 0;
};

}

#endif
