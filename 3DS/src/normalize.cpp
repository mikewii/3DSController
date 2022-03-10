#include "normalize.hpp"

#include <string.h>

Normalize::Normalize()
{
    this->l_stick_step = L_STICK_MAX / this->max;
    this->r_stick_step = R_STICK_MAX / this->max;

    this->touch_x_step = TOUCH_X_MAX / this->max;
    this->touch_y_step = TOUCH_Y_MAX / this->max;
}

Packet_lite_v1 Normalize::v1(const u32 _keys, const touchPosition _touch, const circlePosition _leftstick, const circlePosition _rightstick) const
{
    Packet_lite_v1 p;
    memset(&p, 0, sizeof(p));

    // check only first 16 keys
    for (int i = 0; i < 12; i++)
        if (_keys & 1 << i)
            p.keys |= 1 << i;

    if (_keys & 1 << 14)    // ZL pressed
        p.keys |= 1 << 12;
    if (_keys & 1 << 15)    // ZR pressed
        p.keys |= 1 << 13;

    p.touch_x = _touch.px;
    p.touch_y = _touch.py;

    if ((_leftstick.dx > DEADZONE || _leftstick.dx < DEADZONE) && (_leftstick.dy > DEADZONE || _leftstick.dy < DEADZONE)) {
        p.lx = this->l_stick(_leftstick.dx);
        p.ly = this->l_stick(_leftstick.dy);
    }

    if ((_rightstick.dx > DEADZONE || _rightstick.dx < DEADZONE) && (_rightstick.dy > DEADZONE || _rightstick.dy < DEADZONE)) {
        p.rx = this->r_stick(_rightstick.dx);
        p.ry = this->r_stick(_rightstick.dy);
    }

    return p;
}

Packet_lite_v2 Normalize::v2(const u32 _keys, const touchPosition _touch, const circlePosition _leftstick, const circlePosition _rightstick) const
{
    Packet_lite_v2 p;
    memset(&p, 0, sizeof(p));

    // check only first 16 keys
    for (int i = 0; i < 12; i++)
        if (_keys & 1 << i)
            p.keys |= 1 << i;

    if (_keys & 1 << 14)    // ZL pressed
        p.keys |= 1 << 12;
    if (_keys & 1 << 15)    // ZR pressed
        p.keys |= 1 << 13;

    p.touch_x = this->touch_x(_touch.px);
    p.touch_y = this->touch_y(_touch.py);

    if ((_leftstick.dx > DEADZONE || _leftstick.dx < DEADZONE) && (_leftstick.dy > DEADZONE || _leftstick.dy < DEADZONE)) {
        p.lx = _leftstick.dx;
        p.ly = _leftstick.dy;
    }

    if ((_rightstick.dx > DEADZONE || _rightstick.dx < DEADZONE) && (_rightstick.dy > DEADZONE || _rightstick.dy < DEADZONE)) {
        p.rx = _rightstick.dx;
        p.ry = _rightstick.dy;
    }

    return p;
}
