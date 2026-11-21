# Devil’s Gear firmware

Flash-ready image for **LilyGO T-Embed CC1101**.

| File | What it is | Size |
|---|---|---|
| `DevilsGear-lilygo-t-embed-cc1101.bin` | Merged image (bootloader + partitions + app). Flash at **0x0**. | ~4.6 MB |
| `DevilsGear-app.bin` | Application only. Flash at **0x10000** if you already have a matching bootloader. | ~4.4 MB |

## Flash (esptool)

Hold **BOOT**, tap **RESET**, release **BOOT**, then:

```
esptool.py --chip esp32s3 --baud 921600 write_flash 0x0 firmware/DevilsGear-lilygo-t-embed-cc1101.bin
```

Or from this repo with PlatformIO:

```
pio run -e lilygo-t-embed-cc1101 -t upload
```

## Build from source

```
python -m platformio run -e lilygo-t-embed-cc1101
```

The merged image is also written to the repo root as `Bruce-lilygo-t-embed-cc1101.bin` by Bruce’s merge script; copy it here after each release build.

See [USAGE.md](../USAGE.md) for the magician menus.
