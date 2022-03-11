#include "application.hpp"
#include <filesystem>
#include <fstream>
#include <string.h>
#include <csignal>

Settings settings =
{
    .IP = {'1', '9', '2', '.', '1', '6', '8', '.', '0', '.', '1'},
    .port = 8889,
    .mode = MODE::DEFAULT,
};

Application::Application()
{
    this->setBufferSize();
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
                "# Application support 3 modes, default, lite v1 lite v2\n" \
                "# Mode values: 0 default, 1 lite v1, 2 lite v2" \
                "# default: 16byte packet with no degradation\n" \
                "# lite v1: 8byte packet with both sticks having less precision\n" \
                "# lite v2: 8byte packet with touch having less precision\n" \
                "# Settings read in [key value] pairs\n" \
                "# For controller [left right] left is 3ds button, right is expected button \n" \
                "# Port range 0 - 65535\n" \
                "#\n" \
                "# example settings:\n" \
                "# mode 2\n" \
                "# port 8889\n" \
                "# x y\n";

        if (file.is_open()) {
            file.write(message, strlen(message));
            file.close();
        }
    }
}

const bool Application::read_settings(void)
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

            if (line.find("PORT") != std::string::npos)
                res &= Network::setPort(right);
            else if (line.find("MODE") != std::string::npos)
                res &= Application::setMode(right);
            else
                res &= Controller::replace_button(left, right);
        }

        file.close();
        return res;
    } else return false;
}

const bool Application::setMode(const std::string &mode)
{
    if (mode.size() == 1 && mode.find_first_not_of("0123456789") == std::string::npos) {
        int mode_value = std::stoi(mode);

        if (mode_value >= MODE::DEFAULT && mode_value <= MODE::Lite_V2) {
            settings.mode = mode_value;

            this->setBufferSize();
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
