#include "timer_log.hh"
#include "timer_return.hh"
#include "timer_wheel_accuracy.hh"
#include "timer_rule_duration.hh"

namespace xg::timer {

RuleDuration::RuleDuration(std::chrono::nanoseconds&& nano)
    :_duration_nano(nano) { }

RuleDuration::RuleDuration(std::chrono::microseconds&& micro)
{
    _duration_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(micro);
}
RuleDuration::RuleDuration(std::chrono::milliseconds&& mill)
{
    _duration_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(mill);
}
RuleDuration::RuleDuration(std::chrono::seconds&& sec)
{
    _duration_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(sec);
}
RuleDuration::RuleDuration(std::chrono::minutes&& min)
{
    _duration_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(min);
}
RuleDuration::RuleDuration(std::chrono::hours&& hour)
{
    _duration_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(hour);
}
RuleDuration::RuleDuration(std::chrono::days&& day)
{
    _duration_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(day);
}
RuleDuration::RuleDuration(std::chrono::weeks&& week)
{
    _duration_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(week);
}
RuleDuration::RuleDuration(std::chrono::months&& mon)
{
    _duration_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(mon);
}
RuleDuration::RuleDuration(std::chrono::years&& year)
{
    _duration_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(year);
}

RuleDuration::~RuleDuration() { }

bool RuleDuration::Valid(WheelAccuracy& accuracy)
{
    if (!accuracy.Valid()) {
        return false;
    }
    if (_duration_nano < accuracy.GetAccuracy()) {
        return false;
    }
    if ((_duration_nano.count() % accuracy.GetAccuracy().count() != 0)) {
        return false;
    }
    return true;
}

std::tuple<Return, WheelScale>
RuleDuration::GetNextExprieScale(RefTimePoint&& reftime, WheelAccuracy& accuracy)
{
    std::ignore = reftime;
    if (!Valid(accuracy)) {
        return std::make_tuple(Return(Return::ESCHEDULE_RULE_INVALID), WheelScale());
    }
    return std::make_tuple(Return(Return::SUCCESS), WheelScale(_duration_nano.count() / accuracy.GetAccuracy().count()));
}

std::tuple<Return, RuleDuration::RefTimePoint>
RuleDuration::GetNextExprieTime(RefTimePoint&& reftime)
{
    return {Return::SUCCESS, (reftime + _duration_nano)};
}

}
