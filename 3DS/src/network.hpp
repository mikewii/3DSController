#pragma once
#include "global.hpp"
#include "normalize.hpp"

#include <3ds.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <algorithm>


class Network : public Normalize
{
public:
    Network();
    ~Network();

    bool  isRunning(void) const { return m_running; }

    void    setPort(void);
    void    setIP(void);
    void    sendKeys(const u32 _keys, const touchPosition _touch, const circlePosition _leftstick, const circlePosition _rightstick);

    sockaddr_in& getClient(void) { return this->m_client_addr; }

private:
    bool        m_running;
    int         m_sockfd;
    sockaddr_in m_client_addr;
};
