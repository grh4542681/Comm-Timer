/*******************************************************
 * Copyright (C) For free.
 * All rights reserved.
 *******************************************************
 * @author   : Ronghua Gao
 * @date     : 2022-04-22 09:24
 * @file     : timer_rule.h
 * @brief    : d task scheduling rule base class.
 * @note     : Email - grh4542681@163.com
 * ******************************************************/
#ifndef __TIMER_RULE_HH__
#define __TIMER_RULE_HH__

#include <chrono>
#include "timer_return.hh"
#include "timer_wheel_scale.hh"

namespace xg::timer {

/**
* @brief - d task scheduling rule base class.
*/
class Rule {
public:
    /**
    * @brief - Base time type, accurate to nanoseconds.
    */
    using RefTimePoint = std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>;
public:
    Rule() { }
    virtual ~Rule() { }

    /**
    * @brief Valid - Whether the scheduling rules are correct under the given accuracy.
    *
    * @param [accuracy] - Timer wheel accuracy.
    *
    * @returns Bool 
    */
    virtual bool Valid(WheelAccuracy& accuracy) = 0;

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
    virtual std::tuple<Return, RefTimePoint> GetNextExprieTime(RefTimePoint&& reftime) = 0;
};

}

#endif
