#include <algorithm>
#include "timer_log.h"
#include "timer_rule_duration.h"

using namespace std::chrono_literals;

int main()
{
    xg::timer::RuleDuration sd(std::chrono::seconds(10));
    std::time_t t = std::chrono::system_clock::to_time_t(std::get<1>(sd.GetNextExprieTime(std::chrono::system_clock::now())));
    xg::timer::Log::Info("TEST", std::put_time(std::localtime(&t), "%F %T"));
    sleep(2);
    t = std::chrono::system_clock::to_time_t(std::get<1>(sd.GetNextExprieTime(std::chrono::system_clock::now())));
    xg::timer::Log::Info("TEST", std::put_time(std::localtime(&t), "%F %T"));
    xg::timer::Log::Info("TEST", std::get<1>(sd.GetNextExprieScale(std::chrono::system_clock::now(), xg::timer::WheelAccuracy::Instance())).GetNum());

    return 0;
}
