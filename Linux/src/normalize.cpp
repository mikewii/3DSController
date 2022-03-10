#include "normalize.hpp"

#include <string.h>
#include <limits>

Normalize::Normalize()
{
    switch(settings.mode){
    case MODE::DEFAULT:
    case MODE::Lite_V2:{
        this->l_stick_step = std::numeric_limits<s16>().max() / L_STICK_MAX;
        this->r_stick_step = std::numeric_limits<s16>().max() / R_STICK_MAX;
        break;
    }
    case MODE::Lite_V1:{
        this->l_stick_step = std::numeric_limits<s16>().max() / this->max;
        this->r_stick_step = std::numeric_limits<s16>().max() / this->max;
        break;
    }
    default:break;
    }

    this->l_stick_prestep = L_STICK_MAX / this->max;
    this->r_stick_prestep = R_STICK_MAX / this->max;

    this->touch_x_step = TOUCH_X_MAX / this->max;
    this->touch_y_step = TOUCH_Y_MAX / this->max;
}

s16 Normalize::l_stick(s16 axis) const
{
    if (axis < -DEADZONE || axis > DEADZONE) {
        switch(settings.mode){
        case MODE::DEFAULT:
        case MODE::Lite_V2:
            if (axis < -L_STICK_MAX)
                axis = -L_STICK_MAX;
            else if (axis > L_STICK_MAX)
                axis = L_STICK_MAX;

            return std::min(static_cast<s16>(axis * this->l_stick_step), std::numeric_limits<s16>().max());
        case MODE::Lite_V1:
            return std::min(static_cast<s16>(std::min((static_cast<float>(axis) * this->l_stick_prestep), this->max) * this->l_stick_step), std::numeric_limits<s16>().max());
        }
    } return 0;
}

s16 Normalize::r_stick(s16 axis) const
{
    if (axis < -DEADZONE || axis > DEADZONE) {
        switch(settings.mode){
        case MODE::DEFAULT:
        case MODE::Lite_V2:
            if (axis < -R_STICK_MAX)
                axis = -R_STICK_MAX;
            else if (axis > R_STICK_MAX)
                axis = R_STICK_MAX;

            return std::min(static_cast<s16>(axis * this->r_stick_step), std::numeric_limits<s16>().max());
        case MODE::Lite_V1:
            return std::min(static_cast<s16>(std::min((static_cast<float>(axis) * this->r_stick_prestep), this->max) * this->r_stick_step), std::numeric_limits<s16>().max());
        }
    } return 0;
}
