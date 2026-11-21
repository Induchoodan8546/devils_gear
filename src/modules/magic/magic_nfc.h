#ifndef __MAGIC_NFC_H__
#define __MAGIC_NFC_H__

#include <Arduino.h>

String magicReadUid(int timeoutSeconds = 10);
void magicScanCard();
void magicEnrollCard();
void magicManageCards();
void magicDispatchCard(const String &uid);
void magicCardsMenu();

#endif
