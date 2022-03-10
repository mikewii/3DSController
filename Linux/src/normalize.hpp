#pragma once
#include "global.hpp"

#include <algorithm>


class Normalize
{
public:
    Normalize();

    s16 l_stick(s16 axis) const;
    s16 r_stick(s16 axis) const;
    u16 touch_x(const u16 pos_x) const { return std::min((static_cast<float>(pos_x) * this->touch_x_step), static_cast<float>(TOUCH_X_MAX)); }
    u16 touch_y(const u16 pos_y) const { return std::min((static_cast<float>(pos_y) * this->touch_y_step), static_cast<float>(TOUCH_Y_MAX)); }

private:
    float max = 127;
    float l_stick_prestep = 0;
    float r_stick_prestep = 0;
    float touch_x_step = 0;
    float touch_y_step = 0;

    s16 l_stick_step;
    s16 r_stick_step;
};
