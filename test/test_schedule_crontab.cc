#include <algorithm>
#include "timer_log.h"
#include "timer_schedule_rule_crontab.h"

using namespace std::chrono_literals;

int main()
{
    xg::timer::ScheduleRuleCrontab sc("*/45,2022,2022-2032,2032-2042/3 */13,1,6-9 3 4 5 58     8");
    auto t = std::chrono::system_clock::to_time_t(std::get<1>(sc.GetNextExprieTime()));
    xg::timer::Log::Info("TEST", std::put_time(std::localtime(&t), "%F %T"));
    t = std::chrono::system_clock::to_time_t(std::get<1>(sc.GetNextExprieTime()));
    xg::timer::Log::Info("TEST", std::put_time(std::localtime(&t), "%F %T"));
    return 0;
}
