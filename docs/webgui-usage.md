# WebGUI Usage — comboRGB & keyColors

Pick a color for any key, any layer, from a browser page. Colors show up live, persist in EEPROM after Save.

**Fastest:** open **https://webgui.212-227-193-242.sslip.io/** in Chrome/Edge/Opera. Nothing to install — it's a static page, everything talks directly to your USB device via WebHID. (Firefox/Safari don't support WebHID.)

**Local instead:**
```bash
cd ~/projects/SofleRGB/webgui
python3 -m http.server 8420   # Windows: python -m http.server 8420
```
Open `http://127.0.0.1:8420/index.html`.

## Using it

1. Flash `comboRGB` or `keyColors`.
2. Connect, pick the keyboard from the device picker.
3. Pick a layer, click keys, pick colors.
4. **Save** — wait for confirmation before unplugging/rebooting (mid-write reboot can corrupt stored colors).
5. **Export** before reflashing (flashing wipes EEPROM) — **Import** to restore.

Optional per-key checkboxes: only show a key's color while NumLock/CapsLock/ScrollLock is active.
