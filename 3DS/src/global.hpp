#pragma once
#include "types.h"

#define L_STICK_MAX 155
#define R_STICK_MAX 146
#define DEADZONE 10
#define TOUCH_X_MAX 320
#define TOUCH_Y_MAX 240

template <typename T>
struct T16 {
    T   x;
    T   y;
};

struct Packet {
    u32         keys;
    T16<u16>    touch;
    T16<s16>    leftStick;
    T16<s16>    rightStick;
};

struct Packet_lite_v1 {
    u64 keys    : 14;

    u64 touch_x : 9;
    u64 touch_y : 8;
    u64 unused  : 1;

    s64 lx      : 8;
    s64 ly      : 8;
    s64 rx      : 8;
    s64 ry      : 8;
};

struct Packet_lite_v2 {
    u64 keys    : 14;

    u64 touch_x : 7;
    u64 touch_y : 7;

    s64 lx      : 9;
    s64 ly      : 9;
    s64 rx      : 9;
    s64 ry      : 9;
};

extern char buffer[16];
extern int  buffer_size;

enum MODE : int {
    DEFAULT = 0,
    Lite_V1,
    Lite_V2
};

struct Settings {
    char IP[16];
    int  port;
    int  mode;
};
extern Settings settings;
