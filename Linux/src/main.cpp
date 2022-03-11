#include "application.hpp"

int main(int argc, char *argv[])
{
    Application app;

    if (app.isRunning()) {
        app.write_settings();
        app.read_settings();
    }

    while(app.isRunning()) {
        //app.print_packet();

        if (app.receive()) {
            app.preprocess();
            app.process();
        }
        else {
            app.panic();
        }
    }

    return 0;
}
