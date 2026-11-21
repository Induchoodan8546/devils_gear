#!/usr/bin/env python3
"""Host tests for Devil's Gear firmware + magician UI (no hardware)."""

from __future__ import annotations

import json
import os
import re
import struct
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "src"
FIRMWARE_DIR = ROOT / "firmware"
MERGED_BIN = FIRMWARE_DIR / "DevilsGear-lilygo-t-embed-cc1101.bin"
APP_BIN = FIRMWARE_DIR / "DevilsGear-app.bin"
ELF = ROOT / ".pio" / "build" / "lilygo-t-embed-cc1101" / "firmware.elf"
NM = Path(r"D:\pio\packages\toolchain-xtensa-esp-elf\bin\xtensa-esp-elf-nm.exe")

PASS = 0
FAIL = 0
errors: list[str] = []


def check(name: str, cond: bool, detail: str = "") -> None:
    global PASS, FAIL
    if cond:
        PASS += 1
        print(f"  PASS  {name}")
    else:
        FAIL += 1
        msg = f"  FAIL  {name}" + (f" — {detail}" if detail else "")
        print(msg)
        errors.append(msg)


def read(path: Path) -> str:
    return path.read_text(encoding="utf-8", errors="replace")


def ascii_strings(data: bytes, min_len: int = 6) -> set[str]:
    out: set[str] = set()
    cur = bytearray()
    for b in data:
        if 32 <= b < 127:
            cur.append(b)
        else:
            if len(cur) >= min_len:
                out.add(cur.decode("ascii"))
            cur.clear()
    if len(cur) >= min_len:
        out.add(cur.decode("ascii"))
    return out


# --- product logic (mirrors src/modules/magic) --------------------------------

def magic_normalize_uid(uid: str) -> str:
    return uid.upper().replace(" ", "").replace(":", "").replace("-", "")


def magic_sanitize_filename(name: str) -> str:
    out = []
    for c in name:
        if c.isalnum() or c in "-_":
            out.append(c)
        elif c == " ":
            out.append("_")
    return "".join(out) or "item"


def parse_ir_shortcuts(text: str) -> list[dict]:
    rows = []
    for m in re.finditer(
        r'\{\s*"([^"]+)"\s*,\s*"([^"]+)"\s*,\s*"([^"]+)"\s*,\s*"([^"]+)"\s*,\s*"([^"]+)"\s*\}',
        text,
    ):
        rows.append(
            {
                "brand": m.group(1),
                "name": m.group(2),
                "protocol": m.group(3),
                "address": m.group(4),
                "command": m.group(5),
            }
        )
    return rows


def magic_fire_ir(label: str, shortcuts: list[dict], customs: list[dict]) -> str | None:
    if label == "Universal TV Off":
        return "tvb-gone"
    for s in shortcuts:
        built = f"{s['brand']} {s['name']}"
        if built == label or s["name"] == label:
            return f"ir:{s['brand']}:{s['name']}:{s['protocol']}"
    for c in customs:
        if c["name"] == label:
            return f"custom:{c['name']}"
    return None


def magic_dispatch(card: dict | None, preds: list[dict], triggers: list[str]) -> str:
    if not card:
        return "unmapped"
    t = card["type"]
    payload = card.get("payload", "")
    if t == "prediction":
        for p in preds:
            if p["name"] == payload or str(p.get("id", "")) == payload:
                return f"reveal:{p['text']}"
        return "prediction-missing"
    if t == "trigger":
        return "trigger" if payload in triggers else "trigger-missing"
    if t == "ir":
        return "ir"
    return f"text:{payload or card.get('label', '')}"


def parse_flipper_ir(path: Path) -> dict[str, tuple[str, str, str]]:
    blocks: dict[str, tuple[str, str, str]] = {}
    name = proto = addr = cmd = None
    for line in path.read_text(encoding="utf-8", errors="replace").splitlines():
        line = line.strip()
        if line.startswith("name:"):
            if name and proto and addr and cmd:
                blocks[name] = (proto, addr, cmd)
            name = line.split(":", 1)[1].strip()
            proto = addr = cmd = None
        elif line.startswith("protocol:"):
            proto = line.split(":", 1)[1].strip()
        elif line.startswith("address:"):
            addr = line.split(":", 1)[1].strip()
        elif line.startswith("command:"):
            cmd = line.split(":", 1)[1].strip()
    if name and proto and addr and cmd:
        blocks[name] = (proto, addr, cmd)
    return blocks


