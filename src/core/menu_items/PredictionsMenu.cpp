#include "PredictionsMenu.h"
#include "core/display.h"
#include "modules/magic/magic_predict.h"

void PredictionsMenu::optionsMenu() { magicPredictionsMenu(); }

void PredictionsMenu::drawIcon(float scale) {
    clearIconArea();
    int rx = scale * 22;
    int ry = scale * 14;
    tft.drawEllipse(iconCenterX, iconCenterY, rx, ry, bruceConfig.priColor);
    tft.fillCircle(iconCenterX, iconCenterY, scale * 7, bruceConfig.secColor);
    tft.fillCircle(iconCenterX + scale * 2, iconCenterY - scale * 2, scale * 2, bruceConfig.bgColor);
    tft.drawLine(
        iconCenterX - scale * 18, iconCenterY - scale * 22, iconCenterX - scale * 6, iconCenterY - scale * 12,
        bruceConfig.priColor
    );
    tft.drawLine(
        iconCenterX + scale * 18, iconCenterY - scale * 22, iconCenterX + scale * 6, iconCenterY - scale * 12,
        bruceConfig.priColor
    );
}
