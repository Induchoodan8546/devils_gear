#include "magic_store.h"
#include "core/sd_functions.h"
#include <LittleFS.h>
#include <SD.h>
#include <cctype>
#include <globals.h>

static void writeEmptyArray(const String &path) {
    JsonDocument doc;
    doc.to<JsonArray>();
    magicWriteJson(path, doc);
}

void magicEnsureSeed() {
    setupLittleFS();
    magicEnsureDir(MAGIC_DIR);
    magicEnsureDir(MAGIC_TRIGGERS_DIR);
    magicEnsureDir(MAGIC_IR_DIR);

    if (!LittleFS.exists(MAGIC_CONFIG_PATH) && !(sdcardMounted && SD.exists(MAGIC_CONFIG_PATH))) {
        JsonDocument doc;
        doc["feedback"] = true;
        magicWriteJson(MAGIC_CONFIG_PATH, doc);
    }
    if (!LittleFS.exists(MAGIC_TRIGGERS_INDEX) && !(sdcardMounted && SD.exists(MAGIC_TRIGGERS_INDEX))) {
        writeEmptyArray(MAGIC_TRIGGERS_INDEX);
    }
    if (!LittleFS.exists(MAGIC_CARDS_PATH) && !(sdcardMounted && SD.exists(MAGIC_CARDS_PATH))) {
        writeEmptyArray(MAGIC_CARDS_PATH);
    }
    if (!LittleFS.exists(MAGIC_IR_INDEX) && !(sdcardMounted && SD.exists(MAGIC_IR_INDEX))) {
        writeEmptyArray(MAGIC_IR_INDEX);
    }
    if (!LittleFS.exists(MAGIC_PREDICTIONS_PATH) && !(sdcardMounted && SD.exists(MAGIC_PREDICTIONS_PATH))) {
        writeEmptyArray(MAGIC_PREDICTIONS_PATH);
    }
}

bool magicEnsureDir(const String &dir) {
    setupLittleFS();
    if (!LittleFS.exists(dir) && !LittleFS.mkdir(dir)) return false;
    if (sdcardMounted && !SD.exists(dir)) SD.mkdir(dir);
    return true;
}

FS *magicReadFS(const String &path) {
    if (sdcardMounted && SD.exists(path)) return &SD;
    setupLittleFS();
    if (LittleFS.exists(path)) return &LittleFS;
    return nullptr;
}

bool magicWriteJson(const String &path, JsonDocument &doc) {
    setupLittleFS();
    int slash = path.lastIndexOf('/');
    if (slash > 0) magicEnsureDir(path.substring(0, slash));
    File file = LittleFS.open(path, FILE_WRITE);
    if (!file) return false;
    bool ok = serializeJsonPretty(doc, file) > 0;
    file.close();
    return ok;
}

bool magicReadJson(const String &path, JsonDocument &doc) {
    FS *fs = magicReadFS(path);
    if (!fs) return false;
    File file = fs->open(path, FILE_READ);
    if (!file) return false;
    DeserializationError err = deserializeJson(doc, file);
    file.close();
    return !err;
}

bool magicCopyInto(FS &from, const String &fromPath, const String &destPath) {
    setupLittleFS();
    int slash = destPath.lastIndexOf('/');
    if (slash > 0) magicEnsureDir(destPath.substring(0, slash));
    File src = from.open(fromPath, FILE_READ);
    if (!src) return false;
    File dst = LittleFS.open(destPath, FILE_WRITE);
    if (!dst) {
        src.close();
        return false;
    }
    uint8_t buf[512];
    while (src.available()) {
        size_t n = src.read(buf, sizeof(buf));
        if (dst.write(buf, n) != n) {
            src.close();
            dst.close();
            return false;
        }
    }
    src.close();
    dst.close();
    return true;
}

