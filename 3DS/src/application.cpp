#include "application.hpp"

#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <fstream>
#include <algorithm>
#include <limits>

#define BUFFER_SIZE 1024

Settings settings =
{
    .IP = {'1', '9', '2', '.', '1', '6', '8', '.', '0', '.', '1'},
    .port = 8889,
    .mode = MODE::Lite_V2,
};

const char* getModeStr(const int mode)
{
    switch(mode){
    case MODE::DEFAULT: return "Default";
    case MODE::Lite_V1: return "Lite V1";
    case MODE::Lite_V2: return "Lite V2";
    default:return "Mode error!";
    }
}

const bool isValidIpAddress(const char* ipAddress)
{
    struct sockaddr_in sa;

    int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));

    return result != 0;
}

Application::Application()
{
    if (!Network::isRunning())
        return;

    if (R_FAILED(acInit()))
        return;

    if (R_FAILED(gspLcdInit()))
        return;

    if (R_FAILED(fsInit()))
        return;

    gfxInitDefault();
    gfxSetDoubleBuffering(GFX_TOP, false);
    gfxSetDoubleBuffering(GFX_BOTTOM, false);

    consoleInit(GFX_BOTTOM, &bottomScreen);
    consoleSelect(&bottomScreen);

    this->running = true;
}

Application::~Application()
{
    GSPLCD_PowerOnBacklight(GSPLCD_SCREEN_BOTH);

    fsExit();
    gspLcdExit();
    gfxExit();
    acExit();
}

void Application::prepare(void)
{
    this->wait_for_wifi();
    this->settings_set = this->read_settings();

    if (this->settings_set) {
        Network::setIP();
        Network::setPort();
    }

    this->get_input();
    if (this->settings_set) this->write_settings();

    GSPLCD_PowerOffBacklight(GSPLCD_SCREEN_BOTH);
    aptSetHomeAllowed(false); // because hang in aptJumpToHomeMenu, turning lcd back on doesnt help

    settings.mode = MODE::Lite_V2; // fixme
}

void Application::mainLoop(void)
{
    while (aptMainLoop() && this->isRunning()) {
        hidScanInput();
        irrstScanInput();

        u32             kDown = hidKeysHeld();
        circlePosition  leftStick;
        circlePosition  rightStick;
        touchPosition   touch;

        hidCstickRead(&rightStick);
        hidCircleRead(&leftStick);
        touchRead(&touch);

        Network::sendKeys(kDown, touch, leftStick, rightStick);

        if ((kDown & KEY_START) && (kDown & KEY_SELECT))
            this->running = false;
    }
}

void Application::get_input(void)
{
    const char* initial_IP = "";
    char        initial_port[6];


    while (aptMainLoop() && this->isRunning()) {
        hidScanInput();

        u32 kDown = hidKeysHeld();

        if (kDown & KEY_X) {
            char buffer[BUFFER_SIZE];

            if (this->settings_set) {
                initial_IP = settings.IP;
                itoa(settings.port, initial_port, 10);
            }

            memset(buffer, 0, sizeof(buffer));
            this->keyboard(buffer, "IP", initial_IP);

            if (isValidIpAddress(buffer)) {
                strncpy(settings.IP, buffer, sizeof(settings.IP));
                Network::setIP();
                this->settings_set &= true;
            } else {
                this->settings_set &= false;
                // error
            }

            memset(buffer, 0, sizeof(buffer));
            this->keyboard(buffer, "PORT", initial_port, true);

            int port = std::stoi(buffer);
            if (port <= std::numeric_limits<s16>().max()) {
                settings.port = port;
                Network::setPort();
                this->settings_set &= true;
            } else {
                this->settings_set &= false;
                // error
            }
        } else if (kDown & KEY_DUP) {
            if (settings.mode == MODE::DEFAULT)
                settings.mode = MODE::Lite_V2;
            else  settings.mode -= 1;
            this->settings_set &= true;
            svcSleepThread(200000000); // 200ms
        } else if (kDown & KEY_DDOWN) {
            if (settings.mode == MODE::Lite_V2)
                settings.mode = MODE::DEFAULT;
            else  settings.mode += 1;
            this->settings_set &= true;
            svcSleepThread(200000000); // 200ms
        } else if ((kDown & KEY_START) && (kDown & KEY_SELECT)) {
            this->running = false;
            break;
        } else if (kDown & KEY_R) {
            break;
        }

        printf("\x1b[1;0HIP: %-16s PORT: %-5d", settings.IP, settings.port);
        printf("\x1b[2;0HMODE: %s", getModeStr(settings.mode));

        printf("\x1b[4;0H[X] to configure IP and port.");
        printf("\x1b[5;0H[Dpad Up/Down] to change mode");

        printf("\x1b[7;0H[R] to start");
        printf("\x1b[8;0H[START + SELECT] to exit.");

        this->swap_buffers();
    }
}

