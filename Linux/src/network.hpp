#pragma once
#include "global.hpp"

#include <netinet/in.h>
#include <string>

class Network
{
public:
    Network();
    ~Network();

    const bool  setPort(const std::string& port);
    const bool  setTimeout(const std::string& value);

    const bool  isRunning(void) const { return this->running; }

    const bool  receive(void);

    void        print_packet(void) const;

private:
    bool        running = false;
    int         sockfd = 0;
    sockaddr_in server_addr, client_addr;

    timeval     timeout = {.tv_sec = 0, .tv_usec = 50000}; // 50ms

    const bool  configure_socket(void) const;

    bool        init(void);
    void        exit(void);
};
