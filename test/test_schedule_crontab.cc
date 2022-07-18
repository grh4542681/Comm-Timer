#include <algorithm>
#include "timer_log.hh"
#include "timer_rule_crontab.hh"

using namespace std::chrono_literals;

int main()
{
    xg::timer::RuleCrontab sc("*/45,2022,2022-2032,2032-2042/3 */13,1,6-9 3 4 */3 */60 30");
    auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    xg::timer::Log::Info("TEST", std::put_time(std::gmtime(&t), "%F %T"));
    t = std::chrono::system_clock::to_time_t(std::get<1>(sc.GetNextExprieTime()));
    xg::timer::Log::Info("TEST", std::put_time(std::gmtime(&t), "%F %T"));
    t = std::chrono::system_clock::to_time_t(std::get<1>(sc.GetNextExprieTime()));
    xg::timer::Log::Info("TEST", std::put_time(std::gmtime(&t), "%F %T"));
    return 0;
}
