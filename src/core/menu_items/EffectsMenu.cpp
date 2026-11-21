#include "EffectsMenu.h"
#include "core/display.h"
#include "core/utils.h"
#include "modules/magic/magic_feedback.h"
#include <globals.h>

void EffectsMenu::optionsMenu() {
    returnToMenu = false;
    while (!returnToMenu) {
        options = {
            {"LED Crimson", magicLedCrimson},
            {"LED Gold", magicLedGold},
            {"LED Rainbow", magicLedRainbow},
            {"LED Off", magicLedOff},
            {"Success pattern", magicFeedbackSuccess},
            {"Transmit pattern", magicFeedbackTransmit},
            {"Error pattern", magicFeedbackError},
            {"Boot motif", magicFeedbackBoot},
        };
        addOptionToMainMenu();
        loopOptions(options, MENU_TYPE_SUBMENU, "Effects");
    }
}

void EffectsMenu::drawIcon(float scale) {
    clearIconArea();
    int s = scale * 16;
    tft.fillTriangle(
        iconCenterX, iconCenterY - s * 2, iconCenterX - s / 3, iconCenterY, iconCenterX + s / 3, iconCenterY,
        bruceConfig.secColor
    );
    tft.fillTriangle(
        iconCenterX, iconCenterY + s * 2, iconCenterX - s / 3, iconCenterY, iconCenterX + s / 3, iconCenterY,
        bruceConfig.priColor
    );
    tft.fillCircle(iconCenterX, iconCenterY, scale * 4, bruceConfig.priColor);
    tft.drawLine(iconCenterX - s, iconCenterY - s, iconCenterX + s, iconCenterY + s, bruceConfig.secColor);
    tft.drawLine(iconCenterX + s, iconCenterY - s, iconCenterX - s, iconCenterY + s, bruceConfig.secColor);
}