# --- tests --------------------------------------------------------------------

def test_firmware_images() -> None:
    print("\n[firmware images]")
    check("merged bin exists", MERGED_BIN.is_file(), str(MERGED_BIN))
    check("app bin exists", APP_BIN.is_file(), str(APP_BIN))
    if not MERGED_BIN.is_file() or not APP_BIN.is_file():
        return
    merged = MERGED_BIN.read_bytes()
    app = APP_BIN.read_bytes()
    check("merged size > 1MB", len(merged) > 1_000_000, str(len(merged)))
    check("app size > 1MB", len(app) > 1_000_000, str(len(app)))
    check("merged starts with ESP image magic 0xE9", merged[0] == 0xE9, hex(merged[0]))
    check("app starts with ESP image magic 0xE9", app[0] == 0xE9, hex(app[0]))
    check("merged is larger than app (bootloader+partitions+app)", len(merged) > len(app))
    # App image flash size nibble in byte 3: 0x2 = 8MB, 0x3 = 16MB typically in high nibble
    check("app image has entry point", struct.unpack_from("<I", app, 4)[0] != 0)


def test_firmware_strings() -> None:
    print("\n[firmware UI strings]")
    if not APP_BIN.is_file():
        check("app bin for strings", False)
        return
    blob = APP_BIN.read_bytes()
    required = [
        "DEVIL'S GEAR",
        "EXOTIC MAGICK",
        "Prop Triggers",
        "Magic Cards",
        "IR Reveals",
        "Predictions",
        "Effects",
        "Settings",
        "Workshop",
        "Samsung",
        "Universal TV Off",
        "Prepare",
        "Reveal",
        "Peek",
        "Enroll",
        "Import .sub",
        "LED Crimson",
        "LED Gold",
        "Feedback: ",
    ]
    for needle in required:
        check(f'string "{needle}" in firmware', needle.encode("ascii") in blob)


def test_elf_symbols() -> None:
    print("\n[firmware symbols]")
    check("firmware.elf exists", ELF.is_file(), str(ELF))
    if not ELF.is_file() or not NM.is_file():
        check("xtensa-nm available", NM.is_file(), str(NM))
        return
    out = subprocess.check_output([str(NM), str(ELF)], text=True, errors="replace")
    needed = [
        "magicBootScreen",
        "magicEnsureDir",  # seed helper; magicEnsureSeed is inlined by LTO
        "magicFeedbackBoot",
        "magicPropTriggersMenu",
        "magicScanCard",  # cards menu body; magicCardsMenu is inlined by LTO
        "magicIrRevealsMenu",
        "magicPredictionsMenu",
        "magicFireTrigger",
        "magicFireIr",
        "magicReadUid",
        "magicShowReveal",
        "magicNormalizeUid",
        "magicLedCrimson",
        "_ZN8MainMenu",
        "_ZN14MagicCardsMenu",
        "_ZN16PropTriggersMenu",
        "_ZN13IRRevealsMenu",
        "_ZN15PredictionsMenu",
        "_ZN11EffectsMenu",
        "_ZN10ConfigMenu",
    ]
    for sym in needed:
        check(f"symbol {sym}", sym in out)


def test_main_menu_source() -> None:
    print("\n[main menu / settings UI]")
    mm = read(SRC / "core" / "main_menu.cpp")
    block = re.search(r"#ifdef DEVILS_GEAR\s*_menuItems = \{([^}]+)\}", mm)
    check("DEVILS_GEAR main menu block exists", bool(block))
    items = re.findall(r"&(\w+)", block.group(1) if block else "")
    check(
        "six magician items",
        items
        == [
            "propTriggersMenu",
            "magicCardsMenu",
            "irRevealsMenu",
            "predictionsMenu",
            "effectsMenu",
            "configMenu",
        ],
        str(items),
    )
    check("Wi-Fi not on magician main menu", "wifiMenu" not in items)
    check("BLE not on magician main menu", "bleMenu" not in items)
    check("RF tools not on magician main menu", "rfMenu" not in items)

    cfg = read(SRC / "core" / "menu_items" / "ConfigMenu.cpp")
    opts = cfg.split("void ConfigMenu::optionsMenu()")[1].split("void ConfigMenu::displayUIMenu")[0]
    dg = opts.split("#ifdef DEVILS_GEAR", 1)[1].split("#else", 1)[0]
    check("Settings has Workshop", "Workshop" in dg)
    check("Settings has Feedback toggle", "Feedback:" in dg)
    check("Settings titled Settings", 'MENU_TYPE_SUBMENU, "Settings"' in cfg)
    check("Evil Portal not in magician Settings", "Evil Portal" not in dg)
    check("Dev Mode not in magician Settings", "Dev Mode" not in dg)
    check("Workshop has RF Scan/Copy", "RF Scan/Copy" in cfg)
    check("Workshop has IR Learn", "IR Learn" in cfg)
    check("Workshop has RFID Read", "RFID Read" in cfg)

    names = {
        "PropTriggersMenu.h": "Prop Triggers",
        "MagicCardsMenu.h": "Magic Cards",
        "IRRevealsMenu.h": "IR Reveals",
        "PredictionsMenu.h": "Predictions",
        "EffectsMenu.h": "Effects",
        "ConfigMenu.h": "Settings",
    }
    for fn, label in names.items():
        text = read(SRC / "core" / "menu_items" / fn)
        check(f'{fn} label "{label}"', f'MenuItemInterface("{label}")' in text)


