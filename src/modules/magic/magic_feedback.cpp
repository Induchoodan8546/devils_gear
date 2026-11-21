#include "magic_feedback.h"
#include "magic_store.h"
#include "core/led_control.h"
#include "modules/others/audio.h"
#include <globals.h>

#ifdef HAS_RGB_LED
#define MAGIC_CRIMSON CRGB(0x96, 0x00, 0x00)
#define MAGIC_GOLD CRGB(0xD4, 0xAF, 0x37)
#define MAGIC_RED CRGB(0xFF, 0x00, 0x00)
#endif

static void magicTone(unsigned int freq, unsigned long dur) {
    if (!bruceConfig.soundEnabled) return;
    _tone(freq, dur);
}

static void magicPulse(uint32_t color, int times, int onMs, int offMs) {
#ifdef HAS_RGB_LED
    if (!bruceConfig.ledBlinkEnabled) return;
    beginLed();
    for (int i = 0; i < times; i++) {
        setLedColor(CRGB(color));
        delay(onMs);
        setLedColor(CRGB::Black);
        if (i + 1 < times) delay(offMs);
    }
    setLedColor(CRGB(bruceConfig.ledColor));
#else
    (void)color;
    (void)times;
    (void)onMs;
    (void)offMs;
#endif
}

bool magicFeedbackEnabled() { return magicLoadConfig().feedback; }

void magicSetFeedbackEnabled(bool enabled) {
    MagicConfig cfg = magicLoadConfig();
    cfg.feedback = enabled;
    magicSaveConfig(cfg);
}

void magicFeedbackBoot() {
    if (!magicFeedbackEnabled()) return;
#ifdef HAS_RGB_LED
    if (bruceConfig.ledBlinkEnabled) {
        beginLed();
        setLedColor(MAGIC_CRIMSON);
        delay(180);
        setLedColor(CRGB::Black);
        delay(80);
        setLedColor(CRGB(bruceConfig.ledColor));
    }
#endif
    magicTone(440, 80);
    delay(40);
    magicTone(660, 80);
    delay(40);
    magicTone(880, 140);
}

void magicFeedbackTransmit() {
    if (!magicFeedbackEnabled()) return;
    magicPulse(0x960000, 1, 80, 0);
    magicTone(1200, 40);
}

void magicFeedbackSuccess() {
    if (!magicFeedbackEnabled()) return;
    magicPulse(0xD4AF37, 2, 90, 60);
    magicTone(660, 70);
    delay(30);
    magicTone(990, 110);
}

void magicFeedbackError() {
    if (!magicFeedbackEnabled()) return;
    magicPulse(0xFF0000, 2, 120, 80);
    magicTone(180, 180);
}

void magicFeedbackRevealStart() {
    if (!magicFeedbackEnabled()) return;
#ifdef HAS_RGB_LED
    if (bruceConfig.ledBlinkEnabled) {
        beginLed();
        setLedColor(MAGIC_GOLD);
    }
#endif
    magicTone(523, 90);
    delay(30);
    magicTone(784, 140);
}

void magicFeedbackRevealStop() {
#ifdef HAS_RGB_LED
    if (bruceConfig.ledBlinkEnabled) setLedColor(CRGB(bruceConfig.ledColor));
#endif
}

void magicLedCrimson() {
#ifdef HAS_RGB_LED
    beginLed();
    setLedColor(MAGIC_CRIMSON);
#endif
}

void magicLedGold() {
#ifdef HAS_RGB_LED
    beginLed();
    setLedColor(MAGIC_GOLD);
#endif
}

void magicLedOff() {
#ifdef HAS_RGB_LED
    beginLed();
    setLedColor(CRGB::Black);
#endif
}

void magicLedRainbow() {
#ifdef HAS_RGB_LED
    beginLed();
    setLedEffect(LED_EFFECT_RAINBOW_CHASE);
    ledEffects(true);
    delay(2500);
    ledEffects(false);
    setLedEffect(LED_EFFECT_SOLID);
    setLedColor(CRGB(bruceConfig.ledColor));
#endif
}
