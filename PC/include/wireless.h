#pragma once

#ifndef WINVER
	#define WINVER 0x0500
#endif

#include <windows.h>
#include <winsock.h>

extern SOCKET listener;
extern SOCKET client;

#include <stddef.h>
#include "keys.h"

#define SCREENSHOT_CHUNK 4000

#define IP INADDR_ANY

enum NET_COMMANDS {
    CONNECT = 1,
    DISCONNECT,
    KEYS
};

struct packet { // BBxxIhhHHhh
    struct {
        u8 command;
        u8 keyboardActive;
        u8 padding[2];
    } packetHeader;

    // KEYS
    struct {
        u32 keys;

        struct circlePosition  circlePad;
        struct touchPosition   touch;
        struct circlePosition  cStick;
    } keysPacket;
};

extern struct sockaddr_in client_in;

extern int sockaddr_in_sizePtr;

extern struct packet buffer;
extern char hostName[80];

void initNetwork(void);
void printIPs(void);
void startListening(void);
void sendBuffer(int length);
int receiveBuffer(int length);

void sendScreenshot(void);
