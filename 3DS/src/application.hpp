#pragma once
#include "network.hpp"

#include <3ds.h>
#include <string>

class Application : public Network
{
public:
    Application();
    ~Application();

    const bool isRunning(void) const { return this->running; }

    void prepare(void);
    void mainLoop(void);

private:
    PrintConsole    bottomScreen;
    bool            running = false;
    bool            settings_set = false;

    const char*     settings_filename = "3DSController.ini";

    void wait_for_wifi(void);
    void swap_buffers(void) const;

    const bool read_settings_value(const std::string& value, const int type);
    const bool read_settings(void);
    const bool write_settings(void);

    void    get_input(void);
    int     keyboard(bool isPort, char* buffer, const char* hint, const char* initialText);
};