def test_theme_and_boot() -> None:
    print("\n[theme + boot]")
    theme = read(SRC / "core" / "theme.h")
    check("crimson primary 0xC000", "DEFAULT_PRICOLOR 0xC000" in theme)
    check("gold secondary 0xD5A0", "DEFAULT_SECCOLOR 0xD5A0" in theme)
    cfg = read(SRC / "core" / "config.h")
    check("default LED crimson 0x960000", "ledColor = 0x960000" in cfg)
    check("default grid menu", "MAIN_MENU_GRID" in cfg)
    main = read(SRC / "main.cpp")
    check("boot calls magicBootScreen", "magicBootScreen()" in main)
    check("boot calls magicFeedbackBoot", "magicFeedbackBoot()" in main)
    check("first boot seeds LittleFS", "magicEnsureSeed()" in main)
    disp = read(SRC / "core" / "display.cpp")
    check("status bar DEVIL'S GEAR", 'tft.print("DEVIL\'S GEAR")' in disp)
    ini = read(ROOT / "boards" / "lilygo-t-embed-cc1101" / "lilygo-t-embed-cc1101.ini")
    check("board flag DEVILS_GEAR", "-DDEVILS_GEAR=1" in ini)
    check("board version 1.0.0", "DEVILS_GEAR_VERSION" in ini)


def test_uid_and_filenames() -> None:
    print("\n[UID + filenames]")
    check("colon UID", magic_normalize_uid("04:a1:b2:c3") == "04A1B2C3")
    check("spaced UID", magic_normalize_uid("04 A1 b2 C3") == "04A1B2C3")
    check("dashed UID", magic_normalize_uid("04-a1-b2-c3") == "04A1B2C3")
    check("already compact", magic_normalize_uid("04A1B2C3") == "04A1B2C3")
    check("stage name", magic_sanitize_filename("Haunted Bell") == "Haunted_Bell")
    check("punctuation stripped", magic_sanitize_filename("Bell #1!") == "Bell_1")
    check("empty becomes item", magic_sanitize_filename("@@@") == "item")


