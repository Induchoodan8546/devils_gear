#include "MagicCardsMenu.h"
#include "core/display.h"
#include "modules/magic/magic_nfc.h"

void MagicCardsMenu::optionsMenu() { magicCardsMenu(); }

void MagicCardsMenu::drawIcon(float scale) {
    clearIconArea();
    int w = scale * 40;
    int h = scale * 56;
    int r = scale * 4;
    tft.fillRoundRect(iconCenterX - w / 2, iconCenterY - h / 2, w, h, r, bruceConfig.priColor);
    tft.fillRoundRect(
        iconCenterX - w / 2 + scale * 3, iconCenterY - h / 2 + scale * 3, w - scale * 6, h - scale * 6, r,
        bruceConfig.bgColor
    );
    tft.fillCircle(iconCenterX, iconCenterY - scale * 6, scale * 6, bruceConfig.secColor);
    tft.drawFastHLine(iconCenterX - w / 4, iconCenterY + scale * 10, w / 2, bruceConfig.priColor);
    tft.drawFastHLine(iconCenterX - w / 5, iconCenterY + scale * 16, w / 2.5, bruceConfig.priColor);
}
