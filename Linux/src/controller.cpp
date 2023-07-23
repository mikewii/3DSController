#include "controller.hpp"
#include "log.hpp"

#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits>

struct Axes {
    struct AxesConfig {
        int key;

        s16 min;
        s16 max;
        s16 deathzone;
    };

    AxesConfig leftX;
    AxesConfig leftY;

    AxesConfig rightX;
    AxesConfig rightY;

    AxesConfig dpadX;
    AxesConfig dpadY;
};

static const Axes controllerAxes =
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

const std::map<const char*, const std::pair<const int, const int>> Controller::m_buttonsSetting =
{
    {"A",       {N3DS_KEY_A, BTN_A}},
    {"B",       {N3DS_KEY_B, BTN_B}},
    {"X",       {N3DS_KEY_X, BTN_X}},
    {"Y",       {N3DS_KEY_Y, BTN_Y}},
    {"L",       {N3DS_KEY_L, BTN_TL}},
    {"R",       {N3DS_KEY_R, BTN_TR}},
    {"ZL",      {N3DS_KEY_ZL, BTN_TL2}},
    {"ZR",      {N3DS_KEY_ZR, BTN_TR2}},
    {"START",   {N3DS_KEY_START, BTN_START}},
    {"SELECT",  {N3DS_KEY_SELECT, BTN_SELECT}}
};

const std::map<u32, u32> Controller::m_buttonsDefault =
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

std::map<u32, u32> Controller::m_buttons =
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

u32 Controller::m_touchButton0 = BTN_THUMBL;
u32 Controller::m_touchButton1 = BTN_THUMBR;

Controller::Controller()
    : m_running(false)
    , m_touchType(TOUCH_CROSS)
{
    m_fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);

    if (m_fd < 0) {
        Log::print("Controller::open");
        return;
    }

    configure();

    if (ioctl(m_fd, UI_DEV_SETUP, &m_usetup) < 0) {
        Log::print("Controller::ioctl::UI_DEV_SETUP");
        return;
    }
    if (ioctl(m_fd, UI_DEV_CREATE) < 0) {
        Log::print("Controller::ioctl::UI_DEV_CREATE");
        return;
    }

    m_running = true;
}

Controller::~Controller()
{
    m_running = false;

    if (ioctl(m_fd, UI_DEV_DESTROY) < 0) {
        Log::print("~Controller::ioctl::UI_DEV_DESTROY");
    }

    if (m_fd > 0) {
        if (close(m_fd) < 0)
            Log::print("Controller::close");
        else
            m_fd = 0;
    }
}

bool Controller::isRunning(void) const
{
    return m_running;
}

void Controller::configure(void)
{
    ioctl(m_fd, UI_SET_EVBIT, EV_ABS);

    for (const auto& key : deviceAxes)
        ioctl(m_fd, UI_SET_ABSBIT, key);

    ioctl(m_fd, UI_SET_EVBIT, EV_KEY);

    for (const auto& key : deviceButtons)
        ioctl(m_fd, UI_SET_KEYBIT, key);

    memset(&m_usetup, 0, sizeof(m_usetup));

    m_usetup.id.bustype = BUS_PCI;
    m_usetup.id.vendor = 0x0;
    m_usetup.id.product = 0x0;
    m_usetup.id.version = 0x0;

    strcpy(m_usetup.name, "3DS Controller");
}

void Controller::process(void)
{
    processKeys();
    processTouch();
    processDpad();
    processStick();
    processStick(true);

    clear();
}

void Controller::panic(void)
{
    for (const auto& key : m_buttons)
        _emit(EV_KEY, key.second, 0);

    _emit(EV_KEY, m_touchButton0, 0);
    _emit(EV_KEY, m_touchButton1, 0);

    _emit(EV_ABS, controllerAxes.leftX.key, 0);
    _emit(EV_ABS, controllerAxes.leftY.key, 0);

    _emit(EV_ABS, controllerAxes.rightX.key, 0);
    _emit(EV_ABS, controllerAxes.rightY.key, 0);

    _emit(EV_ABS, controllerAxes.dpadX.key, 0);
    _emit(EV_ABS, controllerAxes.dpadY.key, 0);

    sync();
}

