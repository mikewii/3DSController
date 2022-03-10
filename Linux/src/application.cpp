#include "application.hpp"

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
