#include "log_writer.h"
int main()
{
    xg::timer::Log::Emergency("TEST", "test log");
    return 0;
}
