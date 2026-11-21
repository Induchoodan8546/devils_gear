# Devil’s Gear

**EXOTIC MAGICK** — electronic magic / mentalism firmware for the LilyGO T-Embed CC1101.

A specialised fork of [Bruce](https://github.com/BruceDevices/firmware). The radios are unchanged. The interface is a magician’s tool: Sub-GHz prop triggers, NFC card mapping, IR reveals, on-device predictions, and LED/sound feedback.

## Hardware

LilyGO T-Embed CC1101 (ESP32-S3, CC1101, PN532, IR, encoder, RGB LEDs, speaker, SD).

## Flash

Web / esptool:

```
esptool.py --chip esp32s3 write_flash 0x0 firmware/DevilsGear-lilygo-t-embed-cc1101.bin
```

From source (PlatformIO):

```
pio run -e lilygo-t-embed-cc1101
pio run -e lilygo-t-embed-cc1101 -t upload
```

The flash-ready binary is `firmware/DevilsGear-lilygo-t-embed-cc1101.bin` after a successful build.

## Menu

```
DEVIL’S GEAR
├── Prop Triggers      Sub-GHz one-click library
├── Magic Cards        NFC UID → effect
├── IR Reveals         Remote shortcuts + learn
├── Predictions        On-device reveal
├── Effects            LED / sound
└── Settings           Brightness, sound, workshop
```

Read **[USAGE.md](USAGE.md)** for encoder controls, loading triggers, enrolling cards, and predictions.

## License

GNU Affero General Public License v3.0 — same as Bruce. Source must be offered with any distributed binary. This product includes Bruce firmware; see `LICENSE` and `THIRD_PARTY.md`.
