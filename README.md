# Ithaca Player - JUCE Audio Plugin

Professional sample-based virtual instrument plugin built with JUCE framework and IthacaCore audio engine.

## Project Overview

Ithaca Player is a VST3/AU/Standalone plugin that combines:
- **JUCE Framework** - Cross-platform audio plugin framework
- **IthacaCore** - Custom multiplatform sampler engine (128 voices, velocity layers, ADSR envelopes, LFO panning)
- **GUI Sample Bank Selector** - Runtime sample bank loading via folder picker
- **MIDI Learn** - Dynamic CC mapping for all parameters
- **Multiple Instrument Instances** - Each instrument (VintageV, Rhodes, Wurlitzer, Piano) as separate plugin

## Multiple Instrument Instances Architecture

Each instrument is built as an **independent plugin** with its own:
- Plugin binary (e.g., `IthacaPlayer-VintageV.exe`)
- VST3/AU registration (appears separately in DAW)
- User data directory (config, decorators, logs)
- Sample bank configuration

### Available Instruments

| Instrument | Plugin Code | Binary Name | Description |
|------------|-------------|-------------|-------------|
| VintageV   | VntV        | IthacaPlayer-VintageV | Vintage Electric Piano |
| Rhodes     | Rhds        | IthacaPlayer-Rhodes | Rhodes Electric Piano |
| Wurlitzer  | Wrlz        | IthacaPlayer-Wurlitzer | Wurlitzer Electric Piano |
| Piano      | Pian        | IthacaPlayer-Piano | Acoustic Piano |

### Benefits

- **Multiple instruments in one DAW session** - Load VintageV and Rhodes simultaneously
- **Separate presets** - Each instrument has its own saved state
- **Independent sample banks** - Different sample paths per instrument
- **No interference** - Instruments run completely isolated

## Requirements

- **Visual Studio 2022** (Community or Build Tools) with C++ components
- **CMake 3.22+** (in PATH)
- **Git** with submodules support
- **C++17** compatible compiler (MSVC/GCC/Clang)
- **libsndfile** (included as submodule)
- **JUCE** (included as submodule)
- **nlohmann/json** (included as submodule)

## Quick Start

### 1. Clone Repository with Submodules
```bash
git clone --recursive https://github.com/alchy/Ithaca.git
cd Ithaca

# If already cloned without --recursive:
git submodule update --init --recursive
```

### 2. Build an Instrument

**IMPORTANT: CMake Location**
If you have Visual Studio Build Tools installed, CMake is typically located at:
```
C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe
```

You can find the exact path used for your build in `build/CMakeCache.txt` (line 3)

**Example: Build VintageV**
```bash
# Configure with instrument name (if cmake is in PATH)
cmake -B build -S . -G "Visual Studio 17 2022" -A x64 -DINSTRUMENT_NAME=VintageV

# Build Release
cmake --build build --config Release --target IthacaPlayer-VintageV -j 4

# OR if cmake is not in PATH, use full path:
"C:/Program Files (x86)/Microsoft Visual Studio/2022/BuildTools/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe" --build build --config Debug --target IthacaPlayer-VintageV -j 4
```

**Example: Build Rhodes**
```bash
# Clean previous build (optional)
rm -rf build

# Configure for Rhodes (if cmake is in PATH)
cmake -B build -S . -G "Visual Studio 17 2022" -A x64 -DINSTRUMENT_NAME=Rhodes

# Build Release
cmake --build build --config Release --target IthacaPlayer-Rhodes -j 4

# OR if cmake is not in PATH:
"C:/Program Files (x86)/Microsoft Visual Studio/2022/BuildTools/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe" --build build --config Release --target IthacaPlayer-Rhodes -j 4
```

### Rebuilding After Code Changes

If you've modified source code and want to rebuild without reconfiguring:

```bash
# Debug build (faster, includes debug symbols)
"C:/Program Files (x86)/Microsoft Visual Studio/2022/BuildTools/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe" --build build --config Debug --target IthacaPlayer-VintageV -j 4

# Release build (optimized)
"C:/Program Files (x86)/Microsoft Visual Studio/2022/BuildTools/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe" --build build --config Release --target IthacaPlayer-VintageV -j 4
```

