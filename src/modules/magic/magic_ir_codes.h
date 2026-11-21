#ifndef __MAGIC_IR_CODES_H__
#define __MAGIC_IR_CODES_H__

struct MagicIrShortcut {
    const char *brand;
    const char *name;
    const char *protocol;
    const char *address;
    const char *command;
};

// Curated Flipper-style parsed codes. Addresses/commands match Bruce's sd_files library.
static const MagicIrShortcut MAGIC_IR_SHORTCUTS[] = {
    {"Samsung", "Power",  "Samsung32", "07 00 00 00", "02 00 00 00"},
    {"Samsung", "Mute",   "Samsung32", "07 00 00 00", "0F 00 00 00"},
    {"Samsung", "Vol +",  "Samsung32", "07 00 00 00", "07 00 00 00"},
    {"Samsung", "Vol -",  "Samsung32", "07 00 00 00", "0B 00 00 00"},
    {"Samsung", "CH +",   "Samsung32", "07 00 00 00", "12 00 00 00"},
    {"Samsung", "CH -",   "Samsung32", "07 00 00 00", "10 00 00 00"},
    {"LG",      "Power",  "NEC",       "04 00 00 00", "08 00 00 00"},
    {"LG",      "Mute",   "NEC",       "04 00 00 00", "09 00 00 00"},
    {"LG",      "Vol +",  "NEC",       "04 00 00 00", "02 00 00 00"},
    {"LG",      "Vol -",  "NEC",       "04 00 00 00", "03 00 00 00"},
    {"LG",      "CH +",   "NEC",       "04 00 00 00", "00 00 00 00"},
    {"LG",      "CH -",   "NEC",       "04 00 00 00", "01 00 00 00"},
    {"Sony",    "Power",  "SIRC",      "01 00 00 00", "15 00 00 00"},
};

static const int MAGIC_IR_SHORTCUT_COUNT = sizeof(MAGIC_IR_SHORTCUTS) / sizeof(MAGIC_IR_SHORTCUTS[0]);

#endif
