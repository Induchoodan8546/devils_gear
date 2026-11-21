# Devil’s Gear SD layout

Copy this `magic` folder to the root of a FAT32 microSD card. You can also copy `sd_files/boot.jpg` to `/boot.jpg` on the card.

```
/boot.jpg               optional splash (also /magic/theme/boot.jpg)
/magic/triggers/        named Sub-GHz .sub files (imported on-device)
/magic/ir/              extra .ir shortcuts
/magic/theme/boot.jpg   optional splash override
```

Indexes (`triggers/index.json`, `ir/index.json`, `cards.json`, `predictions.json`) are created on LittleFS at first boot. Import through the menus so the indexes stay in sync.
