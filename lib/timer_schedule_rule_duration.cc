#include "timer_log.h"
#include "timer_return.h"
#include "timer_wheel_accuracy.h"
#include "timer_schedule_rule_duration.h"

namespace xg::timer {

ScheduleRuleDuration::ScheduleRuleDuration(std::chrono::nanoseconds&& nano)
    :_duration_nano(nano) { }

ScheduleRuleDuration::ScheduleRuleDuration(std::chrono::microseconds&& micro)
{
    _duration_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(micro);
}
ScheduleRuleDuration::ScheduleRuleDuration(std::chrono::milliseconds&& mill)
{
    _duration_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(mill);
}
ScheduleRuleDuration::ScheduleRuleDuration(std::chrono::seconds&& sec)
{
    _duration_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(sec);
}
ScheduleRuleDuration::ScheduleRuleDuration(std::chrono::minutes&& min)
{
    _duration_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(min);
}
ScheduleRuleDuration::ScheduleRuleDuration(std::chrono::hours&& hour)
{
    _duration_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(hour);
}
ScheduleRuleDuration::ScheduleRuleDuration(std::chrono::days&& day)
{
    _duration_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(day);
}
ScheduleRuleDuration::ScheduleRuleDuration(std::chrono::weeks&& week)
{
    _duration_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(week);
}
ScheduleRuleDuration::ScheduleRuleDuration(std::chrono::months&& mon)
{
    _duration_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(mon);
}
ScheduleRuleDuration::ScheduleRuleDuration(std::chrono::years&& year)
{
    _duration_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(year);
}

ScheduleRuleDuration::~ScheduleRuleDuration() { }

bool ScheduleRuleDuration::Valid(WheelAccuracy& accuracy)
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
ScheduleRuleDuration::GetNextExprieScale(RefTimePoint&& reftime, WheelAccuracy& accuracy)
{
    std::ignore = reftime;
    if (!Valid(accuracy)) {
        return std::make_tuple(Return(Return::ESCHEDULE_RULE_INVALID), WheelScale());
    }
    return std::make_tuple(Return(Return::SUCCESS), WheelScale(_duration_nano.count() / accuracy.GetAccuracy().count()));
}

ScheduleRuleDuration::RefTimePoint
ScheduleRuleDuration::GetNextExprieTime(RefTimePoint&& reftime)
{
    return (reftime + _duration_nano);
}

}
