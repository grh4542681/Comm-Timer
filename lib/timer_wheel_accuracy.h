#ifndef __TIMER_WHEEL_ACCURACY_H__
#define __TIMER_WHEEL_ACCURACY_H__

#include <mutex>

namespace xg::timer {

/**
* @brief - Time wheel accuracy class
*/
class WheelAccuracy {
public:
    static WheelAccuracy& Instance() {
        static WheelAccuracy instance;
        return instance;
    }

    bool Vaild() {
        std::scoped_lock lock(_mutex);
        return (_accuracy.count() > 0);
    }

    std::chrono::nanoseconds GetAccuracy() {
        std::scoped_lock lock(_mutex);
        return _accuracy;
    }

private:
    WheelAccuracy() : _accuracy(1000000) { }
    WheelAccuracy(const WheelAccuracy&);
    WheelAccuracy& operator=(const WheelAccuracy&);

    WheelAccuracy& SetAccuracy(std::chrono::nanoseconds&& accuracy) {
        std::scoped_lock lock(_mutex);
        _accuracy = accuracy;
        return (*this);
    }

private:
    std::chrono::nanoseconds _accuracy;
    std::mutex _mutex;
};

}

#endif
