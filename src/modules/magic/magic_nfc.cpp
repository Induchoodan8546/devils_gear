#include "magic_nfc.h"
#include "magic_feedback.h"
#include "magic_ir.h"
#include "magic_predict.h"
#include "magic_rf.h"
#include "magic_store.h"
#include "magic_ui.h"
#include "core/display.h"
#include "core/mykeyboard.h"
#include "core/utils.h"
#ifndef LITE_VERSION
#include "modules/rfid/ST25R3916.h"
#endif
#include "modules/rfid/PN532.h"
#include "modules/rfid/RFID2.h"
#include "modules/rfid/RFIDInterface.h"
#include <globals.h>

static RFIDInterface *magicMakeRfid() {
    switch (bruceConfigPins.rfidModule) {
        case PN532_I2C_MODULE: return new PN532(PN532::I2C);
#ifdef M5STICK
        case PN532_I2C_SPI_MODULE: return new PN532(PN532::I2C_SPI);
#endif
        case PN532_SPI_MODULE: return new PN532(PN532::SPI);
        case RC522_SPI_MODULE: return new RFID2(false);
#if !defined(LITE_VERSION)
        case ST25R3916_SPI_MODULE: return new ST25R3916(ST25R3916::SPI_MODE);
        case ST25R3916_I2C_MODULE: return new ST25R3916(ST25R3916::I2C_MODE);
#endif
        case M5_RFID2_MODULE:
        default: return new RFID2();
    }
}

String magicReadUid(int timeoutSeconds) {
    RFIDInterface *rfid = magicMakeRfid();
    if (!rfid) return "";
    if (!rfid->begin()) {
        delete rfid;
        displayError("NFC init failed", true);
        magicFeedbackError();
        return "";
    }

    magicDrawStatus("Magic Cards", "Present card...", "Back to cancel");
    uint32_t start = millis();
    String uid;
    while (millis() - start < (uint32_t)timeoutSeconds * 1000) {
        if (check(EscPress)) break;
        int st = rfid->read();
        if (st == RFIDInterface::SUCCESS) {
            uid = magicNormalizeUid(rfid->printableUID.uid);
            break;
        }
        delay(80);
    }
    delete rfid;
    return uid;
}

void magicDispatchCard(const String &uid) {
    auto cards = magicLoadCards();
    MagicCard *c = magicFindCard(cards, uid);
    if (!c) {
        displayWarning("Unmapped: " + uid, true);
        magicFeedbackError();
        return;
    }
    displaySuccess(c->label, false);
    if (c->type == "prediction") magicRevealByKey(c->payload);
    else if (c->type == "trigger") magicFireTrigger(c->payload);
    else if (c->type == "ir") magicFireIr(c->payload);
    else magicShowReveal(c->label, c->payload.length() ? c->payload : c->label, false);
}

void magicScanCard() {
    String uid = magicReadUid(12);
    if (uid.isEmpty()) {
        displayWarning("No card", true);
        return;
    }
    magicDispatchCard(uid);
}

void magicEnrollCard() {
    String uid = magicReadUid(12);
    if (uid.isEmpty()) {
        displayWarning("No card", true);
        return;
    }
    String label = keyboard("Card", 24, "Card label:");
    if (label.isEmpty() || label == "\x1B") return;

    String type = "text";
    String payload;
    options = {
        {"Prediction", [&]() { type = "prediction"; }},
        {"Prop Trigger", [&]() { type = "trigger"; }},
        {"IR Reveal", [&]() { type = "ir"; }},
        {"Text", [&]() { type = "text"; }},
    };
    loopOptions(options, MENU_TYPE_SUBMENU, "Map effect");

    if (type == "prediction") {
        auto preds = magicLoadPredictions();
        if (preds.empty()) {
            displayWarning("No predictions", true);
            return;
        }
        options = {};
        for (auto &p : preds) {
            String name = p.name;
            options.push_back({name, [&payload, name]() { payload = name; }});
        }
        loopOptions(options, MENU_TYPE_SUBMENU, "Which prediction");
    } else if (type == "trigger") {
        auto trigs = magicLoadTriggers();
        if (trigs.empty()) {
            displayWarning("No triggers", true);
            return;
        }
        options = {};
        for (auto &t : trigs) {
            String name = t.name;
            options.push_back({name, [&payload, name]() { payload = name; }});
        }
        loopOptions(options, MENU_TYPE_SUBMENU, "Which trigger");
    } else if (type == "ir") {
        payload = keyboard("Samsung Power", 32, "IR shortcut name:");
        if (payload.isEmpty() || payload == "\x1B") return;
    } else {
        payload = keyboard("", 64, "Reveal text:");
        if (payload == "\x1B") return;
    }

    magicUpsertCard({uid, label, type, payload});
    displaySuccess("Card enrolled", true);
    magicFeedbackSuccess();
}

void magicManageCards() {
    auto cards = magicLoadCards();
    if (cards.empty()) {
        displayWarning("No cards yet", true);
        return;
    }
    options = {};
    for (size_t i = 0; i < cards.size(); i++) {
        String uid = cards[i].uid;
        String label = cards[i].label;
        options.push_back({label, [uid, label]() {
            options = {
                {"Delete",
                 [uid]() {
                     magicRemoveCard(uid);
                     displaySuccess("Deleted", true);
                 }},
                {"Back", []() {}},
            };
            loopOptions(options, MENU_TYPE_SUBMENU, label.c_str());
        }});
    }
    options.push_back({"Back", []() {}});
    loopOptions(options, MENU_TYPE_SUBMENU, "Magic Cards");
}

void magicCardsMenu() {
    returnToMenu = false;
    while (!returnToMenu) {
        options = {
            {"Scan", magicScanCard},
            {"Enroll", magicEnrollCard},
            {"Manage", magicManageCards},
        };
        addOptionToMainMenu();
        loopOptions(options, MENU_TYPE_SUBMENU, "Magic Cards");
    }
}
