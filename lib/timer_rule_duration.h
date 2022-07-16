/*******************************************************
 * Copyright (C) For free.
 * All rights reserved.
 *******************************************************
 * @author   : Ronghua Gao
 * @date     : 2022-04-22 09:25
 * @file     : timer_rule_duration.h
 * @brief    : Scheduling rules based on time duration.
 * @note     : Email - grh4542681@163.com
 * ******************************************************/
#ifndef __TIMER_RULE_DURATION_H__
#define __TIMER_RULE_DURATION_H__

#include "timer_wheel_scale.h"
#include "timer__rule.h"

namespace xg::timer {

/**
* @brief - Scheduling rules based on time duration.
*/
class RuleDuration : public Rule {
public:
    RuleDuration(std::chrono::nanoseconds&& nano);
    RuleDuration(std::chrono::microseconds&& micro);
    RuleDuration(std::chrono::milliseconds&& mill);
    RuleDuration(std::chrono::seconds&& sec);
    RuleDuration(std::chrono::minutes&& min);
    RuleDuration(std::chrono::hours&& hour);
    RuleDuration(std::chrono::days&& day);
    RuleDuration(std::chrono::weeks&& week);
    RuleDuration(std::chrono::months&& mon);
    RuleDuration(std::chrono::years&& year);
    ~RuleDuration();

    /**
    * @brief Valid - Inherited function(Rule).
    */
    bool Valid(WheelAccuracy& accuracy);
    /**
    * @brief Valid - Inherited function(Rule).
    */
    std::tuple<Return, WheelScale> GetNextExprieScale(RefTimePoint&& reftime, WheelAccuracy& accuracy);

    /**
    * @brief Valid - Inherited function(Rule).
    */
    std::tuple<Return, RefTimePoint> GetNextExprieTime(RefTimePoint&& reftime);

private:
    std::chrono::nanoseconds _duration_nano;
};

}

#endif
