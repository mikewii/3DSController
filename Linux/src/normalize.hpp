#pragma once
#include "types.h"

class Normalize
{
public:
    Normalize();

    s16 stickL(s16 axis) const;
    s16 stickR(s16 axis) const;
    u16 touchX(const u16 posX) const;
    u16 touchY(const u16 posY) const;

private:
    float m_max;
    float m_stickPrestepL;
    float m_stickPrestepR;
    float m_touchStepX;
    float m_touchStepY;
    s16 m_stickStepL;
    s16 m_stickStepR;
};
