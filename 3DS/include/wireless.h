#pragma once

#include <string.h>

#include <3ds.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include "inet_pton.h"

#define SCREENSHOT_CHUNK 4000

#define DEFAULT_PORT 8889

enum NET_COMMANDS {
    CONNECT = 0,
    DISCONNECT,
    KEYS
};

// It is deliberately set up to have an anonymous struct as well as a named struct for convenience, not a mistake!
struct packet { // BBxxIhhHHhh
    struct {
        u8 command;
        u8 keyboardActive;
        u8 padding[2];
    } packetHeader;

    // KEYS
    struct {
        u32 keys;

        circlePosition  circlePad;
        touchPosition   touch;
        circlePosition  cStick;
    } keysPacket;
};

extern int sock;
extern struct sockaddr_in sain, saout;
extern struct packet outBuf, rcvBuf;

extern socklen_t sockaddr_in_sizePtr;

bool    openSocket(int port);
void    sendBuf(int length);
int     receiveBuffer(int length);
void    sendConnectionRequest(void);
void    sendDisconnectRequest(void);
void    sendKeys(unsigned int keys, circlePosition circlePad, touchPosition touch, circlePosition cStick);
