#include "application.hpp"
#include <filesystem>
#include <fstream>
#include <string.h>
#include <csignal>

Settings settings =
{
    .IP = {'1', '9', '2', '.', '1', '6', '8', '.', '0', '.', '1'},
    .port = 8889,
    .mode = MODE::Lite_V2,
};

Application::Application()
{
    // apply mode settings
    switch(settings.mode){
    case MODE::DEFAULT: buffer_size = sizeof(Packet); break;
    case MODE::Lite_V1: buffer_size = sizeof(Packet_lite_v1); break;
    case MODE::Lite_V2: buffer_size = sizeof(Packet_lite_v2); break;
    default:break;
    }
}




void Application::mainLoop(void)
{
    static bool exitRequested = false;
    auto        signal_handler = [](int signal) { exitRequested = true; };

    std::signal(SIGINT, signal_handler);

    while(this->isRunning()) {
        //app.print_packet();

        if (this->receive()) {
            this->preprocess();
            this->process();
        } else {
            this->panic();
        }

        if (exitRequested) break;
    }
}

void Application::preprocess(void)
{
    switch(settings.mode){
    case MODE::DEFAULT:{
        Packet& p = reinterpret_cast<Packet&>(buffer);

        Controller::keys = p.keys;
        Controller::touch.x = p.touch.x;
        Controller::touch.y = p.touch.y;
        Controller::lstick.x = Normalize::l_stick(p.leftStick.x);
        Controller::lstick.y = Normalize::l_stick(p.leftStick.y);
        Controller::rstick.x = Normalize::r_stick(p.rightStick.x);
        Controller::rstick.y = Normalize::r_stick(p.rightStick.y);

        break;
    }
    case MODE::Lite_V1:{
        Packet_lite_v1& p = reinterpret_cast<Packet_lite_v1&>(buffer);

        for (int i = 0; i < 12; i++)
            if (p.keys & 1 << i)
                Controller::keys |= 1 << i;

        if (p.keys & 1 << 12)
            Controller::keys |= N3DS_LITE_KEY_ZL;
        if (p.keys & 1 << 13)
            Controller::keys |= N3DS_LITE_KEY_ZR;


        Controller::touch.x = p.touch_x;
        Controller::touch.y = p.touch_y;

        Controller::lstick.x = Normalize::l_stick(p.lx);
        Controller::lstick.y = Normalize::l_stick(p.ly);
        Controller::rstick.x = Normalize::r_stick(p.rx);
        Controller::rstick.y = Normalize::r_stick(p.ry);

        break;
    }
    case MODE::Lite_V2:{
        Packet_lite_v2& p = reinterpret_cast<Packet_lite_v2&>(buffer);

        for (int i = 0; i < 12; i++)
            if (p.keys & 1 << i)
                Controller::keys |= 1 << i;

        if (p.keys & 1 << 12)
            Controller::keys |= N3DS_KEY_ZL;
        if (p.keys & 1 << 13)
            Controller::keys |= N3DS_KEY_ZR;

        Controller::touch.x = Normalize::touch_x(p.touch_x);
        Controller::touch.y = Normalize::touch_y(p.touch_y);

        Controller::lstick.x = Normalize::l_stick(p.lx);
        Controller::lstick.y = Normalize::l_stick(p.ly);
        Controller::rstick.x = Normalize::r_stick(p.rx);
        Controller::rstick.y = Normalize::r_stick(p.ry);

        break;
    }
    }
}

void Application::write_settings(void) const
{
    std::filesystem::path   fullpath(std::getenv("HOME"));
    fullpath.append(this->settings_filename);

    if (!std::filesystem::exists(fullpath)) {
        std::fstream file(fullpath, std::ios::out);
        static const char* message =
                "# Custom controller map\n" \
                "# [left right] left is 3ds button, right is expected button \n" \
                "# example:\n" \
                "# x y\n";

        if (file.is_open()) {
            file.write(message, strlen(message));
            file.close();
        }
    }
}

const bool Application::read_settings(void) const
{
#define BUFFER_SIZE 1024
    bool                    res = true;
    char                    buffer[BUFFER_SIZE];
    std::filesystem::path   fullpath(std::getenv("HOME"));
    std::fstream            file(fullpath.append(this->settings_filename), std::ios::in);

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

            res &= Controller::replace_button(left, right);
        }

        file.close();
        return res;
    } else return false;
}