bool Controller::replaceButton(const std::string &left,
                               const std::string &right)
{
    auto id_3ds = -1;
    auto id_uinput = -1;

    if (left.size() > 2 || right.size() > 2)
        return false;

    auto validate = [&](const std::string& str) -> bool {
        for (const auto& key : Controller::m_buttonsSetting)
            if (str.find(key.first) != std::string::npos)
                return true;
        return false;
    };

    if (!validate(left)
        || !validate(right))
        return false;

    for (const auto& key : Controller::m_buttonsSetting) {
        if (id_3ds == -1 && left.find(key.first) != std::string::npos)
            id_3ds = key.second.first;

        if (id_uinput == -1 && right.find(key.first) != std::string::npos)
            id_uinput = key.second.second;
    }

    replaceButton(id_3ds, id_uinput);

    return true;
}

void Controller::replaceButton(const u32 id_3ds,
                               const u32 id_uinput)
{
    for (auto& btn : Controller::m_buttons)
        if (btn.first == id_3ds)
            btn.second = id_uinput;
}

void Controller::resetButtons(void)
{
    m_buttons = m_buttonsDefault;
}

void Controller::processKeys(void) const
{
    for (const auto& key : m_buttons) {
        const auto value = key.first & m_keys ? 1 : 0;

        _emit(EV_KEY, key.second, value);

        sync();
    }
}

void Controller::processDpad(void) const
{
    if (N3DS_KEY_DLEFT & m_keys)
        _emit(EV_ABS, ABS_HAT0X, std::numeric_limits<s16>().min());
    else if (N3DS_KEY_DRIGHT & m_keys)
        _emit(EV_ABS, ABS_HAT0X, std::numeric_limits<s16>().max());
    else
        _emit(EV_ABS, ABS_HAT0X, 0);

    if (N3DS_KEY_DUP & m_keys)
        _emit(EV_ABS, ABS_HAT0Y, std::numeric_limits<s16>().min());
    else if (N3DS_KEY_DDOWN & m_keys)
        _emit(EV_ABS, ABS_HAT0Y, std::numeric_limits<s16>().max());
    else
        _emit(EV_ABS, ABS_HAT0Y, 0);

    sync();
}

void Controller::processStick(bool right) const
{
    const auto& cstickX = right ? controllerAxes.rightX : controllerAxes.leftX;
    const auto& cstickY = right ? controllerAxes.rightY : controllerAxes.leftY;

    const auto& x = right ? m_rstick.x : m_lstick.x;
    const auto& y = right ? -m_rstick.y : -m_lstick.y;

    _emit(EV_ABS, cstickX.key, x);
    _emit(EV_ABS, cstickY.key, y);

    sync();
}

void Controller::processTouch(void) const
{
    if (m_touch.x > 0 || m_touch.y > 0) {
        switch(m_touchType) {
        default:break;
        case TOUCH_VERTICAL: return processTouchVertical();
        case TOUCH_HORIZONTAL: return processTouchHorizontal();
        case TOUCH_CROSS: return processTouchCross();
        };
    } else {
        _emit(EV_KEY, m_touchButton0, 0);
        _emit(EV_KEY, m_touchButton1, 0);
    }
}

void Controller::clear(void)
{
    m_keys = 0;
    m_touch.x = 0;
    m_touch.y = 0;
    m_lstick.x = 0;
    m_lstick.y = 0;
    m_rstick.x = 0;
    m_rstick.y = 0;
}

void Controller::sync(void) const
{
    input_event e;

    e.type = EV_SYN;
    e.code = SYN_REPORT;
    e.value = 0;
    e.time.tv_sec = 0;
    e.time.tv_usec = 0;

    write(m_fd, &e, sizeof(e));
}

void Controller::_emit(const int type,
                       const int code,
                       const int val) const
{
   input_event e;

   e.type = type;
   e.code = code;
   e.value = val;
   /* timestamp values below are ignored */
   e.time.tv_sec = 0;
   e.time.tv_usec = 0;

   write(m_fd, &e, sizeof(e));
}
