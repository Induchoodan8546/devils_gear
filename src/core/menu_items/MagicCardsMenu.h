#ifndef __MAGIC_CARDS_MENU_H__
#define __MAGIC_CARDS_MENU_H__

#include <MenuItemInterface.h>

class MagicCardsMenu : public MenuItemInterface {
public:
    MagicCardsMenu() : MenuItemInterface("Magic Cards") {}
    void optionsMenu(void);
    void drawIcon(float scale);
    bool hasTheme() { return false; }
    const String &themePath() override {
        static const String p = "";
        return p;
    }
};

#endif