bool magicRemoveFile(const String &path) {
    setupLittleFS();
    if (LittleFS.exists(path)) LittleFS.remove(path);
    if (sdcardMounted && SD.exists(path)) SD.remove(path);
    return true;
}

String magicNormalizeUid(String uid) {
    uid.toUpperCase();
    uid.replace(" ", "");
    uid.replace(":", "");
    uid.replace("-", "");
    return uid;
}

String magicSanitizeFilename(String name) {
    String out;
    for (size_t i = 0; i < name.length(); i++) {
        char c = name[i];
        if (isalnum((unsigned char)c) || c == '-' || c == '_') out += c;
        else if (c == ' ') out += '_';
    }
    if (out.isEmpty()) out = "item";
    return out;
}

MagicConfig magicLoadConfig() {
    MagicConfig cfg;
    JsonDocument doc;
    if (!magicReadJson(MAGIC_CONFIG_PATH, doc)) return cfg;
    if (!doc["feedback"].isNull()) cfg.feedback = doc["feedback"].as<bool>();
    return cfg;
}

void magicSaveConfig(const MagicConfig &cfg) {
    JsonDocument doc;
    doc["feedback"] = cfg.feedback;
    magicWriteJson(MAGIC_CONFIG_PATH, doc);
}

std::vector<MagicTrigger> magicLoadTriggers() {
    std::vector<MagicTrigger> items;
    JsonDocument doc;
    if (!magicReadJson(MAGIC_TRIGGERS_INDEX, doc)) return items;
    JsonArray arr = doc.as<JsonArray>();
    for (JsonObject o : arr) {
        MagicTrigger t;
        t.name = o["name"] | "";
        t.file = o["file"] | "";
        if (t.name.length()) items.push_back(t);
    }
    return items;
}

bool magicSaveTriggers(const std::vector<MagicTrigger> &items) {
    JsonDocument doc;
    JsonArray arr = doc.to<JsonArray>();
    for (const auto &t : items) {
        JsonObject o = arr.add<JsonObject>();
        o["name"] = t.name;
        o["file"] = t.file;
    }
    return magicWriteJson(MAGIC_TRIGGERS_INDEX, doc);
}

bool magicAddTrigger(const MagicTrigger &item) {
    auto items = magicLoadTriggers();
    for (auto &t : items) {
        if (t.name == item.name) {
            t.file = item.file;
            return magicSaveTriggers(items);
        }
    }
    items.push_back(item);
    return magicSaveTriggers(items);
}

bool magicRemoveTrigger(const String &name) {
    auto items = magicLoadTriggers();
    std::vector<MagicTrigger> kept;
    for (auto &t : items) {
        if (t.name == name) magicRemoveFile(t.file);
        else kept.push_back(t);
    }
    return magicSaveTriggers(kept);
}

MagicTrigger *magicFindTrigger(std::vector<MagicTrigger> &items, const String &name) {
    for (auto &t : items) {
        if (t.name == name) return &t;
    }
    return nullptr;
}

std::vector<MagicCard> magicLoadCards() {
    std::vector<MagicCard> items;
    JsonDocument doc;
    if (!magicReadJson(MAGIC_CARDS_PATH, doc)) return items;
    JsonArray arr = doc.as<JsonArray>();
    for (JsonObject o : arr) {
        MagicCard c;
        c.uid = magicNormalizeUid(o["uid"] | "");
        c.label = o["label"] | "";
        c.type = o["type"] | "text";
        c.payload = o["payload"] | "";
        if (c.uid.length()) items.push_back(c);
    }
    return items;
}

bool magicSaveCards(const std::vector<MagicCard> &items) {
    JsonDocument doc;
    JsonArray arr = doc.to<JsonArray>();
    for (const auto &c : items) {
        JsonObject o = arr.add<JsonObject>();
        o["uid"] = c.uid;
        o["label"] = c.label;
        o["type"] = c.type;
        o["payload"] = c.payload;
    }
    return magicWriteJson(MAGIC_CARDS_PATH, doc);
}

