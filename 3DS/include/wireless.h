#pragma once

#include <string.h>

#include <3ds.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define SCREENSHOT_CHUNK 4000

#define DEFAULT_PORT 8889

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
void    sendDisconnect(void);
void    sendKeys(unsigned int keys, circlePosition circlePad, touchPosition touch, circlePosition cStick);
