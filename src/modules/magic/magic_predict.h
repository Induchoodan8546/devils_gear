#ifndef __MAGIC_PREDICT_H__
#define __MAGIC_PREDICT_H__

#include <Arduino.h>

void magicPreparePrediction();
void magicRevealMenu(bool peek);
void magicRevealByKey(const String &key);
void magicClearPrediction();
void magicPredictionsMenu();

#endif
