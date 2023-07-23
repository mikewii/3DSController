#include "application.hpp"
#include <filesystem>
#include <fstream>
#include <string.h>
#include <csignal>
#include <algorithm>

static const char* message =
    "# Settings read in [key value] pairs\n" \
    "# For controller [left right] left is 3ds button, right is emulated button\n" \
    "# mode:\n" \
    "#   0  (default) 16byte packet with no degradation\n" \
    "#   1  (lite v1) 8byte packet with both sticks having less precision\n" \
    "#   2  (lite v2) 8byte packet with touch having less precision\n" \
    "#\n" \
    "# port:\n" \
    "#   0 - 65535\n" \
    "#\n" \
    "# timeout: time in milliseconds when controller release all buttons if packet is not received\n" \
    "#   0 - 65535\n" \
    "#\n\n" \
    "# example settings:\n" \
    "# mode 2\n" \
    "# port 8889\n" \
    "# timeout 50\n" \
    "# x y\n";


std::filesystem::path           settingsFilePath;
std::filesystem::file_time_type last_write_time;
std::filesystem::file_time_type new_write_time;
std::chrono::duration<double>   time_passed;
auto                            time_start = std::chrono::system_clock::now();
auto                            time_now = std::chrono::system_clock::now();


Settings settings =
{
    .IP = {'1', '9', '2', '.', '1', '6', '8', '.', '0', '.', '1'},
    .port = 8889,
    .network_timeout_ms = 50,
    .mode = MODE::DEFAULT,
};

Application::Application()
{
    setBufferSize();

    settingsFilePath.append((std::getenv("HOME")));
    settingsFilePath.append(settings_filename);
}

void Application::mainLoop(void)
{
    static bool exitRequested = false;
    auto signalHandler = [](int) { exitRequested = true; };

    std::signal(SIGINT, signalHandler);

    last_write_time = std::filesystem::last_write_time(settingsFilePath);

    while(Application::isRunning()) {
        //app.print_packet();

        if (receive()) {
            preProcess();
            process();
        } else {
            panic();
        }

        checkSettingsFile();

        if (exitRequested)
            break;
    }
}

void Application::preProcess(void)
{
    switch(settings.mode) {
    case MODE::DEFAULT: {
        Packet& p = reinterpret_cast<Packet&>(buffer);

        Controller::m_keys = p.keys;
        Controller::m_touch.x = p.touch.x;
        Controller::m_touch.y = p.touch.y;
        Controller::m_lstick.x = Normalize::stickL(p.leftStick.x);
        Controller::m_lstick.y = Normalize::stickL(p.leftStick.y);
        Controller::m_rstick.x = Normalize::stickR(p.rightStick.x);
        Controller::m_rstick.y = Normalize::stickR(p.rightStick.y);

        break;
    }
    case MODE::Lite_V1: {
        Packet_lite_v1& p = reinterpret_cast<Packet_lite_v1&>(buffer);

        for (auto i = 0; i < 12; i++)
            if (p.keys & 1 << i)
                Controller::m_keys |= 1 << i;

        if (p.keys & 1 << 12)
            Controller::m_keys |= N3DS_LITE_KEY_ZL;
        if (p.keys & 1 << 13)
            Controller::m_keys |= N3DS_LITE_KEY_ZR;


        Controller::m_touch.x = p.touch_x;
        Controller::m_touch.y = p.touch_y;

        Controller::m_lstick.x = Normalize::stickL(p.lx);
        Controller::m_lstick.y = Normalize::stickL(p.ly);
        Controller::m_rstick.x = Normalize::stickR(p.rx);
        Controller::m_rstick.y = Normalize::stickR(p.ry);

        break;
    }
    case MODE::Lite_V2: {
        Packet_lite_v2& p = reinterpret_cast<Packet_lite_v2&>(buffer);

        for (auto i = 0; i < 12; i++)
            if (p.keys & 1 << i)
                Controller::m_keys |= 1 << i;

        if (p.keys & 1 << 12)
            Controller::m_keys |= N3DS_KEY_ZL;
        if (p.keys & 1 << 13)
            Controller::m_keys |= N3DS_KEY_ZR;

        Controller::m_touch.x = Normalize::touchX(p.touch_x);
        Controller::m_touch.y = Normalize::touchY(p.touch_y);

        Controller::m_lstick.x = Normalize::stickL(p.lx);
        Controller::m_lstick.y = Normalize::stickL(p.ly);
        Controller::m_rstick.x = Normalize::stickR(p.rx);
        Controller::m_rstick.y = Normalize::stickR(p.ry);

        break;
    }
    };
}

void Application::writeSettings(void) const
{
    std::filesystem::path fullpath(std::getenv("HOME"));

    fullpath.append(settings_filename);

    if (!std::filesystem::exists(fullpath)) {
        std::fstream file(fullpath, std::ios::out);

        if (file.is_open()) {
            file.write(message, strlen(message));
            file.close();
        }
    }
}

bool Application::readSettings(void)
{
#define BUFFER_SIZE 1024
    bool                    res = true;
    char                    buffer[BUFFER_SIZE];
    std::filesystem::path   fullpath(std::getenv("HOME"));
    std::fstream            file(fullpath.append(settings_filename), std::ios::in);

    Controller::resetButtons();

    if (file.is_open()) {
        while(file.getline(buffer, BUFFER_SIZE)) {
            std::string line(buffer);
            std::string left, right;

            std::transform(line.begin(), line.end(), line.begin(), ::toupper);

            if (line.find_first_of("#") != std::string::npos)
                continue;

            auto space_pos = line.find_first_of(' ');

            left = line.substr(0, space_pos);
            right = line.substr(space_pos + 1);

            if (line.find("PORT") != std::string::npos)
                res &= Network::setPort(right);
            else if (line.find("TIMEOUT") != std::string::npos)
                res &= Network::setTimeout(right);
            else if (line.find("MODE") != std::string::npos)
                res &= Application::setMode(right);
            else {
                res &= Controller::replaceButton(left, right);
            }
        }

        file.close();

        return res;
    }

    return false;
}

bool Application::isRunning(void)
{
    return (Application::Network::isRunning() && Application::Controller::isRunning());
}

bool Application::setMode(const std::string &mode)
{
    if (mode.size() == 1 && mode.find_first_not_of("0123456789") == std::string::npos) {
        int mode_value = std::stoi(mode);

        if (mode_value >= MODE::DEFAULT && mode_value <= MODE::Lite_V2) {
            settings.mode = mode_value;

            setBufferSize();
        }
    }

    return false;
}

void Application::setBufferSize(void) const
{
    switch(settings.mode){
    case MODE::DEFAULT: buffer_size = sizeof(Packet); break;
    case MODE::Lite_V1: buffer_size = sizeof(Packet_lite_v1); break;
    case MODE::Lite_V2: buffer_size = sizeof(Packet_lite_v2); break;
    default:break;
    }
}

void Application::checkSettingsFile(void)
{
    time_now = std::chrono::system_clock::now();
    time_passed = time_now - time_start;

    if (time_passed.count() > 10) {
        if (!std::filesystem::exists(settingsFilePath)) {
            writeSettings();
            return;
        }

        new_write_time = std::filesystem::last_write_time(settingsFilePath);
        time_start = std::chrono::system_clock::now();

        if (last_write_time != new_write_time) {
            last_write_time = new_write_time;
            readSettings();
        }
    }
}