def test_ir_library() -> None:
    print("\n[IR shortcuts]")
    header = read(SRC / "modules" / "magic" / "magic_ir_codes.h")
    shortcuts = parse_ir_shortcuts(header)
    check("13 built-in shortcuts", len(shortcuts) == 13, str(len(shortcuts)))
    brands = {s["brand"] for s in shortcuts}
    check("Samsung/LG/Sony present", brands == {"Samsung", "LG", "Sony"}, str(brands))
    protocols = {s["protocol"] for s in shortcuts}
    check("protocols Bruce sendIRCommand understands", protocols <= {"Samsung32", "NEC", "SIRC"})

    samsung = parse_flipper_ir(ROOT / "sd_files" / "infrared" / "TVs" / "Samsung" / "Samsung_AA59.ir")
    lg = parse_flipper_ir(ROOT / "sd_files" / "infrared" / "TVs" / "LG" / "LG.ir")
    sony = parse_flipper_ir(ROOT / "sd_files" / "infrared" / "TVs" / "Sony" / "SonyTV.ir")

    expected = {
        ("Samsung", "Power"): samsung["Power"],
        ("Samsung", "CH +"): samsung["Ch_next"],
        ("Samsung", "CH -"): samsung["Ch_prev"],
        ("Samsung", "Vol +"): samsung["Vol_up"],
        ("Samsung", "Vol -"): samsung["Vol_dn"],
        ("LG", "Power"): lg["Power"],
        ("LG", "Mute"): lg["Mute"],
        ("LG", "Vol +"): lg["Vol_up"],
        ("LG", "Vol -"): lg["Vol_dn"],
        ("LG", "CH +"): lg["Ch_next"],
        ("LG", "CH -"): lg["Ch_prev"],
        ("Sony", "Power"): sony["On_off"],
    }
    by_key = {(s["brand"], s["name"]): s for s in shortcuts}
    for key, (proto, addr, cmd) in expected.items():
        s = by_key[key]
        check(
            f"IR {key[0]} {key[1]} matches library",
            (s["protocol"], s["address"], s["command"]) == (proto, addr, cmd),
            f"{s} vs {(proto, addr, cmd)}",
        )

    check("Samsung Power lookup", magic_fire_ir("Samsung Power", shortcuts, []) == "ir:Samsung:Power:Samsung32")
    check("LG Power lookup", magic_fire_ir("LG Power", shortcuts, []) == "ir:LG:Power:NEC")
    check("Sony Power lookup", magic_fire_ir("Sony Power", shortcuts, []) == "ir:Sony:Power:SIRC")
    check("Universal TV Off", magic_fire_ir("Universal TV Off", shortcuts, []) == "tvb-gone")
    check("custom IR", magic_fire_ir("Haunt lamp", shortcuts, [{"name": "Haunt lamp"}]) == "custom:Haunt lamp")
    check("missing IR", magic_fire_ir("nope", shortcuts, []) is None)

    ir_cpp = read(SRC / "modules" / "magic" / "magic_ir.cpp")
    check("IR uses sendIRCommand", "sendIRCommand(&code, true)" in ir_cpp)
    check("IR uses StartTvBGone", "StartTvBGone()" in ir_cpp)


def test_nfc_and_predictions() -> None:
    print("\n[NFC mapper + predictions]")
    preds = [
        {"id": 1, "name": "Ace", "text": "Ace of Spades"},
        {"id": 2, "name": "Seven", "text": "Lucky 7"},
    ]
    cards = [
        {
            "uid": magic_normalize_uid("04:aa:bb:cc"),
            "label": "Force card",
            "type": "prediction",
            "payload": "Ace",
        },
        {
            "uid": magic_normalize_uid("04-11-22-33"),
            "label": "Bell",
            "type": "trigger",
            "payload": "Haunted Bell",
        },
        {
            "uid": "AABBCCDD",
            "label": "TV kill",
            "type": "ir",
            "payload": "Samsung Power",
        },
        {
            "uid": "DEADBEEF",
            "label": "Note",
            "type": "text",
            "payload": "Look behind you",
        },
    ]
    by_uid = {c["uid"]: c for c in cards}
    check(
        "prediction card",
        magic_dispatch(by_uid.get(magic_normalize_uid("04 aa bb cc")), preds, ["Haunted Bell"])
        == "reveal:Ace of Spades",
    )
    check(
        "trigger card",
        magic_dispatch(by_uid.get(magic_normalize_uid("04-11-22-33")), preds, ["Haunted Bell"]) == "trigger",
    )
    check("IR card", magic_dispatch(by_uid.get("AABBCCDD"), preds, []) == "ir")
    check("text card", magic_dispatch(by_uid.get("DEADBEEF"), preds, []) == "text:Look behind you")
    check("unmapped card", magic_dispatch(None, preds, []) == "unmapped")
    check("missing prediction", magic_dispatch({"type": "prediction", "payload": "Nope"}, preds, []) == "prediction-missing")

    nfc = read(SRC / "modules" / "magic" / "magic_nfc.cpp")
    check("scan timeout 12s", "magicReadUid(12)" in nfc)
    check("dispatch types", all(t in nfc for t in ('"prediction"', '"trigger"', '"ir"', '"text"')))
    pred = read(SRC / "modules" / "magic" / "magic_predict.cpp")
    check("max 8 predictions", "MAGIC_MAX_PREDICTIONS" in pred)
    store = read(SRC / "modules" / "magic" / "magic_store.h")
    check("MAGIC_MAX_PREDICTIONS is 8", "#define MAGIC_MAX_PREDICTIONS 8" in store)
    check("peek path exists", "magicRevealMenu(true)" in pred)
    check("reveal path exists", "magicRevealMenu(false)" in pred)


