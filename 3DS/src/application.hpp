#pragma once
#include "network.hpp"

#include <3ds.h>
#include <string>

class Application : public Network
{
public:
    Application();
    ~Application();

    bool isRunning(void) const { return m_running; }

    void prepare(void);
    void mainLoop(void);

private:
    PrintConsole    m_bottomScreen;
    bool            m_running;

    const char*     settings_filename = "3DSController.ini";

    void wait_for_wifi(void);
    void swap_buffers(void) const;

    bool read_settings_value(const std::string& value, const int type);
    bool read_settings(void);
    bool write_settings(void);

    bool get_input(const bool settings_read);
    void keyboard(char* buffer, const char* hint, const char* initialText, bool isPort = false);
};
