# Ithaca Player - JUCE Audio Plugin

Professional sample-based virtual instrument plugin built with JUCE framework and IthacaCore audio engine.

## Project Overview

Ithaca Player is a VST3/AU/Standalone plugin that combines:
- **JUCE Framework** - Cross-platform audio plugin framework
- **IthacaCore** - Custom multiplatform sampler engine (128 voices, velocity layers, ADSR envelopes, LFO panning)
- **GUI Sample Bank Selector** - Runtime sample bank loading via folder picker
- **MIDI Learn** - Dynamic CC mapping for all parameters
- **DSP Chain** - BBE sonic maximizer, limiter

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

### 2. Configure and Build

```bash
# Configure (if cmake is in PATH)
cmake -B build -S . -G "Visual Studio 17 2022" -A x64

# Build Debug Standalone
cmake --build build --config Debug --target IthacaPlayer_Standalone -j 4

# Build Debug VST3
cmake --build build --config Debug --target IthacaPlayer_VST3 -j 4

# Build Release Standalone
cmake --build build --config Release --target IthacaPlayer_Standalone -j 4

# Build Release VST3
cmake --build build --config Release --target IthacaPlayer_VST3 -j 4

# Build all targets
cmake --build build --config Debug -j 4
```

If cmake is not in PATH, use the full path:
```bash
"C:/Program Files (x86)/Microsoft Visual Studio/2022/BuildTools/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe" --build build --config Debug --target IthacaPlayer_Standalone -j 4
```

**Note:** No need to run `cmake -B build` again after code changes unless you modified CMakeLists.txt or added new files.

### 3. Run Standalone Application

**Debug build:**
```
build\IthacaPlayer_artefacts\Debug\Standalone\IthacaPlayer.exe
```

**Release build:**
```
build\IthacaPlayer_artefacts\Release\Standalone\IthacaPlayer.exe
```

## User Data Structure

The plugin stores its configuration in a platform-specific roaming directory:

**Windows:**
```
C:\Users\<user>\AppData\Roaming\LordAudio\
└── IthacaPlayer\
    ├── decorators\
    │   └── background.jpg             # GUI background image
    └── core_logger\
        └── core_logger.log            # Runtime diagnostics log
```

**macOS:**
```
~/Library/Application Support/LordAudio/
└── IthacaPlayer/
    ├── decorators/background.jpg
    └── core_logger/core_logger.log
```

**Linux:**
```
~/.local/share/LordAudio/
└── IthacaPlayer/
    ├── decorators/background.jpg
    └── core_logger/core_logger.log
```

### User Data Files

1. **decorators/background.jpg** - GUI background (480x650px), installed automatically during build

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
│   ├── dsp/                         # DSP effects chain
│   │   ├── bbe/                     # BBE sonic maximizer
│   │   └── limiter/                 # Limiter
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
- **DSP chain**: BBE sonic maximizer + limiter

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
# Build Standalone (Debug)
cmake --build build --config Debug --target IthacaPlayer_Standalone -j 4

# Build VST3 (Debug)
cmake --build build --config Debug --target IthacaPlayer_VST3 -j 4

# Build Standalone (Release)
cmake --build build --config Release --target IthacaPlayer_Standalone -j 4

# Build VST3 (Release)
cmake --build build --config Release --target IthacaPlayer_VST3 -j 4

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

### Plugin Formats

**Release Build:**
- **VST3**: `build/IthacaPlayer_artefacts/Release/VST3/IthacaPlayer.vst3/`
- **Standalone**: `build/IthacaPlayer_artefacts/Release/Standalone/IthacaPlayer.exe`
- **AU** (macOS): `build/IthacaPlayer_artefacts/Release/AU/IthacaPlayer.component`

**Debug Build:**
- **VST3**: `build/IthacaPlayer_artefacts/Debug/VST3/IthacaPlayer.vst3/`
- **Standalone**: `build/IthacaPlayer_artefacts/Debug/Standalone/IthacaPlayer.exe`

### Installed Files (POST_BUILD)

CMake automatically installs these files after each build:

1. **Decorators**: `%APPDATA%\LordAudio\IthacaPlayer\decorators\background.jpg`
   - Always overwrites to update with new builds
   - Also copied to `build/IthacaPlayer_artefacts/<Config>/Standalone/decorators/` as development fallback

2. **Core logger directory**: `%APPDATA%\LordAudio\IthacaPlayer\core_logger\`
   - Created automatically on first run

### Logs

Runtime logs are written to user roaming directory:
```
%APPDATA%\LordAudio\IthacaPlayer\core_logger\core_logger.log
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

### CMake Configuration Options

```bash
# Basic configure (no options required)
cmake -B build -S . -G "Visual Studio 17 2022" -A x64

# Custom default sample bank path
cmake -B build -S . -G "Visual Studio 17 2022" -A x64 -DSAMPLE_BANK_PATH="D:/Samples/VntV"
```

