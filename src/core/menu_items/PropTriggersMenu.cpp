#include "PropTriggersMenu.h"
#include "core/display.h"
#include "modules/magic/magic_rf.h"

void PropTriggersMenu::optionsMenu() { magicPropTriggersMenu(); }

void PropTriggersMenu::drawIcon(float scale) {
    clearIconArea();
    int radius = scale * 7;
    int delta = scale * 10;
    int triangle = scale * 28;
    tft.fillCircle(iconCenterX, iconCenterY - radius, radius, bruceConfig.priColor);
    tft.fillTriangle(
        iconCenterX,
        iconCenterY,
        iconCenterX - triangle / 2,
        iconCenterY + triangle,
        iconCenterX + triangle / 2,
        iconCenterY + triangle,
        bruceConfig.priColor
    );
    tft.drawArc(
        iconCenterX, iconCenterY - radius, 2.5 * radius, 2 * radius, 40, 140, bruceConfig.priColor,
        bruceConfig.bgColor
    );
    tft.drawArc(
        iconCenterX, iconCenterY - radius, 2.5 * radius + delta, 2 * radius + delta, 40, 140,
        bruceConfig.priColor, bruceConfig.bgColor
    );
    tft.drawArc(
        iconCenterX, iconCenterY - radius, 2.5 * radius, 2 * radius, 220, 320, bruceConfig.priColor,
        bruceConfig.bgColor
    );
    tft.drawArc(
        iconCenterX, iconCenterY - radius, 2.5 * radius + delta, 2 * radius + delta, 220, 320,
        bruceConfig.priColor, bruceConfig.bgColor
    );
}
