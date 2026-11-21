#ifndef __EFFECTS_MENU_H__
#define __EFFECTS_MENU_H__

#include <MenuItemInterface.h>

class EffectsMenu : public MenuItemInterface {
public:
    EffectsMenu() : MenuItemInterface("Effects") {}
    void optionsMenu(void);
    void drawIcon(float scale);
    bool hasTheme() { return false; }
    const String &themePath() override {
        static const String p = "";
        return p;
    }
};

#endif
