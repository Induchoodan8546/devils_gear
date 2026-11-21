#ifndef __IR_REVEALS_MENU_H__
#define __IR_REVEALS_MENU_H__

#include <MenuItemInterface.h>

class IRRevealsMenu : public MenuItemInterface {
public:
    IRRevealsMenu() : MenuItemInterface("IR Reveals") {}
    void optionsMenu(void);
    void drawIcon(float scale);
    bool hasTheme() { return false; }
    const String &themePath() override {
        static const String p = "";
        return p;
    }
};

#endif