void Application::keyboard(char *buffer, const char* hint, const char* initialText, bool isPort)
{
    static SwkbdState   swkbd;

    int limit = isPort ? 5 : 15;

    swkbdInit(&swkbd, SWKBD_TYPE_NUMPAD, 1, limit);
    swkbdSetHintText(&swkbd, hint);
    swkbdSetValidation(&swkbd, SWKBD_ANYTHING, 0, 0);
    swkbdSetFeatures(&swkbd, SWKBD_FIXED_WIDTH);

    if (!isPort)
        swkbdSetNumpadKeys(&swkbd, '.', 0);

    if (strcmp(initialText, "") != 0)
        swkbdSetInitialText(&swkbd, initialText);

    swkbdInputText(&swkbd, buffer, BUFFER_SIZE);
}

void Application::wait_for_wifi(void)
{
    while (aptMainLoop() && this->isRunning()) {
        hidScanInput();

        u32 wifiStatus = 0;
        u32 kDown = hidKeysHeld();

        ACU_GetWifiStatus(&wifiStatus);

        if (wifiStatus) break;

        printf("\x1b[1;0HWaiting for WiFi connection...");
        printf("\x1b[2;0HEnsure you are in range of an access point,");
        printf("\x1b[3;0Hand that wireless communications are enabled.");
        printf("\x1b[4;0HYou can alternatively press Start and Select to exit.");

        this->swap_buffers();

        if ((kDown & KEY_START) && (kDown & KEY_SELECT)) {
            this->running = false;
            break;
        }
    }
}

void Application::swap_buffers(void) const
{
    gfxFlushBuffers();
    gfxSwapBuffers();
    gspWaitForVBlank();
}

const bool Application::read_settings_value(const std::string &value, const int type)
{
    std::string str;
    auto        pos = value.find_first_of(" ");

    if (pos != std::string::npos) {
        str = value.substr(pos);
        str.erase(std::remove_if(str.begin(), str.end(), ::isspace), str.end()); // remove whitespace
    }

    switch(type) {
    default: return false;
    case 0:   // IP
        if (str.size() <= sizeof("000.000.000.000")) {
            strncpy(settings.IP, str.c_str(), sizeof(settings.IP) - 1);
            return true;
        }
        else return false;
    case 1:   // port
        if (str.size() <= sizeof("00000")) {
            settings.port = std::stoi(str);
            return true;
        }
        else return false;
    case 2:   // mode
        if (str.size() <= sizeof("0")) {
            settings.mode = static_cast<MODE>(std::stoi(str));
            return true;
        }
        else return false;
    }
}

const bool Application::read_settings(void)
{
    char            buffer[BUFFER_SIZE];
    std::fstream    input(this->settings_filename, std::ios::in);
    bool            noErr = true;

    if (input.is_open()) {
        while(input.getline(buffer, BUFFER_SIZE))
        {
            std::string line(buffer);
            std::transform(line.begin(), line.end(), line.begin(), ::toupper);

            if (line.find("IP:") != std::string::npos)
                noErr &= this->read_settings_value(line, 0);
            else if (line.find("PORT:") != std::string::npos)
                noErr &= this->read_settings_value(line, 1);
            else if (line.find("MODE:") != std::string::npos)
                noErr &= this->read_settings_value(line, 2);

            if (!noErr) break;
        }

        input.close();
        return noErr;
    } else return false;
}

const bool Application::write_settings(void)
{
    char            buffer[BUFFER_SIZE];
    std::fstream    output(this->settings_filename, std::ios::out);

    if (output.is_open()) {
        snprintf(buffer, BUFFER_SIZE, "IP: %s", settings.IP);
        output << buffer << std::endl;

        snprintf(buffer, BUFFER_SIZE, "PORT: %d", settings.port);
        output << buffer << std::endl;

        snprintf(buffer, BUFFER_SIZE, "MODE: %d", settings.mode);
        output << buffer << std::endl;

        output.close();
        return true;
    } else return false;
}
