#include "controller.hpp"
#include "log.hpp"

#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits>


struct axes {
    struct axes_config {
        int key;

        s16 min;
        s16 max;
        s16 deathzone;
    };

    axes_config    left_x;
    axes_config    left_y;

    axes_config    right_x;
    axes_config    right_y;

    axes_config    dpad_x;
    axes_config    dpad_y;
};

static const axes controller_axes =
{
    // left stick min: -157 max: 155
    {ABS_X, -150, 150, 10},
    {ABS_Y, -150, 150, 10},

    // right stick min: -146 max: 146
    {ABS_RX, -140, 140, 16},
    {ABS_RY, -140, 140, 16},

    // dpad
    {ABS_HAT0X, -1, 1, 0},
    {ABS_HAT0Y, -1, 1, 0}
};

std::map<u32, u32> Controller::buttons =
{
    {N3DS_KEY_A,        BTN_A},
    {N3DS_KEY_B,        BTN_B},
    {N3DS_KEY_X,        BTN_X},
    {N3DS_KEY_Y,        BTN_Y},
    {N3DS_KEY_L,        BTN_TL},
    {N3DS_KEY_R,        BTN_TR},
    {N3DS_KEY_ZL,       BTN_TL2},
    {N3DS_KEY_ZR,       BTN_TR2},
    {N3DS_KEY_SELECT,   BTN_SELECT},
    {N3DS_KEY_START,    BTN_START}
};

u32 Controller::touch_button0 = BTN_THUMBL;
u32 Controller::touch_button1 = BTN_THUMBR;

Controller::Controller()
{
    this->fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (this->fd < 0) {
        Log::print("Controller::open");
        return;
    }

    this->configure();

    if (ioctl(fd, UI_DEV_SETUP, &usetup) < 0) {
        Log::print("Controller::ioctl::UI_DEV_SETUP");
        return;
    }
    if (ioctl(fd, UI_DEV_CREATE) < 0) {
        Log::print("Controller::ioctl::UI_DEV_CREATE");
        return;
    }

    this->running = true;
}

Controller::~Controller()
{
    this->running = false;

    if (ioctl(fd, UI_DEV_DESTROY) < 0) {
        Log::print("~Controller::ioctl::UI_DEV_DESTROY");
    }

    if (this->fd > 0) {
        if (close(this->fd) < 0)
            Log::print("Controller::close");
        else
            this->fd = 0;
    }
}

void Controller::configure(void)
{
    ioctl(this->fd, UI_SET_EVBIT, EV_ABS);
    for (const auto& key : device_axes)
        ioctl(this->fd, UI_SET_ABSBIT, key);

    ioctl(this->fd, UI_SET_EVBIT, EV_KEY);
    for (const auto& key : device_buttons)
        ioctl(this->fd, UI_SET_KEYBIT, key);

    memset(&this->usetup, 0, sizeof(this->usetup));
    this->usetup.id.bustype = BUS_USB;
    this->usetup.id.vendor = 0x0;
    this->usetup.id.product = 0x0;
    this->usetup.id.version = 0x0;
    strcpy(this->usetup.name, "virtual controller");
}

void Controller::process(void)
{
    this->process_keys();
    this->process_touch();
    this->process_dpad();
    this->process_stick();
    this->process_stick(true);

    this->clear();
}

void Controller::panic(void)
{
    for (const auto& key : this->buttons)
        this->_emit(EV_KEY, key.second, 0);

    this->_emit(EV_KEY, this->touch_button0, 0);
    this->_emit(EV_KEY, this->touch_button1, 0);

    this->_emit(EV_ABS, controller_axes.left_x.key, 0);
    this->_emit(EV_ABS, controller_axes.left_y.key, 0);

    this->_emit(EV_ABS, controller_axes.right_x.key, 0);
    this->_emit(EV_ABS, controller_axes.right_y.key, 0);

    this->_emit(EV_ABS, controller_axes.dpad_x.key, 0);
    this->_emit(EV_ABS, controller_axes.dpad_y.key, 0);

    this->sync();
}

void Controller::process_keys(void) const
{
    for (const auto& key : this->buttons) {
        const int value = key.first & this->keys ? 1 : 0;

        this->_emit(EV_KEY, key.second, value);
        this->sync();
    }
}

void Controller::process_dpad(void) const
{
    if (N3DS_KEY_DLEFT & this->keys)
        this->_emit(EV_ABS, ABS_HAT0X, -1);
    else if (N3DS_KEY_DRIGHT & this->keys)
        this->_emit(EV_ABS, ABS_HAT0X, 1);
    else
        this->_emit(EV_ABS, ABS_HAT0X, 0);

    if (N3DS_KEY_DUP & this->keys)
        this->_emit(EV_ABS, ABS_HAT0Y, -1);
    else if (N3DS_KEY_DDOWN & this->keys)
        this->_emit(EV_ABS, ABS_HAT0Y, 1);
    else
        this->_emit(EV_ABS, ABS_HAT0Y, 0);

    this->sync();
}

void Controller::process_stick(bool right) const
{
    const auto& cstick_x = right ? controller_axes.right_x : controller_axes.left_x;
    const auto& cstick_y = right ? controller_axes.right_y : controller_axes.left_y;

    const auto& x = right ? this->rstick.x : this->lstick.x;
    const auto& y = right ? -this->rstick.y : -this->lstick.y;

    // X
    this->_emit(EV_ABS, cstick_x.key, x);

    // Y
    this->_emit(EV_ABS, cstick_y.key, y);

    this->sync();
}

void Controller::process_touch(void) const
{
    if (this->touch.x > 0 || this->touch.y > 0) {
        switch(this->touch_type){
        case TOUCH_VERTICAL: this->process_touch_vertical(); break;
        case TOUCH_HORIZONTAL: this->process_touch_horizontal(); break;
        case TOUCH_CROSS: this->process_touch_cross(); break;
        default:break;
        }
    } else {
        this->_emit(EV_KEY, this->touch_button0, 0);
        this->_emit(EV_KEY, this->touch_button1, 0);
    }
}

void Controller::clear(void)
{
    this->keys = 0;
    this->touch.x = 0;
    this->touch.y = 0;
    this->lstick.x = 0;
    this->lstick.y = 0;
    this->rstick.x = 0;
    this->rstick.y = 0;
}

void Controller::sync(void) const
{
    input_event ie;

    ie.type = EV_SYN;
    ie.code = SYN_REPORT;
    ie.value = 0;
    ie.time.tv_sec = 0;
    ie.time.tv_usec = 0;

    write(this->fd, &ie, sizeof(ie));
}

void Controller::_emit(const int type, const int code, const int val) const
{
   struct input_event ie;

   ie.type = type;
   ie.code = code;
   ie.value = val;
   /* timestamp values below are ignored */
   ie.time.tv_sec = 0;
   ie.time.tv_usec = 0;

   write(this->fd, &ie, sizeof(ie));
}
