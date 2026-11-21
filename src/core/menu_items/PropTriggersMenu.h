#ifndef __PROP_TRIGGERS_MENU_H__
#define __PROP_TRIGGERS_MENU_H__

#include <MenuItemInterface.h>

class PropTriggersMenu : public MenuItemInterface {
public:
    PropTriggersMenu() : MenuItemInterface("Prop Triggers") {}
    void optionsMenu(void);
    void drawIcon(float scale);
    bool hasTheme() { return false; }
    const String &themePath() override {
        static const String p = "";
        return p;
    }
};

#endif
