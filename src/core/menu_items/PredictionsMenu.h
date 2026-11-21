#ifndef __PREDICTIONS_MENU_H__
#define __PREDICTIONS_MENU_H__

#include <MenuItemInterface.h>

class PredictionsMenu : public MenuItemInterface {
public:
    PredictionsMenu() : MenuItemInterface("Predictions") {}
    void optionsMenu(void);
    void drawIcon(float scale);
    bool hasTheme() { return false; }
    const String &themePath() override {
        static const String p = "";
        return p;
    }
};

#endif
