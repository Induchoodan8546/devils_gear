#ifndef __MAGIC_STORE_H__
#define __MAGIC_STORE_H__

#include <Arduino.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <vector>

#define MAGIC_DIR "/magic"
#define MAGIC_TRIGGERS_DIR "/magic/triggers"
#define MAGIC_IR_DIR "/magic/ir"
#define MAGIC_CONFIG_PATH "/magic/config.json"
#define MAGIC_CARDS_PATH "/magic/cards.json"
#define MAGIC_PREDICTIONS_PATH "/magic/predictions.json"
#define MAGIC_TRIGGERS_INDEX "/magic/triggers/index.json"
#define MAGIC_IR_INDEX "/magic/ir/index.json"
#define MAGIC_MAX_PREDICTIONS 8

struct MagicTrigger {
    String name;
    String file;
};

struct MagicCard {
    String uid;
    String label;
    String type;
    String payload;
};

struct MagicIrCustom {
    String name;
    String file;
};

struct MagicPrediction {
    int id;
    String name;
    String text;
};

struct MagicConfig {
    bool feedback = true;
};

void magicEnsureSeed();
bool magicEnsureDir(const String &dir);
FS *magicReadFS(const String &path);
bool magicWriteJson(const String &path, JsonDocument &doc);
bool magicReadJson(const String &path, JsonDocument &doc);
bool magicCopyInto(FS &from, const String &fromPath, const String &destPath);
bool magicRemoveFile(const String &path);
String magicNormalizeUid(String uid);
String magicSanitizeFilename(String name);

MagicConfig magicLoadConfig();
void magicSaveConfig(const MagicConfig &cfg);

std::vector<MagicTrigger> magicLoadTriggers();
bool magicSaveTriggers(const std::vector<MagicTrigger> &items);
bool magicAddTrigger(const MagicTrigger &item);
bool magicRemoveTrigger(const String &name);
MagicTrigger *magicFindTrigger(std::vector<MagicTrigger> &items, const String &name);

std::vector<MagicCard> magicLoadCards();
bool magicSaveCards(const std::vector<MagicCard> &items);
bool magicUpsertCard(const MagicCard &card);
bool magicRemoveCard(const String &uid);
MagicCard *magicFindCard(std::vector<MagicCard> &items, const String &uid);

std::vector<MagicIrCustom> magicLoadIrCustom();
bool magicSaveIrCustom(const std::vector<MagicIrCustom> &items);
bool magicAddIrCustom(const MagicIrCustom &item);
bool magicRemoveIrCustom(const String &name);

std::vector<MagicPrediction> magicLoadPredictions();
bool magicSavePredictions(const std::vector<MagicPrediction> &items);
MagicPrediction *magicFindPrediction(std::vector<MagicPrediction> &items, int id);
MagicPrediction *magicFindPredictionByName(std::vector<MagicPrediction> &items, const String &name);

#endif
