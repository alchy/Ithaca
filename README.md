# Ithaca Player - JUCE Audio Plugin

Professional sample-based virtual instrument plugin built with JUCE framework and IthacaCore audio engine.

## Project Overview

Ithaca Player is a VST3/AU/Standalone plugin that combines:
- **JUCE Framework** - Cross-platform audio plugin framework
- **IthacaCore** - Custom multiplatform sampler engine (128 voices, velocity layers, ADSR envelopes, LFO panning)
- **JSON Configuration** - Flexible sample bank path management
- **MIDI Learn** - Dynamic CC mapping for all parameters

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
git clone --recursive <repository-url>
cd Ithaca

# If already cloned without --recursive:
git submodule update --init --recursive
```

### 2. Configure Sample Bank Path (Optional)

Default path: `C:\SoundBanks\IthacaPlayer\instrument` (Windows)

To change, edit `CMakeLists.txt`:
```cmake
set(SAMPLE_BANK_PATH "C:/YourCustomPath/instrument" CACHE STRING "...")
```

Or override via command line:
```bash
cmake -B build -DSAMPLE_BANK_PATH="D:/MySamples/piano"
```

### 3. Build Project
```bash
# Configure
cmake -B build -G "Visual Studio 17 2022"

# Build
cmake --build build --config Debug
```

### 4. Run Standalone Application
```bash
./build/IthacaPlayer_artefacts/Debug/Standalone/IthacaPlayer.exe
```

## Sample Bank Configuration System

### JSON Config File

Sample bank paths are managed via `samplebank_config.json`:

**Location (platform-specific):**
- **Windows**: `%APPDATA%\LordAudio\IthacaPlayer\samplebank_config.json`
- **macOS**: `~/Library/Application Support/LordAudio/IthacaPlayer/samplebank_config.json`
- **Linux**: `~/.local/share/LordAudio/IthacaPlayer/samplebank_config.json`

**Format:**
```json
{
  "sampleBankPath": "C:/SoundBanks/IthacaPlayer/instrument",
  "version": "1.0",
  "generatedBy": "CMake",
  "buildTimestamp": "251009230525",
  "platform": "Windows"
}
```

### Behavior

1. **Build Time**: CMake generates JSON config from `SAMPLE_BANK_PATH` variable
2. **Install**: POST_BUILD step copies config to roaming profile (preserves existing user config)
3. **Runtime**: Plugin loads path from JSON config on startup
4. **User Changes**: `changeSampleDirectory()` API saves new path to JSON

**Note**: Path uses forward slashes (`/`) for JSON compatibility, even on Windows.

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
│   │   └── components/              # UI components (sliders, info panel)
│   ├── midi/                        # MIDI processing
│   │   ├── MidiProcessor.*          # MIDI message handling
│   │   └── MidiLearnManager.*       # Dynamic CC mapping
│   ├── parameters/                  # Parameter management
│   │   └── ParameterManager.*       # APVTS integration
│   └── config/                      # Configuration headers
│       ├── IthacaConfig.h           # Build configuration
│       └── AppConstants.h           # GUI/MIDI constants
├── ithaca-core/                     # Core audio engine (multiplatform)
│   └── sampler/                     # Sampler implementation
│       ├── voice_manager.*          # Voice management (128 voices)
│       ├── voice.*                  # Single voice processing
│       ├── envelopes/               # ADSR/ASR envelopes
│       ├── instrument_loader.*      # Sample loading
│       └── lfopan.*                 # LFO panning effect
├── cmake/                           # CMake scripts
│   └── InstallSampleBankConfig.cmake
├── decorators/                      # GUI assets
│   └── background.jpg
├── json/                            # nlohmann/json library (submodule)
├── JUCE/                            # JUCE framework (submodule)
├── libsndfile/                      # Audio file I/O (submodule)
├── CMakeLists.txt                   # Build configuration
└── samplebank_config.json.in        # JSON template
```

## Features

### Audio Engine (IthacaCore)
- **128 polyphonic voices** with velocity sensitivity
- **ADSR/ASR envelopes** with RT-safe static data
- **LFO panning** with configurable speed/depth
- **Stereo field** control
- **Master gain/pan** with MIDI CC support
- **Sustain pedal** support

### GUI Features
- **Modern interface** with rounded overlays
- **8 parameter sliders**: Master Gain, Master Pan, Attack, Release, Sustain, LFO Speed, LFO Depth, Stereo Field
- **Real-time statistics**: Active voices, sample rate, CPU usage
- **MIDI Learn**: Right-click any slider to assign MIDI CC
- **Performance monitor**: Dropout detection and CPU usage

