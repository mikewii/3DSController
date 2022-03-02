#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <setjmp.h>

#include <3ds.h>
#include "inet_pton.h"

#include "wireless.h"
#include "settings.h"
#include "drawing.h"
#include "input.h"
#include "keyboard.h"

static jmp_buf exitJmp;

void hang(char *message)
{
    while (aptMainLoop()) {
		hidScanInput();
		
		clearScreen();
		drawString(10, 10, "%s", message);
		drawString(10, 20, "Press Start and Select to exit.");
		
		u32 kHeld = hidKeysHeld();
        if ((kHeld & KEY_START) && (kHeld & KEY_SELECT)) longjmp(exitJmp, 1);
		
		gfxFlushBuffers();
		gspWaitForVBlank();
		gfxSwapBuffers();
	}
}

int main(void)
{
	acInit();
	gfxInitDefault();
    gspLcdInit();
    fsInit();
    socInit((u32 *)memalign(0x1000, 0x100000), 0x100000);

	
	gfxSetDoubleBuffering(GFX_TOP, false);
	gfxSetDoubleBuffering(GFX_BOTTOM, false);
	
    if (setjmp(exitJmp)) goto exit;
	
	preRenderKeyboard();
	
	clearScreen();
	drawString(10, 10, "Initialising FS...");
	gfxFlushBuffers();
	gfxSwapBuffers();
	
	clearScreen();
	drawString(10, 10, "Initialising SOC...");
	gfxFlushBuffers();
	gfxSwapBuffers();
	
	
    while (aptMainLoop()) { /* Wait for WiFi; break when WiFiStatus is truthy */
		u32 wifiStatus = 0;
		ACU_GetWifiStatus(&wifiStatus);
		if(wifiStatus) break;
		
		hidScanInput();
		clearScreen();
		drawString(10, 10, "Waiting for WiFi connection...");
		drawString(10, 20, "Ensure you are in range of an access point,");
		drawString(10, 30, "and that wireless communications are enabled.");
		drawString(10, 50, "You can alternatively press Start and Select to exit.");
		
		u32 kHeld = hidKeysHeld();
        if ((kHeld & KEY_START) && (kHeld & KEY_SELECT)) longjmp(exitJmp, 1);
		
		gfxFlushBuffers();
		gspWaitForVBlank();
		gfxSwapBuffers();
	}
	
	clearScreen();
	drawString(10, 10, "Reading settings...");
	gfxFlushBuffers();
	gfxSwapBuffers();
	
    if (!readSettings()) {
		hang("Could not read 3DSController.ini!");
	}
    
	gfxFlushBuffers();
    gfxSwapBuffers();
    
    while (aptMainLoop()) {
        hidScanInput();
        u32 kDown = hidKeysHeld();

        if (kDown & KEY_X) {
            swkbd(settings.IPString, "IPString", settings.IPString, sizeof(settings.IPString));
            swkbd_int(&settings.port, "Port", settings.port);

            //inet_pton(AF_INET, settings.IPString, &saout.sin_addr);
            inet_pton4(settings.IPString, (unsigned char *)&(saout.sin_addr));
            writeSettings();
        }
        else if ((kDown & KEY_START) && (kDown & KEY_SELECT)) longjmp(exitJmp, 1);
        else if (kDown & KEY_START) break;
        
        clearScreen();

        drawString(10, 10, "IP: %s, port: %d", settings.IPString, settings.port);
        drawString(10, 20, "Press X to configure IP and port.");
        drawString(10, 30, "Press START to continue.");
        drawString(10, 40, "Press START + SELECT to exit.");

        gfxFlushBuffers();
		gspWaitForVBlank();
		gfxSwapBuffers();
    }
	
	clearScreen();
	drawString(10, 10, "Connecting to %s on port %d...", settings.IPString, settings.port);
	gfxFlushBuffers();
	gfxSwapBuffers();
	
	openSocket(settings.port);
	
	clearScreen();
	gfxFlushBuffers();
	gfxSwapBuffers();
	
    // GSPLCD_PowerOffAllBacklights();  // this shit doesnt work
    GSPLCD_PowerOffBacklight(GSPLCD_SCREEN_BOTH);
    aptSetHomeAllowed(false); // because hang in aptJumpToHomeMenu, turning lcd back on doesnt help

    while (aptMainLoop()) {
		hidScanInput();
		irrstScanInput();
		
        u32             kHeld = hidKeysHeld();
        circlePosition  leftStick;
        circlePosition  rightStick;
        touchPosition   touch;

        hidCstickRead(&rightStick);
        hidCircleRead(&leftStick);
		touchRead(&touch);
		
        sendKeys(kHeld, leftStick, rightStick, touch);

        if ((kHeld & KEY_START) && (kHeld & KEY_SELECT)) longjmp(exitJmp, 1);
	}
	
	exit:

    GSPLCD_PowerOnBacklight(GSPLCD_SCREEN_BOTH);
    while (aptMainLoop()) {
        gfxFlushBuffers();
        gspWaitForVBlank();
        gfxSwapBuffers();
        break;
    }

    socExit();
	fsExit();
    gspLcdExit();
	gfxExit();
	acExit();
	
	return 0;
}
