#ifndef __MAGIC_UI_H__
#define __MAGIC_UI_H__

#include <Arduino.h>

void magicBootScreen();
void magicShowReveal(const String &title, const String &text, bool peek);
void magicWaitKey();
void magicDrawStatus(const String &title, const String &line1, const String &line2 = "");

#endif