### MIDI Implementation
- **Note On/Off** - Voice triggering with velocity
- **Sustain Pedal** (CC 64) - Hold notes
- **All Notes Off** (CC 123) - Emergency stop
- **Dynamic CC mapping** via MIDI Learn
- **State persistence** - Mappings saved with project

## Build Targets

```bash
# Build plugin (all formats)
cmake --build build --config Debug --target IthacaPlayer

# Clean logs
cmake --build build --target clean-logs

# Clean exports
cmake --build build --target clean-exports

# Clean all IthacaCore data
cmake --build build --target clean-all-ithaca
```

## Output Files

### Plugin Formats
- **VST3**: `build/IthacaPlayer_artefacts/Debug/VST3/IthacaPlayer.vst3/`
- **Standalone**: `build/IthacaPlayer_artefacts/Debug/Standalone/IthacaPlayer.exe`
- **AU** (macOS): `build/IthacaPlayer_artefacts/Debug/AU/IthacaPlayer.component`

### Logs
- **Main log**: `./core_logger/core_logger.log`
- **Test exports**: `./exports/tests/` (if testing enabled)

## Development

### Watching Logs (PowerShell)
```powershell
# Tail log file
Get-Content -Path "./core_logger/core_logger.log" -Tail 10 -Wait

# Filter by component
Get-Content -Path "./core_logger/core_logger.log" -Tail 10 -Wait | Where-Object { $_ -like "*VoiceManager*" }
```

### Debug Mode
Set `ITHACA_DEBUG_MODE` in `IthacaConfig.h` for:
- Verbose logging
- Assertions enabled
- No optimizations

### CMake Configuration Options

```bash
# Custom sample bank path
cmake -B build -DSAMPLE_BANK_PATH="D:/Samples/piano"

# Release build
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Specify generator
cmake -B build -G "Visual Studio 17 2022"
```

## Architecture

### Audio Pipeline
```
MIDI Input → MidiProcessor → VoiceManager (128 voices) →
EnvelopeStaticData (RT-safe ADSR) → Voice Processing →
LFO Panning → Master Gain/Pan → Audio Output
```

### Sample Loading
1. **Startup**: Load JSON config from roaming profile
2. **AsyncSampleLoader**: Background thread loads samples from configured path
3. **InstrumentLoader**: Parse metadata, allocate stereo buffers
4. **VoiceManager**: Initialize 128 voices with sample references
5. **Transfer**: Move VoiceManager to RT thread when ready

### Parameter Flow
```
GUI Slider → ParameterAttachment → AudioProcessorValueTreeState →
IthacaPluginProcessor → VoiceManager → Voice → Envelope (RT-safe update)
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

### Memory Usage
- **Sample buffers**: Loaded once, shared across voices
- **Envelope data**: Static tables (44.1kHz/48kHz)
- **Voice state**: Minimal per-voice overhead

## Troubleshooting

### Sample Loading Fails
1. Check JSON config exists: `%APPDATA%\LordAudio\IthacaPlayer\samplebank_config.json`
2. Verify path is valid and accessible
3. Check log: `./core_logger/core_logger.log`
4. Ensure samples are in correct format (WAV, stereo/mono)

### Build Errors
1. Update submodules: `git submodule update --init --recursive`
2. Clean build folder: `rm -rf build && cmake -B build`
3. Check MSVC installation: `vcvarsall.bat x64` works
4. Verify CMake version: `cmake --version` (>= 3.22)

### Plugin Not Found in DAW
1. Check VST3 path: `C:\Program Files\Common Files\VST3\`
2. Copy manually from `build/IthacaPlayer_artefacts/Debug/VST3/`
3. Rescan plugins in DAW
4. Check DAW plugin folder settings

## Contributing

### Code Style
- **C++17** standard
- **JUCE coding conventions** for plugin code
- **Modern C++** for IthacaCore (avoid raw pointers, use smart pointers)
- **RT-safe** audio thread code (no allocations, no locks)

### Git Workflow
```bash
# Create feature branch
git checkout -b feature/my-feature

# Commit changes
git add .
git commit -m "Description of changes"

# Push to remote
git push -u origin feature/my-feature
```

## License

[Add license information here]

## Credits

- **JUCE Framework**: [juce.com](https://juce.com)
- **libsndfile**: [libsndfile.github.io](http://libsndfile.github.io/libsndfile/)
- **nlohmann/json**: [github.com/nlohmann/json](https://github.com/nlohmann/json)

## Support

For issues and feature requests, please use GitHub Issues.

---

**Version**: 1.1.0
**Build System**: CMake 3.22+
**Tested On**: Windows 10/11, Visual Studio 2022
