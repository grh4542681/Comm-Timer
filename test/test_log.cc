#include "timer_log.hh"
int main()
{
    xg::timer::Log::Emergency("TEST", "test log");
    xg::timer::Log::Error("TEST", "test log");
    xg::timer::Log::Info("TEST", "test log");
    xg::timer::Log::Debug("TEST", "test log");

    TIMER_OS_INFO("test xg-timer os log");
    return 0;
}