def test_rf_triggers() -> None:
    print("\n[Sub-GHz triggers]")
    rf = read(SRC / "modules" / "magic" / "magic_rf.cpp")
    check("fires via txSubFile", "txSubFile(data, true)" in rf)
    check("reads via readSubFile", "readSubFile(" in rf)
    check("capture uses RFScan", "RFScan()" in rf)
    check("import copies into /magic/triggers", "MAGIC_TRIGGERS_DIR" in rf)
    store = read(SRC / "modules" / "magic" / "magic_store.h")
    check("trigger index path", '"/magic/triggers/index.json"' in store)
    check("cards path", '"/magic/cards.json"' in store)
    check("predictions path", '"/magic/predictions.json"' in store)


def test_feedback_and_effects() -> None:
    print("\n[LED + sound + effects menu]")
    fb = read(SRC / "modules" / "magic" / "magic_feedback.cpp")
    check("boot motif tones", "_tone" in fb and "magicFeedbackBoot" in fb)
    check("transmit pulse", "magicFeedbackTransmit" in fb)
    check("success pulse", "magicFeedbackSuccess" in fb)
    check("error pulse", "magicFeedbackError" in fb)
    check("reveal gold LED", "MAGIC_GOLD" in fb)
    fx = read(SRC / "core" / "menu_items" / "EffectsMenu.cpp")
    for label in [
        "LED Crimson",
        "LED Gold",
        "LED Rainbow",
        "LED Off",
        "Success pattern",
        "Transmit pattern",
        "Error pattern",
        "Boot motif",
    ]:
        check(f'Effects item "{label}"', f'"{label}"' in fx)


def test_seed_json() -> None:
    print("\n[SD / LittleFS seed files]")
    magic_dir = ROOT / "sd_files" / "magic"
    cfg = json.loads((magic_dir / "config.json").read_text(encoding="utf-8"))
    check("config.feedback true", cfg.get("feedback") is True)
    for name in ["cards.json", "predictions.json", "triggers/index.json", "ir/index.json"]:
        data = json.loads((magic_dir / name).read_text(encoding="utf-8"))
        check(f"{name} is empty array", data == [])
    check("theme splash jpg", (magic_dir / "theme" / "boot.jpg").is_file())
    check("root boot.jpg", (ROOT / "sd_files" / "boot.jpg").is_file())
    jpg = (magic_dir / "theme" / "boot.jpg").read_bytes()
    check("boot.jpg is JPEG", jpg[:2] == b"\xff\xd8")


def test_ir_header_compiles() -> None:
    print("\n[IR header compile]")
    gxx = NM.parent / "xtensa-esp-elf-g++.exe"
    src = ROOT / "tests" / "test_magic_ir_host.cpp"
    check("host IR test source exists", src.is_file())
    check("xtensa g++ exists", gxx.is_file(), str(gxx))
    if not gxx.is_file() or not src.is_file():
        return
    r = subprocess.run(
        [str(gxx), "-fsyntax-only", "-std=c++17", str(src)],
        capture_output=True,
        text=True,
    )
    check("magic_ir_codes.h compiles standalone", r.returncode == 0, r.stderr[-400:] if r.stderr else "")


def test_docs() -> None:
    print("\n[docs]")
    usage = read(ROOT / "USAGE.md")
    readme = read(ROOT / "README.md")
    check("USAGE covers Prop Triggers", "Prop Triggers" in usage)
    check("USAGE covers Magic Cards", "Magic Cards" in usage)
    check("USAGE covers flash", "write_flash 0x0" in usage)
    check("README product name", "Devil" in readme and "EXOTIC MAGICK" in readme)


def main() -> int:
    print("Devil's Gear host tests")
    print(f"root: {ROOT}")
    test_firmware_images()
    test_firmware_strings()
    test_elf_symbols()
    test_main_menu_source()
    test_theme_and_boot()
    test_uid_and_filenames()
    test_ir_library()
    test_nfc_and_predictions()
    test_rf_triggers()
    test_feedback_and_effects()
    test_seed_json()
    test_ir_header_compiles()
    test_docs()
    print(f"\n{PASS} passed, {FAIL} failed")
    if errors:
        print("\nFailures:")
        for e in errors:
            print(e)
    return 1 if FAIL else 0


if __name__ == "__main__":
    sys.exit(main())
