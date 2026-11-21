#ifndef __MAGIC_FEEDBACK_H__
#define __MAGIC_FEEDBACK_H__

void magicFeedbackBoot();
void magicFeedbackTransmit();
void magicFeedbackSuccess();
void magicFeedbackError();
void magicFeedbackRevealStart();
void magicFeedbackRevealStop();
void magicLedCrimson();
void magicLedGold();
void magicLedOff();
void magicLedRainbow();
bool magicFeedbackEnabled();
void magicSetFeedbackEnabled(bool enabled);

#endif