**Note:** No need to run `cmake -B build` again unless you changed CMakeLists.txt or added new files.

### 3. Run Standalone Application
```bash
# VintageV
./build/IthacaPlayer-VintageV_artefacts/Release/Standalone/IthacaPlayer-VintageV.exe

# Rhodes
./build/IthacaPlayer-Rhodes_artefacts/Release/Standalone/IthacaPlayer-Rhodes.exe
```

## User Data Structure

Each instrument instance stores its configuration in a platform-specific roaming directory:

**Windows:**
```
C:\Users\<user>\AppData\Roaming\LordAudio\
├── IthacaPlayer-VintageV\
│   ├── decorators\
│   │   └── background.jpg             # GUI background image
│   └── core_logger\
│       └── core_logger.log            # Runtime diagnostics log
├── IthacaPlayer-Rhodes\
│   ├── decorators\
│   │   └── background.jpg
│   └── core_logger\
│       └── core_logger.log
└── ...
```

**macOS:**
```
~/Library/Application Support/LordAudio/
├── IthacaPlayer-VintageV/
│   ├── decorators/background.jpg
│   └── core_logger/core_logger.log
└── ...
```

**Linux:**
```
~/.local/share/LordAudio/
├── IthacaPlayer-VintageV/
│   ├── decorators/background.jpg
│   └── core_logger/core_logger.log
└── ...
```

### User Data Files

1. **decorators/background.jpg** - GUI background (480x650px), installed during build

2. **core_logger/core_logger.log** - Diagnostic log with startup, loading, and error info

## Sample Bank Loading

### GUI-Based Loading Workflow

The plugin uses **runtime sample bank loading via GUI** instead of configuration files:

1. **Startup**: Plugin initializes with **sine wave test tones** (immediate playback, no files needed)
2. **User Selection**: Click **"Load Sample Bank..."** button in GUI
3. **Folder Picker**: Browse to sample bank directory (e.g., `C:/SoundBanks/IthacaPlayer/VntV`)
4. **Async Loading**: Background thread loads samples without blocking audio/GUI
5. **Automatic Swap**: When complete, plugin seamlessly replaces sine waves with real samples

### Benefits

- **No configuration files** - Sample bank path managed entirely through GUI
- **Instant startup** - Always playable with sine waves
- **Runtime switching** - Load different sample banks without restarting
- **Per-instrument independence** - Each plugin instance remembers its own sample bank

### Sample Bank Structure

For detailed sample bank structure requirements, see [SAMPLEPATHS.md](SAMPLEPATHS.md).

## Project Structure

```
Ithaca/
├── ithaca/                          # JUCE plugin code
│   ├── audio/                       # Audio processing & sample loading
│   │   ├── IthacaPluginProcessor.*  # Main audio processor
│   │   ├── AsyncSampleLoader.*      # Background sample loading
│   │   ├── SampleBankPathManager.*  # JSON config management
│   │   ├── PerformanceMonitor.*     # CPU usage tracking
│   │   └── PluginStateManager.*     # Save/load state
│   ├── gui/                         # User interface
│   │   ├── IthacaPluginEditor.*     # Main editor window
│   │   └── components/              # UI components (sliders, sample bank selector)
│   ├── midi/                        # MIDI processing
│   │   ├── MidiProcessor.*          # MIDI message handling
│   │   └── MidiLearnManager.*       # Dynamic CC mapping
│   ├── parameters/                  # Parameter management
│   │   └── ParameterManager.*       # APVTS integration
│   └── config/                      # Configuration headers
│       ├── IthacaConfig.h           # JUCE plugin configuration
│       └── AppConstants.h           # GUI/MIDI constants
├── ithaca-core/                     # Core audio engine (framework-agnostic)
│   ├── sampler/                     # Sampler implementation
│   │   ├── voice_manager.*          # Voice management (128 voices)
│   │   ├── voice.*                  # Single voice processing
│   │   ├── envelopes/               # ADSR/ASR envelopes
│   │   ├── instrument_loader.*      # Sample loading
│   │   └── lfopan.*                 # LFO panning effect
│   └── config/
│       └── IthacaConfig.h           # Core audio engine configuration
├── cmake/                           # CMake installation scripts
│   └── InstallDecorators.cmake      # Install GUI assets
├── decorators/                      # GUI assets (source)
│   └── background.jpg
├── json/                            # nlohmann/json library (submodule)
├── JUCE/                            # JUCE framework (submodule)
├── libsndfile/                      # Audio file I/O (submodule)
├── CMakeLists.txt                   # Build configuration
├── SAMPLEPATHS.md                   # Sample bank structure documentation
└── README.md                        # This file
```

