#pragma once
#include "network.hpp"
#include "controller.hpp"
#include "normalize.hpp"

class Application : public Network, public Controller, Normalize
{
public:
    Application();

    void preprocess(void);

    const bool isRunning(void) { return (Application::Network::isRunning() && Application::Controller::isRunning()); }
};
