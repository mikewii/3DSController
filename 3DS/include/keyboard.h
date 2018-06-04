#pragma once

extern const char keyboardChars[60];

extern unsigned char keyboardActive;
extern unsigned char keyboardToggle;

extern unsigned char keyboardGfx[320 * 240 * 3];

void preRenderKeyboard(void);
extern void drawKeyboard(void);

int swkbd(char* out, const char* htext, const char* def, int maxlength);
int swkbd_int(int* out, const char* htext, int def);