## Features

### Audio Engine (IthacaCore)
- **128 polyphonic voices** with velocity sensitivity
- **ADSR/ASR envelopes** with RT-safe static data
- **LFO panning** with configurable speed/depth
- **Stereo field** control
- **Master gain/pan** with MIDI CC support
- **Sustain pedal** support
- **Velocity layers** support via JSON metadata

### GUI Features
- **Modern interface** with rounded overlays and custom background
- **Sample bank selector** - Runtime sample bank loading via folder picker
- **8 parameter sliders**: Master Gain, Master Pan, Attack, Release, Sustain, LFO Speed, LFO Depth, Stereo Field
- **Real-time statistics**: Active voices, sample rate, CPU usage, velocity layers
- **MIDI Learn**: Right-click any slider to assign MIDI CC
- **Performance monitor**: Dropout detection and CPU usage
- **Instrument name display** with velocity layer count

### MIDI Implementation
- **Note On/Off** - Voice triggering with velocity
- **Sustain Pedal** (CC 64) - Hold notes
- **All Notes Off** (CC 123) - Emergency stop
- **Dynamic CC mapping** via MIDI Learn
- **State persistence** - Mappings saved with project

## Build Targets

```bash
# Build specific instrument (VintageV example)
cmake --build build --config Release --target IthacaPlayer-VintageV

# Build all targets
cmake --build build --config Release

# Clean logs (removes core_logger directory)
cmake --build build --target clean-logs

# Clean exports (removes test exports)
cmake --build build --target clean-exports

# Clean all IthacaCore data
cmake --build build --target clean-all-ithaca
```

## Output Files

### Plugin Formats (VintageV example)

**Release Build:**
- **VST3**: `build/IthacaPlayer-VintageV_artefacts/Release/VST3/IthacaPlayer-VintageV.vst3/`
- **Standalone**: `build/IthacaPlayer-VintageV_artefacts/Release/Standalone/IthacaPlayer-VintageV.exe`
- **AU** (macOS): `build/IthacaPlayer-VintageV_artefacts/Release/AU/IthacaPlayer-VintageV.component`

**Debug Build:**
- **VST3**: `build/IthacaPlayer-VintageV_artefacts/Debug/VST3/IthacaPlayer-VintageV.vst3/`
- **Standalone**: `build/IthacaPlayer-VintageV_artefacts/Debug/Standalone/IthacaPlayer-VintageV.exe`

### Installed Files (POST_BUILD)

CMake automatically installs these files to user roaming directory:

1. **Decorators**: `%APPDATA%\LordAudio\IthacaPlayer-VintageV\decorators\background.jpg`
   - Always overwrites to update with new builds

2. **Core logger directory**: `%APPDATA%\LordAudio\IthacaPlayer-VintageV\core_logger\`
   - Created automatically on first run

### Logs

Runtime logs are written to user roaming directory:
```
%APPDATA%\LordAudio\IthacaPlayer-VintageV\core_logger\core_logger.log
```

Log includes:
- Plugin initialization (sine wave startup)
- Sample loading progress (GUI-triggered)
- Decorators path resolution
- VoiceManager swap operations
- MIDI events (if verbose logging enabled)
- Performance warnings (dropouts, CPU usage)
- Error diagnostics

## Development

### Building Multiple Instruments

To build all instruments sequentially:

```bash
# VintageV
cmake -B build-vintagev -S . -G "Visual Studio 17 2022" -A x64 -DINSTRUMENT_NAME=VintageV
cmake --build build-vintagev --config Release --target IthacaPlayer-VintageV

