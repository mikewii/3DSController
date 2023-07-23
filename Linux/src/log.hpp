#pragma once
#include <stdio.h>
#include <errno.h>
#include <string.h>

class Log
{
public:
    static inline void print(const char* text, bool err = true)
    {
        if (err)
            fprintf(stderr, "error: %s %s\n", text, strerror(errno));
        else
            fprintf(stderr, "%s\n", text);
    }
};
