#pragma once
#include "network.hpp"
#include "controller.hpp"
#include "normalize.hpp"

#include <string>

class Application : public Network, public Controller, Normalize
{
public:
    Application();

    void preprocess(void);

    void        write_settings(void) const;
    const bool  read_settings(void) const;

    const bool isRunning(void) { return (Application::Network::isRunning() && Application::Controller::isRunning()); }

private:
    const char* settings_filename = "3DSController.conf";
};