# Rhodes
cmake -B build-rhodes -S . -G "Visual Studio 17 2022" -A x64 -DINSTRUMENT_NAME=Rhodes
cmake --build build-rhodes --config Release --target IthacaPlayer-Rhodes

# Wurlitzer
cmake -B build-wurlitzer -S . -G "Visual Studio 17 2022" -A x64 -DINSTRUMENT_NAME=Wurlitzer
cmake --build build-wurlitzer --config Release --target IthacaPlayer-Wurlitzer

# Piano
cmake -B build-piano -S . -G "Visual Studio 17 2022" -A x64 -DINSTRUMENT_NAME=Piano
cmake --build build-piano --config Release --target IthacaPlayer-Piano
```

### Watching Logs (PowerShell)

```powershell
# Tail log file for VintageV
Get-Content -Path "$env:APPDATA\LordAudio\IthacaPlayer-VintageV\core_logger\core_logger.log" -Tail 20 -Wait

# Filter by component
Get-Content -Path "$env:APPDATA\LordAudio\IthacaPlayer-VintageV\core_logger\core_logger.log" -Tail 20 -Wait | Where-Object { $_ -like "*VoiceManager*" }

# Filter by severity
Get-Content -Path "$env:APPDATA\LordAudio\IthacaPlayer-VintageV\core_logger\core_logger.log" -Tail 20 -Wait | Where-Object { $_ -like "*ERROR*" }
```

### CMake Configuration Options

```bash
# Specify instrument (required)
cmake -B build -DINSTRUMENT_NAME=VintageV

# Custom sample bank path
cmake -B build -DINSTRUMENT_NAME=VintageV -DSAMPLE_BANK_PATH="D:/Samples/VntV"

# Release build
cmake -B build -DINSTRUMENT_NAME=VintageV -DCMAKE_BUILD_TYPE=Release

# Specify generator
cmake -B build -DINSTRUMENT_NAME=VintageV -G "Visual Studio 17 2022" -A x64

# Multiple options
cmake -B build -S . -G "Visual Studio 17 2022" -A x64 \
  -DINSTRUMENT_NAME=VintageV \
  -DSAMPLE_BANK_PATH="D:/Samples/VntV" \
  -DCMAKE_BUILD_TYPE=Release
```

### Debug Mode

IthacaCore supports verbose logging via `IthacaConfig.h`:

```cpp
// ithaca-core/IthacaConfig.h
#define ITHACA_DEBUG_LOGGING 1  // Enable verbose logging
```

JUCE plugin has additional GUI debug mode:

```cpp
// ithaca/gui/IthacaPluginEditor.cpp
#define BACKGROUND_PICTURE_OFF 1  // Disable background, show debug overlay
```

## Architecture

### Multiple Instance Isolation

Each instrument instance is **completely isolated**:

```
Plugin Registration:
- VintageV → Plugin Code: VntV → IthacaPlayer-VintageV.vst3
- Rhodes   → Plugin Code: Rhds → IthacaPlayer-Rhodes.vst3

User Data:
- VintageV → %APPDATA%\LordAudio\IthacaPlayer-VintageV\
- Rhodes   → %APPDATA%\LordAudio\IthacaPlayer-Rhodes\

