#pragma once
#include "global.hpp"

#include <3ds.h>
#include <algorithm>

class Normalize
{
public:
    Normalize();

    Packet_lite_v1 v1(const u32 _keys, const touchPosition _touch, const circlePosition _leftstick, const circlePosition _rightstick) const;
    Packet_lite_v2 v2(const u32 _keys, const touchPosition _touch, const circlePosition _leftstick, const circlePosition _rightstick) const;

    s16 l_stick(const s16 axis) const { return std::min((static_cast<float>(axis) / this->l_stick_step), this->max); }
    s16 r_stick(const s16 axis) const { return std::min((static_cast<float>(axis) / this->r_stick_step), this->max); }
    u16 touch_x(const u16 pos_x) const { return std::min((static_cast<float>(pos_x) / this->touch_x_step), this->max); }
    u16 touch_y(const u16 pos_y) const { return std::min((static_cast<float>(pos_y) / this->touch_y_step), this->max); }

private:
    float max = 127;
    float l_stick_step = 0;
    float r_stick_step = 0;
    float touch_x_step = 0;
    float touch_y_step = 0;
};
