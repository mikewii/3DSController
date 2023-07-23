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
    : m_running(false)
{
    if (R_FAILED(socInit(static_cast<u32*>(memalign(SOC_ALIGN, SOC_BUFFERSIZE)), SOC_BUFFERSIZE)))
        return;

    m_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (m_sockfd <= 0)
        return;

    m_client_addr.sin_family = AF_INET;
    m_client_addr.sin_port   = htons(settings.port);
    m_client_addr.sin_addr.s_addr = inet_pton(AF_INET, settings.IP, &m_client_addr.sin_addr);

    if (R_FAILED(fcntl(m_sockfd, F_SETFL, O_NONBLOCK)))
        return;

    m_running = true;
}

Network::~Network()
{
    m_running = false;

    if (m_sockfd > 0 ) {
        if (R_FAILED(closesocket(m_sockfd))) {
            //Log::print("close");
        }
        else m_sockfd = 0;
    }

    socExit();
}

void Network::setPort(void)
{
    m_client_addr.sin_port = htons(settings.port);
}

void Network::setIP(void)
{
    inet_pton(AF_INET, settings.IP, &m_client_addr.sin_addr);
}

void Network::sendKeys(const u32 keys,
                       const touchPosition touchPad,
                       const circlePosition leftStick,
                       const circlePosition rightStick)
{
    switch(settings.mode) {
    default:break;
    case MODE::DEFAULT:
        reinterpret_cast<Packet&>(buffer).keys = keys;
        reinterpret_cast<Packet&>(buffer).touchPad.x = touchPad.px;
        reinterpret_cast<Packet&>(buffer).touchPad.y = touchPad.py;
        reinterpret_cast<Packet&>(buffer).leftStick.x = leftStick.dx;
        reinterpret_cast<Packet&>(buffer).leftStick.y = leftStick.dy;
        reinterpret_cast<Packet&>(buffer).rightStick.x = rightStick.dx;
        reinterpret_cast<Packet&>(buffer).rightStick.y = rightStick.dy;

        buffer_size = sizeof(Packet);
        break;
    case MODE::Lite_V1:
        reinterpret_cast<Packet_lite_v1&>(buffer) = Normalize::v1(keys, touchPad, leftStick, rightStick);

        buffer_size = sizeof(Packet_lite_v1);
        break;
    case MODE::Lite_V2:
        reinterpret_cast<Packet_lite_v2&>(buffer) = Normalize::v2(keys, touchPad, leftStick, rightStick);

        buffer_size = sizeof(Packet_lite_v2);
        break;
    }

    sendto(m_sockfd, buffer, buffer_size, 0, reinterpret_cast<const struct sockaddr *>(&m_client_addr), sizeof(m_client_addr));
}
