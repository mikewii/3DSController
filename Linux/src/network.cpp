#include "network.hpp"
#include "log.hpp"
#include "global.hpp"

#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

char buffer[16];
int  buffer_size = 0;

Network::Network()
    : m_running(false)
    , m_sockfd(0)
    , m_timeout{.tv_sec = 0, .tv_usec = 50000} // 50ms
{
    m_running = init();
}

Network::~Network()
{
    m_running = false;

    exit();
}

bool Network::setPort(const std::string &port)
{
    if (port.size() <= 5
        && port.find_first_not_of("0123456789") == std::string::npos) {
        u16 port_value = std::stoi(port);

        settings.port = port_value;

        if (m_server_addr.sin_port != htons(settings.port)) {
            exit();

            m_server_addr.sin_port = htons(settings.port);

            init();
        }
        return true;
    } else return false;
}

bool Network::setTimeout(const std::string& value)
{
    if (value.size() <= 5
        && value.find_first_not_of("0123456789") == std::string::npos) {
        u16 timeout_value = std::stoi(value);

        settings.network_timeout_ms = timeout_value;

        // idk if we can update timeout value on fly, reinit just to be sure
        if (m_timeout.tv_usec != settings.network_timeout_ms * 1000) {
            exit();

            m_timeout.tv_usec = settings.network_timeout_ms * 1000;

            init();
        }

        return true;
    } else return false;
}

bool Network::receive(void)
{
    socklen_t client_addr_len = sizeof(m_client_addr);
    int received;

    received = recvfrom
    (
        m_sockfd,
        buffer,
        buffer_size,
        MSG_WAITALL,
        reinterpret_cast<__SOCKADDR_ARG>(&m_client_addr),
        &client_addr_len
    );

    // TODO: any need to deal with timeout errors?

    return received == buffer_size;
}

void Network::printPacket(void) const
{
    Packet_lite_v2& packet = reinterpret_cast<Packet_lite_v2&>(buffer);

    fprintf(stderr,
        "keys        : %d\n"
        "left  stick : %d %d\n"
        "right stick : %d %d\n"
        "touch       : %d %d\n"
        , packet.keys
        , packet.lx, packet.ly
        , packet.rx, packet.ry
        , packet.touch_x, packet.touch_y);
}

bool Network::configureSocket(void)
{
    int opt = 1;

    if (setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        Log::print("setsockopt (SO_REUSEADDR | SO_REUSEPORT)");
        return false;
    }

    if (setsockopt(m_sockfd, SOL_SOCKET, SO_RCVTIMEO, &m_timeout, sizeof(m_timeout)) < 0) {
        Log::print("setsockopt (SO_RCVTIMEO)");
        return false;
    }

    return true;
}

bool Network::init(void)
{
    m_sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (m_sockfd <= 0) {
        Log::print("socket");
        return false;
    }

    if (!configureSocket())
        return false;

    memset(&m_server_addr, 0, sizeof(m_server_addr));
    memset(&m_client_addr, 0, sizeof(m_client_addr));

    m_server_addr.sin_family = AF_INET;
    m_server_addr.sin_addr.s_addr = INADDR_ANY;
    m_server_addr.sin_port = htons(settings.port);

    if (bind(m_sockfd, reinterpret_cast<__CONST_SOCKADDR_ARG>(&m_server_addr), sizeof(m_server_addr)) < 0) {
        Log::print("bind");
        return false;
    }

    return true;
}

void Network::exit(void)
{
    if (m_sockfd > 0 ) {
        if (close(m_sockfd) < 0) {
            Log::print("close");
        }

        m_sockfd = 0;
    }
}
