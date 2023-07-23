#include "application.hpp"

int main(int, char *[])
{
    Application app;

    if (app.isRunning()) {
        app.writeSettings();
        app.readSettings();

        app.mainLoop();
    }

    return 0;
}
