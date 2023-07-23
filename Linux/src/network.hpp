#pragma once
#include <netinet/in.h>
#include <string>

class Network
{
public:
    Network();
    ~Network();

    bool setPort(const std::string& port);
    bool setTimeout(const std::string& value);

    bool isRunning(void) const { return this->m_running; }

    bool receive(void);

    void printPacket(void) const;

private:
    bool m_running;
    int m_sockfd;
    sockaddr_in m_server_addr;
    sockaddr_in m_client_addr;
    timeval m_timeout;

    bool configureSocket(void);
    bool init(void);
    void exit(void);
};
