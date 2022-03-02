#include "keyboard.h"
#include "wireless.h"


int sock;
struct sockaddr_in sain, saout;
struct packet outBuf, rcvBuf;

socklen_t sockaddr_in_sizePtr = (int)sizeof(struct sockaddr_in);

bool openSocket(int port)
{
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	
	saout.sin_family = sain.sin_family = AF_INET;
	saout.sin_port = sain.sin_port = htons(port);
	sain.sin_addr.s_addr = INADDR_ANY;
	
	bind(sock, (struct sockaddr *)&sain, sizeof(sain));
	
	fcntl(sock, F_SETFL, O_NONBLOCK);
	
	return true;
}

void sendBuf(int length)
{
	sendto(sock, (char *)&outBuf, length, 0, (struct sockaddr *)&saout, sizeof(saout));
}

int receiveBuffer(int length)
{
	return recvfrom(sock, (char *)&rcvBuf, length, 0, (struct sockaddr *)&sain, &sockaddr_in_sizePtr);
}

void sendKeys(const u32 _keys, const circlePosition _leftStick, const circlePosition _rightStick, const touchPosition _touch)
{
    outBuf.keys = _keys;
    outBuf.leftStick = _leftStick;
    outBuf.rightStick = _rightStick;
    outBuf.touch = _touch;

    sendBuf(sizeof(struct packet));
}
