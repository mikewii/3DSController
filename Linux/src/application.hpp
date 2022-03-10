#pragma once
#include "network.hpp"
#include "controller.hpp"

class Application : public Network, public Controller
{
public:
    const bool isRunning(void) { return (Application::Network::isRunning() && Application::Controller::isRunning()); }
};
