#include "network.hpp"
#include "log.hpp"

#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

char buffer[16];
int  buffer_size = 0;

Network::Network()
{
    this->sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (this->sockfd <= 0) {
        Log::print("socket");
        return;
    }

    if (!this->configure_socket()) return;

    memset(&this->server_addr, 0, sizeof(this->server_addr));
    memset(&this->client_addr, 0, sizeof(this->client_addr));

    this->server_addr.sin_family = AF_INET;
    this->server_addr.sin_addr.s_addr = INADDR_ANY;
    this->server_addr.sin_port = htons(settings.port);

    if (bind(this->sockfd, reinterpret_cast<__CONST_SOCKADDR_ARG>(&this->server_addr), sizeof(this->server_addr)) < 0) {
        Log::print("bind");
        return;
    }

    this->running = true;
}

Network::~Network()
{
    this->running = false;

    if (this->sockfd > 0 ) {
        if (shutdown(this->sockfd, SHUT_RDWR) < 0) {
            Log::print("shutdown");
        }

        if (close(this->sockfd) < 0) {
            Log::print("close");
        }

        this->sockfd = 0;
    }
}

const bool Network::receive(void)
{
    socklen_t   client_addr_len = sizeof(this->client_addr);
    int         received;

    received = recvfrom
            (
                sockfd,
                buffer,
                buffer_size,
                MSG_WAITALL,
                reinterpret_cast<__SOCKADDR_ARG>(&this->client_addr),
                &client_addr_len
            );

    // TODO: any need to deal with timeout errors?

    return received == buffer_size;
}

void Network::print_packet(void) const
{
    Packet_lite_v2& packet = reinterpret_cast<Packet_lite_v2&>(buffer);

    printf("keys        : %d\n", packet.keys);
    printf("left  stick : %d %d\n", packet.lx, packet.ly);
    printf("right stick : %d %d\n", packet.rx, packet.ry);
    printf("touch       : %d %d\n", packet.touch_x, packet.touch_y);
    fflush(stdout);
}

const bool Network::configure_socket(void) const
{
    int opt = 1;

    if (setsockopt(this->sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        Log::print("setsockopt (SO_REUSEADDR | SO_REUSEPORT)");
        return false;
    }

    if (setsockopt(this->sockfd, SOL_SOCKET, SO_RCVTIMEO, &this->timeout, sizeof(this->timeout)) < 0) {
        Log::print("setsockopt (SO_RCVTIMEO)");
        return false;
    }

    return true;
}
