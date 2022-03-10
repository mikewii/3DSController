#include "controller.hpp"

#define TOUCH_MAX_X 320
#define TOUCH_MAX_Y 240

void Controller::process_touch_vertical(void) const
{
    if (packet.touch.x < TOUCH_MAX_X / 2)
        this->_emit(EV_KEY, this->touch_button0, 1);
    else this->_emit(EV_KEY, this->touch_button0, 0);

    if (packet.touch.x > TOUCH_MAX_X / 2)
        this->_emit(EV_KEY, this->touch_button1, 1);
    else this->_emit(EV_KEY, this->touch_button1, 0);

    this->sync();
}

void Controller::process_touch_horizontal(void) const
{
    if (packet.touch.y < TOUCH_MAX_Y / 2)
        this->_emit(EV_KEY, this->touch_button0, 1);
    else this->_emit(EV_KEY, this->touch_button0, 0);

    if (packet.touch.y > TOUCH_MAX_Y / 2)
        this->_emit(EV_KEY, this->touch_button1, 1);
    else this->_emit(EV_KEY, this->touch_button1, 0);

    this->sync();
}

void Controller::process_touch_cross(void) const
{
    if (packet.touch.x < TOUCH_MAX_X / 2 && packet.touch.y < TOUCH_MAX_Y / 2)
        this->_emit(EV_KEY, this->touch_button0, 1);
    else if (packet.touch.x > TOUCH_MAX_X / 2 && packet.touch.y > TOUCH_MAX_Y / 2)
        this->_emit(EV_KEY, this->touch_button0, 1);
    else this->_emit(EV_KEY, this->touch_button0, 0);

    if (packet.touch.x > TOUCH_MAX_X / 2 && packet.touch.y < TOUCH_MAX_Y / 2)
        this->_emit(EV_KEY, this->touch_button1, 1);
    else if (packet.touch.x < TOUCH_MAX_X / 2 && packet.touch.y > TOUCH_MAX_Y / 2)
        this->_emit(EV_KEY, this->touch_button1, 1);
    else this->_emit(EV_KEY, this->touch_button1, 0);

    this->sync();
}
