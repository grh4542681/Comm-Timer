#include <algorithm>
#include "timer_log.h"
#include "timer_schedule_rule_crontab.h"

using namespace std::chrono_literals;

int main()
{
    xg::timer::ScheduleRuleCrontab sc("*/45,2022,2022-2032,2032-2042/3 */13,1,6-9 3 4 5 6     8");
    sc.GetNextExprieTime(std::chrono::system_clock::now());
    return 0;
}
