#include "application.hpp"

int main(int argc, char *argv[])
{
    Application app;

    if (app.isRunning()) {
        app.write_settings();
        app.read_settings();

        app.mainLoop();
    }

    return 0;
}
