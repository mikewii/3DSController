#pragma once
#include "global.hpp"

#include <linux/uinput.h>
#include <queue>
#include <map>
#include <string>

#define BIT(n) (1U<<(n))

enum : u32 {
    N3DS_KEY_A       = BIT(0),       ///< A
    N3DS_KEY_B       = BIT(1),       ///< B
    N3DS_KEY_SELECT  = BIT(2),       ///< Select
    N3DS_KEY_START   = BIT(3),       ///< Start
    N3DS_KEY_DRIGHT  = BIT(4),       ///< D-Pad Right
    N3DS_KEY_DLEFT   = BIT(5),       ///< D-Pad Left
    N3DS_KEY_DUP     = BIT(6),       ///< D-Pad Up
    N3DS_KEY_DDOWN   = BIT(7),       ///< D-Pad Down
    N3DS_KEY_R       = BIT(8),       ///< R
    N3DS_KEY_L       = BIT(9),       ///< L
    N3DS_KEY_X       = BIT(10),      ///< X
    N3DS_KEY_Y       = BIT(11),      ///< Y
    N3DS_KEY_ZL      = BIT(14),      ///< ZL (New 3DS only)
    N3DS_KEY_ZR      = BIT(15),      ///< ZR (New 3DS only)
    N3DS_KEY_TOUCH   = BIT(20),      ///< Touch (Not actually provided by HID)
    N3DS_KEY_CSTICK_RIGHT = BIT(24), ///< C-Stick Right (New 3DS only)
    N3DS_KEY_CSTICK_LEFT  = BIT(25), ///< C-Stick Left (New 3DS only)
    N3DS_KEY_CSTICK_UP    = BIT(26), ///< C-Stick Up (New 3DS only)
    N3DS_KEY_CSTICK_DOWN  = BIT(27), ///< C-Stick Down (New 3DS only)
    N3DS_KEY_CPAD_RIGHT = BIT(28),   ///< Circle Pad Right
    N3DS_KEY_CPAD_LEFT  = BIT(29),   ///< Circle Pad Left
    N3DS_KEY_CPAD_UP    = BIT(30),   ///< Circle Pad Up
    N3DS_KEY_CPAD_DOWN  = BIT(31),   ///< Circle Pad Down

    // Generic catch-all directions
    N3DS_KEY_UP    = N3DS_KEY_DUP    | N3DS_KEY_CPAD_UP,    ///< D-Pad Up or Circle Pad Up
    N3DS_KEY_DOWN  = N3DS_KEY_DDOWN  | N3DS_KEY_CPAD_DOWN,  ///< D-Pad Down or Circle Pad Down
    N3DS_KEY_LEFT  = N3DS_KEY_DLEFT  | N3DS_KEY_CPAD_LEFT,  ///< D-Pad Left or Circle Pad Left
    N3DS_KEY_RIGHT = N3DS_KEY_DRIGHT | N3DS_KEY_CPAD_RIGHT, ///< D-Pad Right or Circle Pad Right
};

enum lite : u16 {
    N3DS_LITE_KEY_A       = BIT(0),
    N3DS_LITE_KEY_B       = BIT(1),
    N3DS_LITE_KEY_SELECT  = BIT(2),
    N3DS_LITE_KEY_START   = BIT(3),
    N3DS_LITE_KEY_DRIGHT  = BIT(4),
    N3DS_LITE_KEY_DLEFT   = BIT(5),
    N3DS_LITE_KEY_DUP     = BIT(6),
    N3DS_LITE_KEY_DDOWN   = BIT(7),
    N3DS_LITE_KEY_R       = BIT(8),
    N3DS_LITE_KEY_L       = BIT(9),
    N3DS_LITE_KEY_X       = BIT(10),
    N3DS_LITE_KEY_Y       = BIT(11),
    N3DS_LITE_KEY_ZL      = BIT(12),
    N3DS_LITE_KEY_ZR      = BIT(13)
};



static const u32 device_axes[] =
{
    ABS_X, ABS_Y, ABS_RX, ABS_RY, ABS_HAT0X, ABS_HAT0Y
};

static const u32 device_buttons[] =
{
    BTN_A, BTN_B, BTN_X, BTN_Y, BTN_TL, BTN_TR, BTN_TL2, BTN_TR2,
    BTN_SELECT, BTN_START, BTN_MODE, BTN_THUMBL, BTN_THUMBR
};

class Controller
{
public:
    enum {
        TOUCH_VERTICAL,
        TOUCH_HORIZONTAL,
        TOUCH_CROSS,

        TOUCH_INVERTED
    };

    Controller();
    ~Controller();

    const bool isRunning(void) const { return this->running; }

    void process(void);
    void panic(void);

    bool replace_button(const std::string& left, const std::string& right) const;
    void replace_button(int id_3ds, int id_uinput) const;
    void reset_buttons(void);

    u32         keys;
    T16<u16>    touch;
    T16<s16>    lstick, rstick;
    static const std::map<const char*, const std::pair<const int, const int>> buttons_setting;
    static std::map<u32, u32> buttons;
    static const std::map<u32, u32> buttons_default;
    static u32 touch_button0, touch_button1;

private:
    bool            running = false;
    int             touch_type = TOUCH_CROSS;

    uinput_setup    usetup;
    int             fd;

    void process_keys(void) const;
    void process_dpad(void) const;
    void process_stick(bool right = false) const;
    void process_touch(void) const;
    void clear(void);

    void process_touch_vertical(void) const;
    void process_touch_horizontal(void) const;
    void process_touch_cross(void) const;

    void configure(void);
    void run_queue(void);
    void sync(void) const;
    void _emit(const int type, const int code, const int val) const;
};


