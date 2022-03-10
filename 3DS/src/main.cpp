#include "application.hpp"

int main(void)
{	
    Application app;

    if (app.isRunning()) {
        app.prepare();
        app.mainLoop();
    }
	
	return 0;
}
