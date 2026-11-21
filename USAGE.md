# Devil’s Gear — Usage

Firmware for the **LilyGO T-Embed CC1101**. Encoder to move, press to select, side button to go back. Hold the encoder to power off.

## First boot

The splash shows **DEVIL’S GEAR / EXOTIC MAGICK**. Press any key to skip it. The main menu is a six-item grid:

1. Prop Triggers
2. Magic Cards
3. IR Reveals
4. Predictions
5. Effects
6. Settings

## Prop Triggers (Sub-GHz)

One-click transmit of saved `.sub` files (bells, haunted objects, remote props).

- **Import .sub** — pick a Flipper/Bruce `.sub` from SD or LittleFS and give it a stage name.
- **Capture RF** — open Bruce Scan/Copy, save the signal, then Import it.
- Select a named trigger to transmit immediately. The LED pulses crimson and a click plays.

Drop files on an SD card at `/magic/triggers/` and add them through Import, or copy a `.sub` in **Settings → Workshop → Files**.

## Magic Cards (NFC)

The T-Embed’s onboard PN532 reads a tag UID and fires a mapped effect.

- **Scan** — present a card. Mapped cards run their effect (prediction, trigger, IR, or text).
- **Enroll** — present a card, name it, choose the effect.
- **Manage** — delete mappings.

Unmapped cards show the UID so you can enroll them.

## IR Reveals

Built-in shortcuts (no SD needed):

- Samsung: Power, Mute, Vol ±, CH ±
- LG: Power, Mute, Vol ±, CH ±
- Sony: Power
- Universal TV Off (TV-B-Gone)

**Import .ir** adds Flipper-format files. **Learn IR** opens the receiver; save, then Import.

## Predictions

Up to 8 prepared lines.

- **Prepare** — name + text (encoder keyboard).
- **Reveal** — fullscreen gold/crimson display, LED gold, chime. Press to close.
- **Peek** — magician-only small text, no LED.
- A Magic Card can be mapped to a prediction by name.

## Effects

LED crimson / gold / rainbow / off, plus the success, transmit, error, and boot patterns.

## Settings

Brightness, dim, orientation, sound, LED, feedback on/off, power, about.

**Workshop** (loading your own material):

- Files (SD / LittleFS / WebUI)
- RF Scan/Copy
- IR Learn
- RFID Read

Wi-Fi attacks, Evil Portal, BLE spam, and jammers are not on the magician menu.

## Flash

Hold **BOOT**, tap **RESET**, release **BOOT**, then:

```
esptool.py --chip esp32s3 --baud 921600 write_flash 0x0 firmware/DevilsGear-lilygo-t-embed-cc1101.bin
```

The file in `firmware/` is a merged image (bootloader + partitions + app), about 4.6 MB. Flash it at address `0x0`.

Or with PlatformIO from this repo:

```
pio run -e lilygo-t-embed-cc1101 -t upload
```

Erase first if you are leaving factory firmware.

## Files on SD (optional)

```
/magic/triggers/     .sub files
/magic/ir/           .ir files
/magic/theme/boot.jpg
```

The device also stores maps in LittleFS, so it works with no card.

Based on [Bruce](https://github.com/BruceDevices/firmware), AGPL-3.0.
