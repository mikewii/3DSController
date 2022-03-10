#include "application.hpp"

int main(int argc, char *argv[])
{
    Application app;

    while(app.isRunning()) {
        app.print_packet();

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
