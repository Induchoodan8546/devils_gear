#include "magic_predict.h"
#include "magic_feedback.h"
#include "magic_store.h"
#include "magic_ui.h"
#include "core/display.h"
#include "core/mykeyboard.h"
#include "core/utils.h"
#include <globals.h>

static int nextPredictionId(const std::vector<MagicPrediction> &items) {
    int maxId = 0;
    for (auto &p : items)
        if (p.id > maxId) maxId = p.id;
    return maxId + 1;
}

void magicRevealByKey(const String &key) {
    auto items = magicLoadPredictions();
    MagicPrediction *p = magicFindPredictionByName(items, key);
    if (!p) {
        int id = key.toInt();
        if (id > 0) p = magicFindPrediction(items, id);
    }
    if (!p) {
        displayError("No prediction", true);
        magicFeedbackError();
        return;
    }
    magicShowReveal(p->name, p->text, false);
}

void magicPreparePrediction() {
    auto items = magicLoadPredictions();
    if ((int)items.size() >= MAGIC_MAX_PREDICTIONS) {
        displayWarning("All 8 slots full", true);
        return;
    }
    String name = keyboard("Slot " + String(nextPredictionId(items)), 24, "Prediction name:");
    if (name.isEmpty() || name == "\x1B") return;
    String text = keyboard("", 96, "Prediction text:");
    if (text.isEmpty() || text == "\x1B") return;
    items.push_back({nextPredictionId(items), name, text});
    magicSavePredictions(items);
    displaySuccess("Prepared", true);
    magicFeedbackSuccess();
}

static void pickAndShow(bool peek) {
    auto items = magicLoadPredictions();
    if (items.empty()) {
        displayWarning("Nothing prepared", true);
        return;
    }
    options = {};
    for (auto &p : items) {
        String name = p.name;
        options.push_back({name, [name, peek]() {
            auto list = magicLoadPredictions();
            MagicPrediction *found = magicFindPredictionByName(list, name);
            if (found) magicShowReveal(found->name, found->text, peek);
        }});
    }
    options.push_back({"Back", []() {}});
    loopOptions(options, MENU_TYPE_SUBMENU, peek ? "Peek" : "Reveal");
}

void magicRevealMenu(bool peek) { pickAndShow(peek); }

void magicClearPrediction() {
    auto items = magicLoadPredictions();
    if (items.empty()) {
        displayWarning("Nothing to clear", true);
        return;
    }
    options = {};
    for (auto &p : items) {
        String name = p.name;
        options.push_back({name, [name]() {
            auto list = magicLoadPredictions();
            std::vector<MagicPrediction> kept;
            for (auto &x : list)
                if (x.name != name) kept.push_back(x);
            magicSavePredictions(kept);
            displaySuccess("Cleared", true);
        }});
    }
    options.push_back({"Back", []() {}});
    loopOptions(options, MENU_TYPE_SUBMENU, "Clear");
}

void magicPredictionsMenu() {
    returnToMenu = false;
    while (!returnToMenu) {
        options = {
            {"Prepare", magicPreparePrediction},
            {"Reveal", []() { magicRevealMenu(false); }},
            {"Peek", []() { magicRevealMenu(true); }},
            {"Clear", magicClearPrediction},
        };
        addOptionToMainMenu();
        loopOptions(options, MENU_TYPE_SUBMENU, "Predictions");
    }
}
