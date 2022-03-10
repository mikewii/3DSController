#include "log.hpp"

#include <stdio.h>
#include <errno.h>
#include <string.h>

void Log::print(const char *text)
{
    fprintf(stderr, "error: %s %s\n", text, strerror(errno));
}
