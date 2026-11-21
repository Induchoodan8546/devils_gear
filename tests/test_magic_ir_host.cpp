// Host compile check for the IR shortcut table. No Arduino.
#include <cstdio>
#include <cstring>
#include "../src/modules/magic/magic_ir_codes.h"

int main() {
    if (MAGIC_IR_SHORTCUT_COUNT != 13) {
        std::printf("FAIL count %d\n", MAGIC_IR_SHORTCUT_COUNT);
        return 1;
    }
    int samsung = 0, lg = 0, sony = 0;
    for (int i = 0; i < MAGIC_IR_SHORTCUT_COUNT; i++) {
        const MagicIrShortcut &s = MAGIC_IR_SHORTCUTS[i];
        if (!s.brand || !s.name || !s.protocol || !s.address || !s.command) return 2;
        if (std::strcmp(s.brand, "Samsung") == 0) samsung++;
        if (std::strcmp(s.brand, "LG") == 0) lg++;
        if (std::strcmp(s.brand, "Sony") == 0) sony++;
        if (std::strcmp(s.brand, "Samsung") == 0 && std::strcmp(s.name, "Power") == 0) {
            if (std::strcmp(s.protocol, "Samsung32") != 0) return 3;
            if (std::strcmp(s.command, "02 00 00 00") != 0) return 4;
        }
    }
    if (samsung != 6 || lg != 6 || sony != 1) {
        std::printf("FAIL brands s=%d lg=%d sony=%d\n", samsung, lg, sony);
        return 5;
    }
    std::printf("PASS IR table: %d shortcuts\n", MAGIC_IR_SHORTCUT_COUNT);
    return 0;
}
