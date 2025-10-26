# IthacaPlayer Decorators

This directory contains visual assets for IthacaPlayer:

## Contents

### Background Image
**background.jpg** - GUI background image displayed in plugin window

### Application Icon
**icon.ico** - Windows application icon (multi-size: 16×16 to 256×256)
**icon.icns** - macOS application icon

**Current Icon:** White square with black letter "I" (placeholder)

## Replacing the Icon

To use a custom icon:

1. **Prepare your design:**
   - Create a square image (1024×1024 recommended)
   - Use your logo, instrument image, or custom design
   - Ensure good visibility at small sizes (16×16)

2. **Convert to required formats:**
   - **For Windows (.ico):**
     - Online: https://convertio.co/png-ico/
     - Select multi-size output (16, 32, 48, 256)

   - **For macOS (.icns):**
     - Online: https://cloudconvert.com/png-to-icns
     - Or use macOS `iconutil` command

3. **Replace files:**
   ```bash
   # Replace existing icon files
   cp your_icon.ico decorators/icon.ico
   cp your_icon.icns decorators/icon.icns
   ```

4. **Rebuild project:**
   ```bash
   cmake --build build --config Debug
   ```

The new icon will be embedded into:
- `IthacaPlayer.exe` (Windows Standalone)
- `IthacaPlayer.vst3` (VST3 Plugin)
- `IthacaPlayer.component` (macOS AU)

## Icon Specifications

### Windows (.ico)
- Format: ICO multi-size
- Recommended sizes: 16×16, 32×32, 48×48, 64×64, 128×128, 256×256
- Color depth: 32-bit RGBA (with transparency)

### macOS (.icns)
- Format: ICNS (Apple Icon Image)
- Sizes: 16×16 to 1024×1024 (HiDPI @2x variants)
- Color depth: 32-bit RGBA

## Helper Files

Individual PNG exports are available for reference:
- `icon_16x16.png` through `icon_256x256.png`
- `icon_1024x1024.png` - High-res source for conversions

These PNG files are NOT used by the application - they're just references.
Only `icon.ico` and `icon.icns` are embedded into the executable.

## Notes

- Icon changes require project rebuild to take effect
- Windows taskbar and Alt+Tab will show the embedded icon
- macOS Dock and Finder will use the .icns icon
- Plugin hosts (DAWs) may cache icons - restart host after rebuild
