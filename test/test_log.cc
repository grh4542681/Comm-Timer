#include "log_writer.h"
int main()
{
    xg::timer::Log::Emergency("TEST", "test log");
    xg::timer::Log::Error("TEST", "test log");
    xg::timer::Log::Info("TEST", "test log");
    xg::timer::Log::Debug("TEST", "test log");
    return 0;
}
