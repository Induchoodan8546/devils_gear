#ifndef __CONFIG_MENU_H__
#define __CONFIG_MENU_H__

#include <MenuItemInterface.h>

class ConfigMenu : public MenuItemInterface {
public:
#ifdef DEVILS_GEAR
    ConfigMenu() : MenuItemInterface("Settings") {}
#else
    ConfigMenu() : MenuItemInterface("Config") {}
#endif

    void optionsMenu(void);
    void drawIcon(float scale);
    bool hasTheme() { return bruceConfig.theme.config; }
    const String &themePath() override { return bruceConfig.theme.paths.config; }

private:
    // Submenus
    void displayUIMenu(void);
    void ledMenu(void);
    void audioMenu(void);
    void systemMenu(void);
    void advancedMenu(void);
    void powerMenu(void);
    void pinsMenu(void);
    void devMenu(void);
#ifdef DEVILS_GEAR
    void workshopMenu(void);
#endif

    // Helper methods for complex operations
    void switchToUSBSerial(void);
    void switchToUARTSerial(void);
};

#endif