> **Note:** `INSTRUMENT_NAME` is deprecated and no longer used. The plugin builds as a unified `IthacaPlayer` target.

### Watching Logs (PowerShell)

```powershell
# Tail log file
Get-Content -Path "$env:APPDATA\LordAudio\IthacaPlayer\core_logger\core_logger.log" -Tail 20 -Wait

# Filter by component
Get-Content -Path "$env:APPDATA\LordAudio\IthacaPlayer\core_logger\core_logger.log" -Tail 20 -Wait | Where-Object { $_ -like "*VoiceManager*" }

# Filter by severity
Get-Content -Path "$env:APPDATA\LordAudio\IthacaPlayer\core_logger\core_logger.log" -Tail 20 -Wait | Where-Object { $_ -like "*ERROR*" }
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

### Audio Pipeline
```
MIDI Input → MidiProcessor → VoiceManager (128 voices) →
EnvelopeStaticData (RT-safe ADSR) → Voice Processing →
LFO Panning → DSP Chain (BBE + Limiter) → Master Gain/Pan → Audio Output
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
- **Voice pooling**: Pre-allocated 128 voices
- **SIMD potential**: Prepared for vectorization (currently scalar)
- **Lock-free**: Atomic parameters for GUI↔audio communication
- **Async loading**: Non-blocking sample loading in background thread

### Memory Usage
- **Sample buffers**: Loaded once, shared across voices
- **Envelope data**: Static tables (44.1kHz/48kHz)
- **Voice state**: Minimal per-voice overhead

## Troubleshooting

### Sample Loading Fails

1. **Plugin starts with sine waves:**
   - This is normal behavior - plugin initializes immediately with test tones
   - Use "Load Sample Bank..." button to load real samples

2. **Sample bank not loading after selection:**
   - Check log for diagnostics:
     ```powershell
     Get-Content "$env:APPDATA\LordAudio\IthacaPlayer\core_logger\core_logger.log" -Tail 20
     ```
   - Look for AsyncSampleLoader errors or InstrumentLoader failures

3. **Verify sample bank structure:**
   - See [SAMPLEPATHS.md](SAMPLEPATHS.md) for required directory structure
   - Ensure `instrument-definition.json` exists in selected directory
   - Check sample files are WAV format

4. **Sample bank loads but still uses sine waves:**
   - Check log for VoiceManager swap messages
   - Verify AsyncSampleLoader completed successfully

### Background Image Not Loading

1. **Check decorators directory:**
   ```powershell
   ls "$env:APPDATA\LordAudio\IthacaPlayer\decorators\background.jpg"
   ```

2. **Verify installation:**
   - CMake POST_BUILD installs `decorators/background.jpg` automatically
   - If missing, run build again to trigger installation

3. **Check log:**
   - Search for `IthacaPluginEditor/setupBackground` in log

### Build Errors

1. **Update submodules:**
   ```bash
   git submodule update --init --recursive
   ```

2. **Clean build folder:**
   ```bash
   rm -rf build
   cmake -B build -S . -G "Visual Studio 17 2022" -A x64
   ```

3. **Check MSVC installation:**
   ```cmd
   "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64
   ```

4. **Verify CMake version:**
   ```bash
   cmake --version  # Should be >= 3.22
   ```

### Plugin Not Found in DAW

1. **Check VST3 path:**
   ```
   C:\Program Files\Common Files\VST3\IthacaPlayer.vst3
   ```

2. **Copy manually from build directory:**
   ```bash
   cp -r build/IthacaPlayer_artefacts/Release/VST3/IthacaPlayer.vst3 \
     "C:\Program Files\Common Files\VST3\"
   ```

3. **Rescan plugins in DAW** or restart after adding new plugins.

## Contributing

### Code Style
- **C++17** standard
- **JUCE coding conventions** for plugin code (`ithaca/`)
- **Modern C++** for IthacaCore (`ithaca-core/`) - avoid raw pointers, use smart pointers
- **RT-safe** audio thread code (no allocations, no locks in `processBlock()`)

### Git Workflow
```bash
git checkout -b feature/my-feature
git add .
git commit -m "Description of changes"
git push -u origin feature/my-feature
```

## License

[Add license information here]

## Credits

- **JUCE Framework**: [juce.com](https://juce.com)
- **libsndfile**: [libsndfile.github.io](http://libsndfile.github.io/libsndfile/)
- **nlohmann/json**: [github.com/nlohmann/json](https://github.com/nlohmann/json)

## Documentation

- **[SAMPLEPATHS.md](SAMPLEPATHS.md)** - Complete sample bank path system documentation
- **[CMakeLists.txt](CMakeLists.txt)** - Build system configuration

## Support

For issues and feature requests, please use [GitHub Issues](https://github.com/alchy/Ithaca/issues).

---

**Version**: 1.1.0
**Build System**: CMake 3.22+
**Plugin Target**: IthacaPlayer (unified)
**Tested On**: Windows 10/11, Visual Studio 2022
