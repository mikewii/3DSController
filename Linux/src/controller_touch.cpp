#include "controller.hpp"

#define TOUCH_MAX_X 320
#define TOUCH_MAX_Y 240

void Controller::processTouchVertical(void) const
{
    if (m_touch.x < TOUCH_MAX_X / 2)
        _emit(EV_KEY, m_touchButton0, 1);
    else _emit(EV_KEY, m_touchButton0, 0);

    if (m_touch.x > TOUCH_MAX_X / 2)
        _emit(EV_KEY, m_touchButton1, 1);
    else _emit(EV_KEY, m_touchButton1, 0);

    sync();
}

void Controller::processTouchHorizontal(void) const
{
    if (m_touch.y < TOUCH_MAX_Y / 2)
        _emit(EV_KEY, m_touchButton0, 1);
    else _emit(EV_KEY, m_touchButton0, 0);

    if (m_touch.y > TOUCH_MAX_Y / 2)
        _emit(EV_KEY, m_touchButton1, 1);
    else _emit(EV_KEY, m_touchButton1, 0);

    sync();
}

void Controller::processTouchCross(void) const
{
    if (m_touch.x < TOUCH_MAX_X / 2 && m_touch.y < TOUCH_MAX_Y / 2)
        _emit(EV_KEY, m_touchButton0, 1);
    else if (m_touch.x > TOUCH_MAX_X / 2 && m_touch.y > TOUCH_MAX_Y / 2)
        _emit(EV_KEY, m_touchButton0, 1);
    else _emit(EV_KEY, m_touchButton0, 0);

    if (m_touch.x > TOUCH_MAX_X / 2 && m_touch.y < TOUCH_MAX_Y / 2)
        _emit(EV_KEY, m_touchButton1, 1);
    else if (m_touch.x < TOUCH_MAX_X / 2 && m_touch.y > TOUCH_MAX_Y / 2)
        _emit(EV_KEY, m_touchButton1, 1);
    else _emit(EV_KEY, m_touchButton1, 0);

    sync();
}