Sample Banks:
- VintageV → C:/SoundBanks/IthacaPlayer/VntV/
- Rhodes   → C:/SoundBanks/IthacaPlayer/Rhds/
```

**No shared state between instruments** - Each runs in its own plugin instance with separate:
- Audio processing thread
- Voice manager (128 voices each)
- MIDI processor
- Parameter state
- MIDI Learn mappings

### Audio Pipeline
```
MIDI Input → MidiProcessor → VoiceManager (128 voices) →
EnvelopeStaticData (RT-safe ADSR) → Voice Processing →
LFO Panning → Master Gain/Pan → Audio Output
```

### Sample Loading Workflow
1. **Startup**: Initialize with sine wave test tones (immediate, no files needed)
2. **GUI Selection**: User clicks "Load Sample Bank..." button, selects directory
3. **AsyncSampleLoader**: Background thread loads samples without blocking audio/GUI
4. **InstrumentLoader**: Parse metadata, allocate stereo buffers, read velocity layers
5. **VoiceManager Creation**: New VoiceManager with 128 voices and sample references
6. **RT-Safe Swap**: In processBlock(), atomically replace sine waves with loaded samples

### Parameter Flow
```
GUI Slider → ParameterAttachment → AudioProcessorValueTreeState →
IthacaPluginProcessor → VoiceManager → Voice → Envelope (RT-safe update)
```

### Logger Flow
```
IthacaPluginProcessor → Logger → core_logger.log
IthacaPluginEditor → Logger → core_logger.log (decorators path diagnostics)
VoiceManager → Logger → core_logger.log (voice state, MIDI events)
```

## MIDI CC Mappings (Default)

| Parameter | CC Number | Range | Description |
|-----------|-----------|-------|-------------|
| Master Gain | 7 | 0-127 | Volume control |
| Master Pan | 10 | 0-127 | Stereo balance (64=center) |
| Attack | 73 | 0-127 | Envelope attack time |
| Release | 72 | 0-127 | Envelope release time |
| Sustain Level | 71 | 0-127 | Sustain level |
| LFO Speed | 74 | 0-127 | LFO modulation rate |
| LFO Depth | 75 | 0-127 | LFO modulation depth |
| Stereo Field | 76 | 0-127 | Stereo width |

**Note**: All mappings can be customized via MIDI Learn (right-click slider).

## Performance

### CPU Optimization
- **RT-safe envelopes**: Pre-computed ADSR curves (no math in audio thread)
- **Voice pooling**: Pre-allocated 128 voices per instrument instance
- **SIMD potential**: Prepared for vectorization (currently scalar)
- **Lock-free**: Atomic parameters for GUI↔audio communication
- **Async loading**: Non-blocking sample loading in background thread

### Memory Usage
- **Sample buffers**: Loaded once per instrument, shared across voices
- **Envelope data**: Static tables (44.1kHz/48kHz)
- **Voice state**: Minimal per-voice overhead
- **Multiple instances**: Each instrument has separate memory (no shared samples between VintageV/Rhodes/etc.)

## Troubleshooting

### Sample Loading Fails

1. **Plugin starts with sine waves:**
   - This is normal behavior - plugin initializes immediately with test tones
   - Use "Load Sample Bank..." button to load real samples

2. **Sample bank not loading after selection:**
   - Check log for diagnostics:
     ```bash
     tail -20 "$env:APPDATA\LordAudio\IthacaPlayer-VintageV\core_logger\core_logger.log"
     ```
   - Look for AsyncSampleLoader errors or InstrumentLoader failures

3. **Verify sample bank structure:**
   - See [SAMPLEPATHS.md](SAMPLEPATHS.md) for required directory structure
   - Ensure `instrument-definition.json` exists in selected directory
   - Check sample files are WAV format
   - Verify directory is accessible (not on network drive with permission issues)

4. **Sample bank loads but still uses sine waves:**
   - Check log for VoiceManager swap messages
   - Verify AsyncSampleLoader completed successfully
   - Try restarting plugin after loading

### Background Image Not Loading

1. **Check decorators directory:**
   ```bash
   ls "$env:APPDATA\LordAudio\IthacaPlayer-VintageV\decorators\background.jpg"
   ```

2. **Verify installation:**
   - CMake POST_BUILD should install `decorators/background.jpg`
   - If missing, run build again to trigger installation

3. **Check log:**
   - Log shows decorators path resolution attempts
   - Search for `IthacaPluginEditor/setupBackground` in log

### Build Errors

1. **Update submodules:**
   ```bash
   git submodule update --init --recursive
   ```

2. **Clean build folder:**
   ```bash
   rm -rf build
   cmake -B build -S . -G "Visual Studio 17 2022" -A x64 -DINSTRUMENT_NAME=VintageV
   ```

3. **Check MSVC installation:**
   ```cmd
   "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64
   ```

4. **Verify CMake version:**
   ```bash
   cmake --version  # Should be >= 3.22
   ```

5. **Missing INSTRUMENT_NAME:**
   ```
   CMake Error: INSTRUMENT_NAME must be defined
   ```
   **Solution:** Add `-DINSTRUMENT_NAME=VintageV` to cmake command

### Plugin Not Found in DAW

1. **Check VST3 installation path:**
   ```bash
   # Should be installed at:
   C:\Program Files\Common Files\VST3\IthacaPlayer-VintageV.vst3
   ```

2. **Copy manually from build directory:**
   ```bash
   cp -r build/IthacaPlayer-VintageV_artefacts/Release/VST3/IthacaPlayer-VintageV.vst3 \
     "C:\Program Files\Common Files\VST3\"
   ```

3. **Rescan plugins in DAW:**
   - Most DAWs have "Rescan plugins" option
   - Some require restart after adding new plugins

4. **Check DAW plugin folder settings:**
   - Verify DAW is scanning `C:\Program Files\Common Files\VST3\`
   - Check DAW plugin blacklist

### Wrong Instrument Loads

If you built multiple instruments but the wrong one loads:

1. **Each instrument has unique plugin code:**
   - VintageV → VntV
   - Rhodes → Rhds
   - Check DAW plugin list for correct name

2. **Check user data directory:**
   - Ensure each instrument has separate directory
   - Verify config files don't point to wrong sample bank

## Contributing

### Code Style
- **C++17** standard
- **JUCE coding conventions** for plugin code (`ithaca/`)
- **Modern C++** for IthacaCore (`ithaca-core/`) - avoid raw pointers, use smart pointers
- **RT-safe** audio thread code (no allocations, no locks in `processBlock()`)

### Git Workflow
```bash
# Create feature branch
git checkout -b feature/my-feature

