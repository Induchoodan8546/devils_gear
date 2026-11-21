#include "magic_ir.h"
#include "magic_feedback.h"
#include "magic_ir_codes.h"
#include "magic_store.h"
#include "magic_ui.h"
#include "core/display.h"
#include "core/mykeyboard.h"
#include "core/sd_functions.h"
#include "core/utils.h"
#include "modules/ir/TV-B-Gone.h"
#include "modules/ir/custom_ir.h"
#include "modules/ir/ir_read.h"
#include <globals.h>

static bool fireShortcut(const MagicIrShortcut &s) {
    IRCode code;
    code.protocol = s.protocol;
    code.address = s.address;
    code.command = s.command;
    code.name = s.name;
    magicDrawStatus("IR Reveals", String(s.brand) + " " + s.name, "Sending...");
    magicFeedbackTransmit();
    sendIRCommand(&code, true);
    displaySuccess(String(s.brand) + " " + s.name, true);
    magicFeedbackSuccess();
    return true;
}

bool magicFireIr(const String &label) {
    if (label == "Universal TV Off") {
        magicFeedbackTransmit();
        StartTvBGone();
        magicFeedbackSuccess();
        return true;
    }
    for (int i = 0; i < MAGIC_IR_SHORTCUT_COUNT; i++) {
        String built = String(MAGIC_IR_SHORTCUTS[i].brand) + " " + MAGIC_IR_SHORTCUTS[i].name;
        if (built == label || String(MAGIC_IR_SHORTCUTS[i].name) == label) {
            return fireShortcut(MAGIC_IR_SHORTCUTS[i]);
        }
    }
    auto customs = magicLoadIrCustom();
    for (auto &c : customs) {
        if (c.name == label) {
            FS *fs = magicReadFS(c.file);
            if (!fs) {
                displayError("IR file missing", true);
                magicFeedbackError();
                return false;
            }
            magicDrawStatus("IR Reveals", c.name, "Sending...");
            magicFeedbackTransmit();
            txIrFile(fs, c.file, true);
            displaySuccess(c.name, true);
            magicFeedbackSuccess();
            return true;
        }
    }
    displayError("IR shortcut missing", true);
    magicFeedbackError();
    return false;
}

void magicIrBrandMenu(const String &brand) {
    options = {};
    for (int i = 0; i < MAGIC_IR_SHORTCUT_COUNT; i++) {
        if (brand != MAGIC_IR_SHORTCUTS[i].brand) continue;
        const MagicIrShortcut *s = &MAGIC_IR_SHORTCUTS[i];
        options.push_back({s->name, [s]() { fireShortcut(*s); }});
    }
    options.push_back({"Back", []() {}});
    loopOptions(options, MENU_TYPE_SUBMENU, brand.c_str());
}

void magicImportIr() {
    FS *fs = nullptr;
    options = {
        {"LittleFS", [&]() { fs = &LittleFS; }},
    };
    if (setupSdCard()) options.insert(options.begin(), {"SD Card", [&]() { fs = &SD; }});
    options.push_back({"Cancel", [&]() { fs = nullptr; }});
    loopOptions(options, MENU_TYPE_SUBMENU, "Import from");
    if (!fs) return;

    String path = loopSD(*fs, true, "IR", "/");
    if (path.isEmpty()) return;

    String name = keyboard(path.substring(path.lastIndexOf('/') + 1), 24, "Shortcut name:");
    if (name.isEmpty() || name == "\x1B") return;

    String dest = String(MAGIC_IR_DIR) + "/" + magicSanitizeFilename(name) + ".ir";
    if (!magicCopyInto(*fs, path, dest)) {
        displayError("Copy failed", true);
        magicFeedbackError();
        return;
    }
    magicAddIrCustom({name, dest});
    displaySuccess("IR saved", true);
    magicFeedbackSuccess();
}

void magicLearnIr() {
    displayInfo("Learn, then Import", true);
    IrRead();
}

void magicManageIr() {
    auto items = magicLoadIrCustom();
    if (items.empty()) {
        displayWarning("No custom IR yet", true);
        return;
    }
    options = {};
    for (size_t i = 0; i < items.size(); i++) {
        String name = items[i].name;
        options.push_back({name, [name]() {
            options = {
                {"Delete",
                 [name]() {
                     magicRemoveIrCustom(name);
                     displaySuccess("Deleted", true);
                 }},
                {"Back", []() {}},
            };
            loopOptions(options, MENU_TYPE_SUBMENU, name.c_str());
        }});
    }
    options.push_back({"Back", []() {}});
    loopOptions(options, MENU_TYPE_SUBMENU, "Custom IR");
}

void magicIrRevealsMenu() {
    returnToMenu = false;
    while (!returnToMenu) {
        options = {
            {"Samsung", []() { magicIrBrandMenu("Samsung"); }},
            {"LG", []() { magicIrBrandMenu("LG"); }},
            {"Sony", []() { magicIrBrandMenu("Sony"); }},
            {"Universal TV Off", []() { magicFireIr("Universal TV Off"); }},
        };
        auto customs = magicLoadIrCustom();
        for (size_t i = 0; i < customs.size(); i++) {
            String name = customs[i].name;
            options.push_back({name, [name]() { magicFireIr(name); }});
        }
        options.push_back({"Import .ir", magicImportIr});
        options.push_back({"Learn IR", magicLearnIr});
        options.push_back({"Manage", magicManageIr});
        addOptionToMainMenu();
        loopOptions(options, MENU_TYPE_SUBMENU, "IR Reveals");
    }
}
