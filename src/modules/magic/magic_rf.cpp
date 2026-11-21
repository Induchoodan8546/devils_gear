#include "magic_rf.h"
#include "magic_feedback.h"
#include "magic_store.h"
#include "magic_ui.h"
#include "core/display.h"
#include "core/mykeyboard.h"
#include "core/sd_functions.h"
#include "core/utils.h"
#include "modules/rf/rf_scan.h"
#include "modules/rf/rf_send.h"
#include <globals.h>

bool magicFireTrigger(const String &name) {
    auto items = magicLoadTriggers();
    MagicTrigger *t = magicFindTrigger(items, name);
    if (!t) {
        displayError("Trigger missing", true);
        magicFeedbackError();
        return false;
    }
    FS *fs = magicReadFS(t->file);
    if (!fs) {
        displayError("File not found", true);
        magicFeedbackError();
        return false;
    }
    RfCodes data{};
    if (!readSubFile(fs, t->file, data)) {
        displayError("Bad .sub file", true);
        magicFeedbackError();
        return false;
    }
    magicDrawStatus("Prop Triggers", t->name, "Transmitting...");
    magicFeedbackTransmit();
    txSubFile(data, true);
    displaySuccess(t->name, true);
    magicFeedbackSuccess();
    return true;
}

void magicImportTrigger() {
    FS *fs = nullptr;
    options = {
        {"LittleFS", [&]() { fs = &LittleFS; }},
    };
    if (setupSdCard()) options.insert(options.begin(), {"SD Card", [&]() { fs = &SD; }});
    options.push_back({"Cancel", [&]() { fs = nullptr; }});
    loopOptions(options, MENU_TYPE_SUBMENU, "Import from");
    if (!fs) return;

    String path = loopSD(*fs, true, "SUB", "/");
    if (path.isEmpty()) return;

    String name = keyboard(path.substring(path.lastIndexOf('/') + 1), 24, "Stage name:");
    if (name.isEmpty() || name == "\x1B") return;

    String dest = String(MAGIC_TRIGGERS_DIR) + "/" + magicSanitizeFilename(name) + ".sub";
    if (!magicCopyInto(*fs, path, dest)) {
        displayError("Copy failed", true);
        magicFeedbackError();
        return;
    }
    magicAddTrigger({name, dest});
    displaySuccess("Trigger saved", true);
    magicFeedbackSuccess();
}

void magicCaptureTrigger() {
    displayInfo("Capture, then Import", true);
    RFScan();
}

void magicManageTriggers() {
    auto items = magicLoadTriggers();
    if (items.empty()) {
        displayWarning("No props yet", true);
        return;
    }
    options = {};
    for (size_t i = 0; i < items.size(); i++) {
        String name = items[i].name;
        options.push_back({name, [name]() {
            options = {
                {"Rename",
                 [name]() {
                     String n = keyboard(name, 24, "New name:");
                     if (n.isEmpty() || n == "\x1B") return;
                     auto list = magicLoadTriggers();
                     for (auto &t : list) {
                         if (t.name == name) t.name = n;
                     }
                     magicSaveTriggers(list);
                 }},
                {"Delete",
                 [name]() {
                     magicRemoveTrigger(name);
                     displaySuccess("Deleted", true);
                 }},
                {"Back", []() {}},
            };
            loopOptions(options, MENU_TYPE_SUBMENU, name.c_str());
        }});
    }
    options.push_back({"Back", []() {}});
    loopOptions(options, MENU_TYPE_SUBMENU, "Manage Triggers");
}

void magicPropTriggersMenu() {
    returnToMenu = false;
    while (!returnToMenu) {
        auto items = magicLoadTriggers();
        options = {};
        for (size_t i = 0; i < items.size(); i++) {
            String name = items[i].name;
            options.push_back({name, [name]() { magicFireTrigger(name); }});
        }
        if (items.empty()) options.push_back({"No props yet", []() { displayInfo("Import or capture a .sub", true); }});
        options.push_back({"Import .sub", magicImportTrigger});
        options.push_back({"Capture RF", magicCaptureTrigger});
        options.push_back({"Manage", magicManageTriggers});
        addOptionToMainMenu();
        loopOptions(options, MENU_TYPE_SUBMENU, "Prop Triggers");
    }
}
