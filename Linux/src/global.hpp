#pragma once
#include "types.h"

template <typename T>
struct T16 {
    T   x;
    T   y;
};

struct Packet {
    u32         keys;
    T16<s16>    leftStick;
    T16<s16>    rightStick;
    T16<u16>    touch;

    char*       data(void) { return reinterpret_cast<char*>(this); }
    const int   size(void) const { return sizeof(Packet); }
};

extern Packet packet;
