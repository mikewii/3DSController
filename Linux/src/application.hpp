#pragma once
#include "network.hpp"
#include "controller.hpp"
#include "normalize.hpp"

#include <string>

class Application : public Network, public Controller, Normalize
{
public:
    Application();

    void mainLoop(void);
    void preProcess(void);

    void writeSettings(void) const;
    bool readSettings(void);

    bool isRunning(void);

private:
    const char* settings_filename = "3DSController.conf";

    bool setMode(const std::string& mode);
    void setBufferSize(void) const;
    void checkSettingsFile(void);
};
