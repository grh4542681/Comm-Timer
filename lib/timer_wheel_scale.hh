#ifndef __TIMER_WHEEL_SCALE_HH__
#define __TIMER_WHEEL_SCALE_HH__

#include "timer_wheel_accuracy.hh"

namespace xg::timer {

/**
* @brief - Timer wheel scales
*/
class WheelScale {
public:
    WheelScale() : _scale_num(0) { }
    WheelScale(const long long scale) : _scale_num(scale) { }
    WheelScale(const WheelScale&& other) : _scale_num(other._scale_num) { }

    /**
    * @brief GetNum - Get wheel scales.
    *
    * @returns  Number of scales.
    */
    long long GetNum() {
        return _scale_num;
    }

    /**
    * @brief SetNum - Set wheel scales.
    *
    * @param [scale] - Number of scales.
    *
    * @returns  Self.
    */
    WheelScale& SetNum(long long scale) {
        _scale_num = scale;
        return (*this);
    }

private:
    long long _scale_num;
};

}

#endif
