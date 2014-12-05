#pragma once

#include <string.h>

#include <3ds.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include "inet_pton.h"

//#define DEFAULT_IP { 192, 168, 0, 4 }
#define DEFAULT_PORT 8888

enum NET_COMMANDS {
	CONNECT,
	KEYS,
};

struct packet {
	unsigned char command;
	
	unsigned int keys;
	
	struct {
		short x;
		short y;
	} cstick;
	
	struct {
		unsigned short x;
		unsigned short y;
	} touch;
};

extern int sock;
extern struct sockaddr_in sain, saout;
extern char outBuf[sizeof(struct packet)], rcvBuf[sizeof(struct packet)];

bool openSocket(int port);
void sendBuf(int length);
void sendConnectionRequest(void);
void sendKeys(unsigned int keys, circlePosition cstick, touchPosition touch);