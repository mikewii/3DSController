#include "normalize.hpp"
#include "global.hpp"

#include <algorithm>
#include <string>
#include <limits>

Normalize::Normalize()
    : m_max{127}
    , m_stickPrestepL{0}
    , m_stickPrestepR{0}
    , m_touchStepX{0}
    , m_touchStepY{0}
{
    switch(settings.mode) {
    default:break;
    case MODE::DEFAULT:
    case MODE::Lite_V2:
        m_stickStepL = std::numeric_limits<s16>().max() / L_STICK_MAX;
        m_stickStepR = std::numeric_limits<s16>().max() / R_STICK_MAX;
        break;
    case MODE::Lite_V1:
        m_stickStepL = std::numeric_limits<s16>().max() / m_max;
        m_stickStepR = std::numeric_limits<s16>().max() / m_max;
        break;
    }

    m_stickPrestepL = L_STICK_MAX / m_max;
    m_stickPrestepR = R_STICK_MAX / m_max;

    m_touchStepX = TOUCH_X_MAX / m_max;
    m_touchStepY = TOUCH_Y_MAX / m_max;
}

s16 Normalize::stickL(s16 axis) const
{
    if (axis < -DEADZONE || axis > DEADZONE) {
        switch(settings.mode){
        default:break;
        case MODE::DEFAULT:
        case MODE::Lite_V2:
            if (axis < -L_STICK_MAX)
                axis = -L_STICK_MAX;
            else if (axis > L_STICK_MAX)
                axis = L_STICK_MAX;

            return std::min(static_cast<s16>(axis * m_stickStepL), std::numeric_limits<s16>().max());
        case MODE::Lite_V1:
            return std::min(static_cast<s16>(std::min((static_cast<float>(axis) * m_stickPrestepL), m_max) * m_stickStepL), std::numeric_limits<s16>().max());
        }
    }

    return 0;
}

s16 Normalize::stickR(s16 axis) const
{
    if (axis < -DEADZONE || axis > DEADZONE) {
        switch(settings.mode){
        default:break;
        case MODE::DEFAULT:
        case MODE::Lite_V2:
            if (axis < -R_STICK_MAX)
                axis = -R_STICK_MAX;
            else if (axis > R_STICK_MAX)
                axis = R_STICK_MAX;

            return std::min(static_cast<s16>(axis * m_stickStepR), std::numeric_limits<s16>().max());
        case MODE::Lite_V1:
            return std::min(static_cast<s16>(std::min((static_cast<float>(axis) * m_stickPrestepR), m_max) * m_stickStepR), std::numeric_limits<s16>().max());
        }
    }

    return 0;
}

u16 Normalize::touchX(const u16 posX) const
{
    return std::min((static_cast<float>(posX) * m_touchStepX)
                    , static_cast<float>(TOUCH_X_MAX));
}

u16 Normalize::touchY(const u16 posY) const
{
    return std::min((static_cast<float>(posY) * m_touchStepY)
                    , static_cast<float>(TOUCH_Y_MAX));
}
