#ifndef __MAGIC_RF_H__
#define __MAGIC_RF_H__

#include <Arduino.h>

bool magicFireTrigger(const String &name);
void magicImportTrigger();
void magicCaptureTrigger();
void magicManageTriggers();
void magicPropTriggersMenu();

#endif