bool magicUpsertCard(const MagicCard &card) {
    auto items = magicLoadCards();
    String uid = magicNormalizeUid(card.uid);
    for (auto &c : items) {
        if (c.uid == uid) {
            c = card;
            c.uid = uid;
            return magicSaveCards(items);
        }
    }
    MagicCard copy = card;
    copy.uid = uid;
    items.push_back(copy);
    return magicSaveCards(items);
}

bool magicRemoveCard(const String &uid) {
    String n = magicNormalizeUid(uid);
    auto items = magicLoadCards();
    std::vector<MagicCard> kept;
    for (auto &c : items) {
        if (c.uid != n) kept.push_back(c);
    }
    return magicSaveCards(kept);
}

MagicCard *magicFindCard(std::vector<MagicCard> &items, const String &uid) {
    String n = magicNormalizeUid(uid);
    for (auto &c : items) {
        if (c.uid == n) return &c;
    }
    return nullptr;
}

std::vector<MagicIrCustom> magicLoadIrCustom() {
    std::vector<MagicIrCustom> items;
    JsonDocument doc;
    if (!magicReadJson(MAGIC_IR_INDEX, doc)) return items;
    JsonArray arr = doc.as<JsonArray>();
    for (JsonObject o : arr) {
        MagicIrCustom i;
        i.name = o["name"] | "";
        i.file = o["file"] | "";
        if (i.name.length()) items.push_back(i);
    }
    return items;
}

bool magicSaveIrCustom(const std::vector<MagicIrCustom> &items) {
    JsonDocument doc;
    JsonArray arr = doc.to<JsonArray>();
    for (const auto &i : items) {
        JsonObject o = arr.add<JsonObject>();
        o["name"] = i.name;
        o["file"] = i.file;
    }
    return magicWriteJson(MAGIC_IR_INDEX, doc);
}

bool magicAddIrCustom(const MagicIrCustom &item) {
    auto items = magicLoadIrCustom();
    for (auto &i : items) {
        if (i.name == item.name) {
            i.file = item.file;
            return magicSaveIrCustom(items);
        }
    }
    items.push_back(item);
    return magicSaveIrCustom(items);
}

bool magicRemoveIrCustom(const String &name) {
    auto items = magicLoadIrCustom();
    std::vector<MagicIrCustom> kept;
    for (auto &i : items) {
        if (i.name == name) magicRemoveFile(i.file);
        else kept.push_back(i);
    }
    return magicSaveIrCustom(kept);
}

std::vector<MagicPrediction> magicLoadPredictions() {
    std::vector<MagicPrediction> items;
    JsonDocument doc;
    if (!magicReadJson(MAGIC_PREDICTIONS_PATH, doc)) return items;
    JsonArray arr = doc.as<JsonArray>();
    for (JsonObject o : arr) {
        MagicPrediction p;
        p.id = o["id"] | 0;
        p.name = o["name"] | "";
        p.text = o["text"] | "";
        items.push_back(p);
    }
    return items;
}

bool magicSavePredictions(const std::vector<MagicPrediction> &items) {
    JsonDocument doc;
    JsonArray arr = doc.to<JsonArray>();
    for (const auto &p : items) {
        JsonObject o = arr.add<JsonObject>();
        o["id"] = p.id;
        o["name"] = p.name;
        o["text"] = p.text;
    }
    return magicWriteJson(MAGIC_PREDICTIONS_PATH, doc);
}

MagicPrediction *magicFindPrediction(std::vector<MagicPrediction> &items, int id) {
    for (auto &p : items) {
        if (p.id == id) return &p;
    }
    return nullptr;
}

MagicPrediction *magicFindPredictionByName(std::vector<MagicPrediction> &items, const String &name) {
    for (auto &p : items) {
        if (p.name == name) return &p;
    }
    return nullptr;
}
