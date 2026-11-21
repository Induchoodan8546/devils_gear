#include "IRRevealsMenu.h"
#include "core/display.h"
#include "modules/magic/magic_ir.h"

void IRRevealsMenu::optionsMenu() { magicIrRevealsMenu(); }

void IRRevealsMenu::drawIcon(float scale) {
    clearIconArea();
    int iconSize = scale * 56;
    int radius = scale * 6;
    int delta = scale * 9;
    tft.fillRect(
        iconCenterX - iconSize / 2, iconCenterY - iconSize / 2, iconSize / 6, iconSize, bruceConfig.priColor
    );
    tft.fillRect(
        iconCenterX - iconSize / 3, iconCenterY - iconSize / 3, iconSize / 6, 2 * iconSize / 3,
        bruceConfig.priColor
    );
    tft.drawCircle(iconCenterX - iconSize / 6, iconCenterY, radius, bruceConfig.priColor);
    tft.drawArc(
        iconCenterX - iconSize / 6, iconCenterY, 2.5 * radius, 2 * radius, 220, 320, bruceConfig.priColor,
        bruceConfig.bgColor
    );
    tft.drawArc(
        iconCenterX - iconSize / 6, iconCenterY, 2.5 * radius + delta, 2 * radius + delta, 220, 320,
        bruceConfig.priColor, bruceConfig.bgColor
    );
    tft.drawArc(
        iconCenterX - iconSize / 6, iconCenterY, 2.5 * radius + 2 * delta, 2 * radius + 2 * delta, 220, 320,
        bruceConfig.priColor, bruceConfig.bgColor
    );
}