# Make changes
git add .
git commit -m "Description of changes"

# Push to remote
git push -u origin feature/my-feature
```

### Adding New Instruments

To add a new instrument (e.g., "Clavinet"):

1. **Edit CMakeLists.txt** (around line 30):
   ```cmake
   elseif(INSTRUMENT_NAME STREQUAL "Clavinet")
       set(PLUGIN_CODE "Clav")
       set(PLUGIN_DESCRIPTION "Clavinet Electric Piano")
   ```

2. **Build:**
   ```bash
   cmake -B build -S . -DINSTRUMENT_NAME=Clavinet
   cmake --build build --config Release --target IthacaPlayer-Clavinet
   ```

3. **Prepare sample bank:**
   - Create `C:/SoundBanks/IthacaPlayer/Clav/`
   - Add samples and `instrument-definition.json`
   - See [SAMPLEPATHS.md](SAMPLEPATHS.md) for structure

## License

[Add license information here]

## Credits

- **JUCE Framework**: [juce.com](https://juce.com)
- **libsndfile**: [libsndfile.github.io](http://libsndfile.github.io/libsndfile/)
- **nlohmann/json**: [github.com/nlohmann/json](https://github.com/nlohmann/json)

## Documentation

- **[SAMPLEPATHS.md](SAMPLEPATHS.md)** - Complete sample bank path system documentation
- **[CMakeLists.txt](CMakeLists.txt)** - Build system configuration with instrument mapping

## Support

For issues and feature requests, please use [GitHub Issues](https://github.com/alchy/Ithaca/issues).

---

**Version**: 1.1.0
**Build System**: CMake 3.22+
**Architecture**: Multiple Instrument Instances (per-instrument isolation)
**Tested On**: Windows 10/11, Visual Studio 2022
