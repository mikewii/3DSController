#pragma once
#include "types.h"
#include <stdbool.h>

struct settings {
	char IPString[16];
	int port;
};

extern struct settings settings;
extern struct settings defaultSettings;

extern Handle fileHandle;

bool readSettings(void);
void writeSettings(void);
