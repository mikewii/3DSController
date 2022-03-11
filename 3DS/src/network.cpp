#include "network.hpp"

#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <3ds.h>
#include <malloc.h>
#include <memory>
#include <string.h>

#define SOC_ALIGN       0x1000
#define SOC_BUFFERSIZE  0x100000


char    buffer[16];
int     buffer_size = 0;

Network::Network()
{
    if (R_FAILED(socInit(static_cast<u32*>(memalign(SOC_ALIGN, SOC_BUFFERSIZE)), SOC_BUFFERSIZE)))
        return;

    this->sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (this->sockfd <= 0)
        return;

    this->client_addr.sin_family = AF_INET;
    this->client_addr.sin_port   = htons(settings.port);
    this->client_addr.sin_addr.s_addr = inet_pton(AF_INET, settings.IP, &this->client_addr.sin_addr);

    if (R_FAILED(fcntl(this->sockfd, F_SETFL, O_NONBLOCK)))
        return;

    this->running = true;
}

Network::~Network()
{
    this->running = false;

    if (this->sockfd > 0 ) {
        if (R_FAILED(closesocket(this->sockfd))) {
            //Log::print("close");
        }
        else this->sockfd = 0;
    }

    socExit();
}

void Network::setPort(void)
{
    this->client_addr.sin_port = htons(settings.port);
}

void Network::setIP(void)
{
    inet_pton(AF_INET, settings.IP, &this->client_addr.sin_addr);
}

void Network::sendKeys(const u32 _keys, const touchPosition _touch, const circlePosition _leftstick, const circlePosition _rightstick)
{
    switch(settings.mode){
    case MODE::DEFAULT: {
        Packet& p = reinterpret_cast<Packet&>(buffer);

        p.keys = _keys;
        p.touch.x = _touch.px;
        p.touch.y = _touch.py;
        p.leftStick.x = _leftstick.dx;
        p.leftStick.y = _leftstick.dy;
        p.rightStick.x = _rightstick.dx;
        p.rightStick.y = _rightstick.dy;

        buffer_size = sizeof(Packet);
        break;
    }
    case MODE::Lite_V1: {
        Packet_lite_v1& p = reinterpret_cast<Packet_lite_v1&>(buffer);

        p = Normalize::v1(_keys, _touch, _leftstick, _rightstick);

        buffer_size = sizeof(Packet_lite_v1);
        break;
    }
    case MODE::Lite_V2: {
        Packet_lite_v2& p = reinterpret_cast<Packet_lite_v2&>(buffer);

        p = Normalize::v2(_keys, _touch, _leftstick, _rightstick);

        buffer_size = sizeof(Packet_lite_v2);
        break;
    }
    default:break;
    }

    sendto(this->sockfd, buffer, buffer_size, 0, reinterpret_cast<const struct sockaddr *>(&this->client_addr), sizeof(this->client_addr));
}



