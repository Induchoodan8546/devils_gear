#include "magic_ui.h"
#include "magic_feedback.h"
#include "core/display.h"
#include "core/mykeyboard.h"
#include "core/sd_functions.h"
#include <globals.h>

#ifndef DEVILS_GEAR_VERSION
#define DEVILS_GEAR_VERSION "1.0.0"
#endif

void magicWaitKey() {
    while (check(SelPress) || check(EscPress) || check(AnyKeyPress)) delay(10);
    while (!check(SelPress) && !check(EscPress) && !check(AnyKeyPress)) delay(30);
}

void magicDrawStatus(const String &title, const String &line1, const String &line2) {
    drawMainBorderWithTitle(title);
    tft.setTextSize(FM);
    tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);
    tft.drawCentreString(line1, tftWidth / 2, tftHeight / 2 - LH * FM, 1);
    if (line2.length()) {
        tft.setTextSize(FP);
        tft.setTextColor(bruceConfig.secColor, bruceConfig.bgColor);
        tft.drawCentreString(line2, tftWidth / 2, tftHeight / 2 + 6, 1);
    }
    printFootnote("Press to continue");
}

void magicBootScreen() {
    bool drewImage = false;
    auto tryJpeg = [&](FS &fs, const String &path) {
        if (!fs.exists(path)) return false;
        tft.fillScreen(bruceConfig.bgColor);
        drawImg(fs, path, 0, 0, true);
        return true;
    };

    if (sdcardMounted) {
        drewImage = tryJpeg(SD, "/magic/theme/boot.jpg") || tryJpeg(SD, "/boot.jpg");
    }
    if (!drewImage) {
        setupLittleFS();
        drewImage = tryJpeg(LittleFS, "/boot.jpg") || tryJpeg(LittleFS, "/magic/theme/boot.jpg");
    }

    if (!drewImage) {
        tft.fillScreen(bruceConfig.bgColor);
        tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);
        tft.setTextSize(FM);
        tft.drawCentreString("DEVIL'S GEAR", tftWidth / 2, tftHeight / 2 - 28, 1);
        tft.setTextColor(bruceConfig.secColor, bruceConfig.bgColor);
        tft.setTextSize(FP);
        tft.drawCentreString("EXOTIC MAGICK", tftWidth / 2, tftHeight / 2 - 6, 1);
        tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);
        tft.drawCentreString("DG " + String(DEVILS_GEAR_VERSION), tftWidth / 2, tftHeight / 2 + 16, 1);

        int y = tftHeight / 2 + 36;
        tft.drawFastHLine(tftWidth / 2 - 40, y, 80, bruceConfig.priColor);
        tft.fillCircle(tftWidth / 2, y, 3, bruceConfig.secColor);
    }

    unsigned long start = millis();
    while (millis() - start < 2800) {
        if (check(AnyKeyPress)) break;
        delay(20);
    }
    tft.fillScreen(bruceConfig.bgColor);
}

void magicShowReveal(const String &title, const String &text, bool peek) {
    tft.fillScreen(bruceConfig.bgColor);
    if (!peek) magicFeedbackRevealStart();

    uint16_t titleColor = peek ? bruceConfig.priColor : bruceConfig.secColor;
    uint16_t bodyColor = peek ? bruceConfig.secColor : bruceConfig.priColor;
    int titleSize = peek ? FP : FM;
    int bodySize = peek ? FP : FM;

    tft.setTextColor(titleColor, bruceConfig.bgColor);
    tft.setTextSize(titleSize);
    tft.drawCentreString(title.substring(0, 28), tftWidth / 2, peek ? 8 : 18, 1);

    tft.setTextColor(bodyColor, bruceConfig.bgColor);
    tft.setTextSize(bodySize);
    int maxChars = max(8, (tftWidth - 24) / (LW * bodySize));
    String remaining = text;
    int lineY = peek ? 28 : tftHeight / 2 - 24;
    int lineH = LH * bodySize + 2;
    while (remaining.length() && lineY + lineH < tftHeight - 18) {
        String line;
        if ((int)remaining.length() <= maxChars) {
            line = remaining;
            remaining = "";
        } else {
            int cut = remaining.lastIndexOf(' ', maxChars);
            if (cut < maxChars / 2) cut = maxChars;
            line = remaining.substring(0, cut);
            remaining = remaining.substring(cut);
            remaining.trim();
        }
        tft.drawCentreString(line, tftWidth / 2, lineY, 1);
        lineY += lineH;
    }

    tft.setTextSize(FP);
    tft.setTextColor(bruceConfig.secColor, bruceConfig.bgColor);
    tft.drawCentreString(peek ? "peek  ·  press to hide" : "press to close", tftWidth / 2, tftHeight - 16, 1);

    magicWaitKey();
    if (!peek) magicFeedbackRevealStop();
    tft.fillScreen(bruceConfig.bgColor);
}
