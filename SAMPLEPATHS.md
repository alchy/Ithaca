# Sample Bank Paths Configuration Guide

**Document Version:** 3.0
**Last Updated:** 2026-03-24
**Purpose:** Guide pro instalaci Ithaca VST/Standalone s dynamickým výběrem sample bank

**⚠️ MAJOR CHANGE:** Ithaca nyní používá **Single Unified Plugin** přístup - jeden plugin, runtime výběr sample bank.

---

## 📋 Obsah

1. [Přehled systému](#přehled-systému)
2. [Single Unified Plugin Architecture](#single-unified-plugin-architecture)
3. [Sine Wave Fallback](#sine-wave-fallback)
4. [Struktura souborů](#struktura-souborů)
5. [Konfigurace podle platforem](#konfigurace-podle-platforem)
6. [Build proces](#build-proces)
7. [Instalace - Zadání pro installer](#instalace---zadání-pro-installer)
8. [Řešení problémů](#řešení-problémů)
9. [Migrace z Multiple Instances (v2.x)](#migrace-z-multiple-instances-v2x)

---

## Přehled systému

Ithaca používá **Single Unified Plugin** přístup:
- **Jeden plugin binary**: `IthacaPlayer.vst3` / `IthacaPlayer.exe`
- **Runtime sample bank selection**: Uživatel vybere sample bank přes GUI folder picker
- **Sine wave fallback**: Plugin startuje s procedurálními sine waves když není sample bank
- **State persistence**: Vybraná sample bank cesta se ukládá v plugin state (DAW project)
- **No hardcoded paths**: Žádné build-time konfigurace nástrojů

### Architektura Flow

```
┌──────────────────────────────────────────────────────────────┐
│                   SINGLE UNIFIED PLUGIN                       │
├──────────────────────────────────────────────────────────────┤
│                                                               │
│  Plugin Binary: IthacaPlayer.vst3 (Code: Itca)              │
│      ↓                                                        │
│  1. STARTUP: Sine Waves (procedurální test tones)           │
│      └─ 128 MIDI notes × 8 velocity layers                   │
│      └─ 2-second stereo sine waves s fade in/out            │
│      └─ Ready to play immediately                            │
│                                                               │
│  2. USER ACTION: Select Sample Bank (GUI Folder Picker)     │
│      └─ User vybere: C:/SoundBanks/VintageV/                │
│                                                               │
│  3. ASYNC LOAD: Load real samples from selected directory   │
│      └─ Background thread načte WAV samples                  │
│      └─ Hot-swap: Replace sine waves with real samples      │
│                                                               │
│  4. STATE PERSISTENCE: Save path in DAW project              │
│      └─ Path saved in plugin state (XML)                     │
│      └─ Auto-load on project reopen                          │
│                                                               │
│  5. FALLBACK: If path invalid, revert to sine waves         │
│      └─ No crash, graceful degradation                       │
│                                                               │
│  User Data: %APPDATA%/LordAudio/IthacaPlayer/               │
│      └─ samplebank-config-Itca.json (optional default)      │
│      └─ decorators/background.jpg                            │
│      └─ core_logger/core_logger.log                          │
│                                                               │
└──────────────────────────────────────────────────────────────┘
```

### Klíčové principy

- **Single Plugin Binary**: Jeden plugin pro všechny nástroje
- **Runtime Selection**: User vybírá sample bank za běhu, ne při buildu
- **Sine Wave Fallback**: Vždy hraje, i bez sample banky
- **State Persistence**: Path k sample bank se ukládá v DAW projektu
- **Graceful Degradation**: Neexistující bank → fallback na sine waves
- **No Build-Time Configuration**: Žádné `-DINSTRUMENT_NAME` parametry

---

## Single Unified Plugin Architecture

### Přístup

Plugin je buildnut jako **jeden universal binary** bez hardcoded instrument:
- `IthacaPlayer.vst3` - VST3 plugin
- `IthacaPlayer.exe` - Standalone aplikace
- `IthacaPlayer.component` - AU plugin (macOS)

### Build Process

**Single unified build:**

```bash
# CMake configuration (NO -DINSTRUMENT_NAME!)
cmake -B build -S .

# Build
cmake --build build --config Release

# Output
build/IthacaPlayer_artefacts/Release/
├── VST3/IthacaPlayer.vst3
└── Standalone/IthacaPlayer.exe
```

**⚠️ ZMĚNA od verze 2.x:**
- ❌ **Odstraněno:** `-DINSTRUMENT_NAME=VintageV` parameter
- ❌ **Odstraněno:** Multiple build directories (build-vintagev, build-rhodes, atd.)
- ✅ **Nově:** Jeden build, jeden plugin pro všechny nástroje

### User Data Directory Structure

Plugin má **jeden unified user data adresář** v roaming profile:

```
Windows:
C:\Users\<user>\AppData\Roaming\LordAudio\IthacaPlayer\
├── samplebank-config-Itca.json     ← Optional default sample bank path
├── decorators\
│   └── background.jpg
└── core_logger\
    └── core_logger.log

macOS:
~/Library/Application Support/LordAudio/IthacaPlayer/
├── samplebank-config-Itca.json
├── decorators/
│   └── background.jpg
└── core_logger/
    └── core_logger.log

Linux:
~/.local/share/LordAudio/IthacaPlayer/
├── samplebank-config-Itca.json
├── decorators/
│   └── background.jpg
└── core_logger/
    └── core_logger.log
```

**User Data Files:**
- `samplebank-config-Itca.json` - **Optional** default sample bank path (can be empty)
- `decorators/background.jpg` - GUI background image
- `core_logger/core_logger.log` - Runtime log file

**⚠️ ZMĚNA od verze 2.x:**
- ❌ **Odstraněno:** `IthacaPlayer-VintageV/`, `IthacaPlayer-Rhodes/` adresáře
- ❌ **Odstraněno:** Multiple config files per instrument
- ✅ **Nově:** Jeden adresář `IthacaPlayer/` pro všechny nástroje
- ✅ **Nově:** Config je optional (plugin funguje i bez něj)

### Sample Bank Structure

**Flexible Structure:** User může mít sample banky kdekoliv na disku

```
Příklad 1: Organizované v subdirectories
C:\SoundBanks\
├── VintageV\
│   ├── instrument-definition.json
│   └── *.wav files (704)
├── Rhodes\
│   ├── instrument-definition.json
│   └── *.wav files (704)
└── Wurlitzer\
    ├── instrument-definition.json
    └── *.wav files (704)

Příklad 2: User's custom location
D:\My Music\Samples\Electric Pianos\MyCustomRhodes\
├── instrument-definition.json
└── *.wav files

Příklad 3: External drive
E:\Sample Libraries\Vintage Keys\EP Collection\
├── instrument-definition.json
└── *.wav files
```

**Naming Convention:** Adresáře mohou mít **jakékoliv jméno** (user choice)

### Benefits

✅ **Simplified Distribution**: Jeden plugin pro všechny nástroje
✅ **User Flexibility**: User vybírá kteroukoliv sample bank
✅ **Instant Playability**: Sine waves - plugin hraje okamžitě
✅ **Graceful Fallback**: Chybějící bank → sine waves (no crash)
✅ **State Persistence**: Path se ukládá v DAW projektu
✅ **Hot-Swap**: Změna sample bank za běhu bez restartu DAW

### Implementation Details

**CMakeLists.txt variables:**
- `PLUGIN_CODE`: `"Itca"` (unified legacy code)
- `PLUGIN_TARGET_NAME`: `"IthacaPlayer"`
- `PLUGIN_PRODUCT_NAME`: `"IthacaPlayer"`
- `SAMPLE_BANK_PATH`: Empty/Optional (runtime selection)

**Runtime behavior:**
1. **Startup**: Initialize with sine waves (procedural generation)
2. **Check config**: Read `samplebank-config-Itca.json` for default path (optional)
3. **Load state**: Restore sample bank path from DAW project state (if available)
4. **Async load**: Load sample bank in background thread (if path valid)
5. **User selection**: GUI folder picker allows changing sample bank
6. **Save state**: Store selected path in plugin state (persists in DAW project)

---

## Sine Wave Fallback

### Přehled

Plugin **vždy startuje** s procedurálními sine waves jako audio source:
- **128 MIDI notes** (C-1 to G9)
- **8 velocity layers** (linear amplitude mapping)
- **2-second duration** per note
- **Stereo** with slight phase offset (0.05 rad) for width
- **Fade in/out** (5ms / 10ms) for smooth ADSR application
- **Instant availability** (no file I/O required)

### Purpose

1. **Instant playability**: Plugin hraje okamžitě po startu
2. **Testing/Development**: Test MIDI routing, envelope, effects bez sample bank
3. **Graceful degradation**: Missing/deleted sample bank → still functional
4. **No crash guarantee**: Plugin nikdy nespadne kvůli missing samples

### Frequency Calculation

```
f(MIDI) = 440 Hz × 2^((MIDI - 69) / 12)

Examples:
MIDI 69 (A4)  → 440.00 Hz
MIDI 60 (C4)  → 261.63 Hz
MIDI 21 (A0)  → 27.50 Hz
MIDI 108 (C8) → 4186.01 Hz
```

### Technical Details

**Generator:** `ithaca-core/sampler/sine_wave_generator.h`
**Memory:** ~88 MB (128 notes × 8 layers × 2s × 44.1kHz × stereo float)
**Generation time:** ~6 seconds (first launch, cached afterward)
**RT-safe:** Yes (all data pre-generated, no allocation in audio thread)

---

## Struktura souborů

### 1. Runtime Config (`samplebank-config-Itca.json`) - OPTIONAL

**⚠️ IMPORTANT:** Tento soubor je **volitelný**! Plugin funguje i bez něj (sine wave mode).

**Umístění:**
- Windows: `%APPDATA%\LordAudio\IthacaPlayer\samplebank-config-Itca.json`
- macOS: `~/Library/Application Support/LordAudio/IthacaPlayer/samplebank-config-Itca.json`
- Linux: `~/.local/share/LordAudio/IthacaPlayer/samplebank-config-Itca.json`

**Formát:**
```json
{
  "sampleBankPath": "C:/SoundBanks/VintageV",
  "version": "1.0",
  "generatedBy": "User",
  "buildTimestamp": "260324120000",
  "platform": "Windows"
}
```

**Pole:**

| Pole | Typ | Povinné | Popis |
|------|-----|---------|-------|
| `sampleBankPath` | string | **ANO** | Absolutní cesta k sample bank (forward slashes!) |
| `version` | string | NE | Verze formátu (pro budoucí kompatibilitu) |
| `generatedBy` | string | NE | Zdroj konfigurace (`"Installer"`, `"User"`, `"GUI"`) |
| `buildTimestamp` | string | NE | Časové razítko (format: `YYMMDDHHMMSS`) |
| `platform` | string | NE | Platforma (`"Windows"`, `"Darwin"`, `"Linux"`) |

**Behavior:**
- **Soubor existuje + path valid** → Plugin načte sample bank z path
- **Soubor existuje + path invalid** → Plugin použije sine waves (log warning)
- **Soubor neexistuje** → Plugin použije sine waves (normální stav)
- **Soubor má JSON syntax error** → Plugin použije sine waves (log error)

**⚠️ DŮLEŽITÉ:**
- Cesta **MUSÍ** používat forward slashes: `C:/Path/To/Samples`
- Cesta **MUSÍ** být absolutní
- Config je **pouze default** - user může změnit přes GUI

### 2. Plugin State (DAW Project) - PRIMARY SOURCE

**Ukládání v DAW projektu:**

Plugin ukládá vybranou sample bank path do **plugin state** (JUCE AudioProcessorValueTreeState):

```xml
<IthacaPlayerState sampleBankPath="C:/SoundBanks/VintageV">
  <PARAM id="volume" value="0.8"/>
  <PARAM id="pan" value="0.0"/>
  <!-- ... other parameters -->
  <MidiLearn>
    <!-- MIDI learn mappings -->
  </MidiLearn>
</IthacaPlayerState>
```

**Priority:**
1. **Plugin State** (DAW project) - highest priority
2. **Config file** (`samplebank-config-Itca.json`) - fallback default
3. **Sine waves** - final fallback

**Workflow:**
```
User opens DAW project
  ↓
Plugin reads state from DAW project
  ↓
If sampleBankPath in state:
  └─ Load sample bank from path
  └─ If path invalid: Fallback to config file
     └─ If config invalid: Use sine waves
Else:
  └─ Check config file for default
     └─ If no config: Use sine waves
```

### 3. Instrument Metadata (`instrument-definition.json`)

**Umístění:** `<sampleBankPath>/instrument-definition.json`

**Formát:**
```json
{
  "instrumentName": "VintageV Electric Piano",
  "velocityMaps": "8",
  "instrumentVersion": "1.0.0",
  "author": "LordAudio",
  "description": "Vintage electric piano with 8 velocity layers",
  "category": "Piano",
  "sampleCount": 704
}
```

**Pole:**

| Pole | Typ | Povinné | Popis |
|------|-----|---------|-------|
| `instrumentName` | string | **ANO** | Název nástroje (zobrazí se v GUI) |
| `velocityMaps` | string | **ANO** | Počet velocity layers (`"1"` až `"8"`) - string! |
| `instrumentVersion` | string | NE | Verze sample banky |
| `author` | string | NE | Autor/tvůrce |
| `description` | string | NE | Popis nástroje |
| `category` | string | NE | Kategorie (`"Piano"`, `"Synth"`, atd.) |
| `sampleCount` | number | NE | Celkový počet WAV souborů |

**⚠️ DŮLEŽITÉ:**
- `velocityMaps` je **string**, ne číslo: `"8"` (NE `8`)
- Validní rozsah: `"1"` až `"8"`
- Pokud soubor chybí nebo je invalid, použije se fallback (adresář název, 8 layers)

### 4. Sample Files (WAV)

**Konvence názvů:** `<MIDI_nota>_<velocity_layer>.wav`

**Příklady:**
- `60_1.wav` - MIDI nota 60 (C4), velocity layer 1
- `60_8.wav` - MIDI nota 60 (C4), velocity layer 8
- `21_1.wav` - MIDI nota 21 (A0, nejnižší)
- `108_1.wav` - MIDI nota 108 (C8, nejvyšší)

**Rozsahy:**
- MIDI noty: `21` až `108` (88 kláves klavíru)
- Velocity layers: `1` až `8` (podle `velocityMaps`)

**Příklad struktury pro 8 velocity layers:**
```
SampleBank/
├── instrument-definition.json
├── 21_1.wav
├── 21_2.wav
├── ...
├── 21_8.wav
├── 60_1.wav
├── 60_2.wav
├── ...
├── 60_8.wav
├── 108_1.wav
├── ...
└── 108_8.wav

Celkem: 88 not × 8 layers = 704 WAV souborů
```

---

## Konfigurace podle platforem

### Windows

#### Výchozí cesty

| Co | Cesta |
|----|-------|
| **User data** | `C:\Users\<user>\AppData\Roaming\LordAudio\IthacaPlayer\` |
| **Config (optional)** | `%APPDATA%\LordAudio\IthacaPlayer\samplebank-config-Itca.json` |
| **Logs** | `%APPDATA%\LordAudio\IthacaPlayer\core_logger\core_logger.log` |
| **VST3 plugin** | `C:\Program Files\Common Files\VST3\IthacaPlayer.vst3\` |
| **Standalone exe** | `C:\Program Files\LordAudio\IthacaPlayer\IthacaPlayer.exe` |
| **Sample bank** | User choice (GUI picker) - example: `C:\SoundBanks\VintageV\` |

#### Environment variables
```cmd
%APPDATA%                = C:\Users\<user>\AppData\Roaming
%ProgramFiles%           = C:\Program Files
%CommonProgramFiles%     = C:\Program Files\Common Files
```

#### Příklad konfigurace
```json
{
  "sampleBankPath": "C:/SoundBanks/VintageV",
  "version": "1.0",
  "generatedBy": "User",
  "buildTimestamp": "260324120000",
  "platform": "Windows"
}
```

---

### macOS

#### Výchozí cesty

| Co | Cesta |
|----|-------|
| **User data** | `~/Library/Application Support/LordAudio/IthacaPlayer/` |
| **Config (optional)** | `~/Library/Application Support/LordAudio/IthacaPlayer/samplebank-config-Itca.json` |
| **Logs** | `~/Library/Application Support/LordAudio/IthacaPlayer/core_logger/core_logger.log` |
| **VST3 plugin** | `~/Library/Audio/Plug-Ins/VST3/IthacaPlayer.vst3/` |
| **AU plugin** | `~/Library/Audio/Plug-Ins/Components/IthacaPlayer.component/` |
| **Standalone app** | `/Applications/IthacaPlayer.app` |
| **Sample bank** | User choice (GUI picker) - example: `~/Music/Samples/VintageV/` |

#### Environment variables
```bash
$HOME                    = /Users/<username>
```

#### Příklad konfigurace
```json
{
  "sampleBankPath": "/Users/jindra/Music/Samples/VintageV",
  "version": "1.0",
  "generatedBy": "User",
  "buildTimestamp": "260324120000",
  "platform": "Darwin"
}
```

---

### Linux

#### Výchozí cesty

| Co | Cesta |
|----|-------|
| **User data** | `~/.local/share/LordAudio/IthacaPlayer/` |
| **Config (optional)** | `~/.local/share/LordAudio/IthacaPlayer/samplebank-config-Itca.json` |
| **Logs** | `~/.local/share/LordAudio/IthacaPlayer/core_logger/core_logger.log` |
| **VST3 plugin** | `~/.vst3/IthacaPlayer.vst3/` |
| **Standalone binary** | `/opt/IthacaPlayer/bin/IthacaPlayer` |
| **Sample bank** | User choice (GUI picker) - example: `~/.local/share/Samples/VintageV/` |

#### Environment variables
```bash
$HOME                    = /home/<username>
$XDG_DATA_HOME          = ~/.local/share (default)
```

#### Příklad konfigurace
```json
{
  "sampleBankPath": "/home/jindra/.local/share/Samples/VintageV",
  "version": "1.0",
  "generatedBy": "User",
  "buildTimestamp": "260324120000",
  "platform": "Linux"
}
```

---

## Build proces

### Fáze 1: CMAKE Configuration

**Soubory:**
- `samplebank-config.template.json` - Template (optional default)
- `CMakeLists.txt` - Build script

**Krok 1: Unified plugin settings** (CMakeLists.txt)
```cmake
# Single unified plugin (NO INSTRUMENT_NAME!)
set(PLUGIN_CODE "Itca")
set(PLUGIN_DESCRIPTION "Ithaca Player - Universal Sample Player")
set(PLUGIN_TARGET_NAME "IthacaPlayer")
set(PLUGIN_PRODUCT_NAME "IthacaPlayer")

# No default sample bank path (runtime selection)
# Config file is optional
```

**Krok 2: Generování metadat**
```cmake
# Timestamp (YYMMDDHHMMSS)
string(TIMESTAMP BUILD_TIMESTAMP "%y%m%d%H%M%S")

# Hostname
cmake_host_system_information(RESULT BUILD_HOST QUERY HOSTNAME)

# Git info
execute_process(COMMAND git rev-parse --short HEAD OUTPUT_VARIABLE GIT_HASH)
```

**Krok 3: Optional config generation** (CMakeLists.txt)
```cmake
# Generate empty/optional config
configure_file(
    samplebank-config.template.json
    build/generated/samplebank-config-Itca.json
    @ONLY
)
```

**Template (`samplebank-config.template.json`):**
```json
{
  "sampleBankPath": "",
  "version": "1.0",
  "generatedBy": "CMake",
  "buildTimestamp": "@BUILD_TIMESTAMP@",
  "platform": "@CMAKE_SYSTEM_NAME@"
}
```

**⚠️ ZMĚNA:** `sampleBankPath` je prázdný (runtime selection)

### Fáze 2: Build

Kompilace a linkování pluginu (VST3, Standalone, AU).

### Fáze 3: POST_BUILD

**Soubor:** `cmake/InstallSampleBankConfig.cmake`

**Krok 1: Detekce roaming directory**
```cmake
if(WIN32)
    file(TO_CMAKE_PATH "$ENV{APPDATA}" ROAMING_DIR)
elseif(APPLE)
    file(TO_CMAKE_PATH "$ENV{HOME}/Library/Application Support" ROAMING_DIR)
else()
    file(TO_CMAKE_PATH "$ENV{HOME}/.local/share" ROAMING_DIR)
endif()

set(PLUGIN_DATA_DIR "${ROAMING_DIR}/LordAudio/IthacaPlayer")
set(CONFIG_FILENAME "samplebank-config-Itca.json")
```

**Krok 2: Vytvoření adresáře + subdirectories**
```cmake
file(MAKE_DIRECTORY "${PLUGIN_DATA_DIR}")
file(MAKE_DIRECTORY "${PLUGIN_DATA_DIR}/decorators")
file(MAKE_DIRECTORY "${PLUGIN_DATA_DIR}/core_logger")
```

**Krok 3: Instalace config (optional, pouze pokud neexistuje)**
```cmake
set(TARGET_FILE "${PLUGIN_DATA_DIR}/${CONFIG_FILENAME}")

if(NOT EXISTS "${TARGET_FILE}")
    # Create empty/optional config
    file(WRITE "${TARGET_FILE}" "{\n  \"sampleBankPath\": \"\",\n  \"version\": \"1.0\"\n}")
else()
    message(STATUS "Config exists, preserving user settings")
endif()
```

**Example výsledná cesta:**
- Windows: `C:\Users\<user>\AppData\Roaming\LordAudio\IthacaPlayer\samplebank-config-Itca.json`
- macOS: `~/Library/Application Support/LordAudio/IthacaPlayer/samplebank-config-Itca.json`
- Linux: `~/.local/share/LordAudio/IthacaPlayer/samplebank-config-Itca.json`

---

## Instalace - Zadání pro installer

### Přehled instalace

Installer musí nainstalovat:
1. **VST3/AU plugin** binárku
2. **Standalone** aplikaci
3. **User data directories** (empty, plugin creates files)
4. **OPTIONAL:** Sample banks (user choice during install)

### Instalační kroky

#### Krok 1: Instalace pluginu

**Windows:**
```
Zdroj:     IthacaPlayer.vst3/
Cíl:       C:\Program Files\Common Files\VST3\IthacaPlayer.vst3\
Oprávnění: Vyžaduje admin práva
```

**macOS:**
```
Zdroj VST3: IthacaPlayer.vst3/
Cíl VST3:   ~/Library/Audio/Plug-Ins/VST3/IthacaPlayer.vst3/

Zdroj AU:   IthacaPlayer.component/
Cíl AU:     ~/Library/Audio/Plug-Ins/Components/IthacaPlayer.component/

Oprávnění:  User-level (žádný admin)
```

**Linux:**
```
Zdroj:     IthacaPlayer.vst3/
Cíl:       ~/.vst3/IthacaPlayer.vst3/
Oprávnění: User-level
```

#### Krok 2: Instalace Standalone

**Windows:**
```
Zdroj:     IthacaPlayer.exe
Cíl:       C:\Program Files\LordAudio\IthacaPlayer\IthacaPlayer.exe
Oprávnění: Vyžaduje admin práva
Desktop:   Vytvořit shortcut na Desktop (optional)
Start Menu: Vytvořit link v Start Menu
```

**macOS:**
```
Zdroj:     IthacaPlayer.app
Cíl:       /Applications/IthacaPlayer.app
Oprávnění: User-level (může vyžadovat heslo pro /Applications)
```

**Linux:**
```
Zdroj:     IthacaPlayer (binary)
Cíl:       /opt/IthacaPlayer/bin/IthacaPlayer
Desktop:   ~/.local/share/applications/ithacaplayer.desktop
Oprávnění: Může vyžadovat sudo pro /opt
```

#### Krok 3: Vytvoření User Data Directories

**Instalační akce:**

1. **Vytvořit base directory:**
   ```
   Windows: %APPDATA%\LordAudio\IthacaPlayer\
   macOS:   ~/Library/Application Support/LordAudio/IthacaPlayer/
   Linux:   ~/.local/share/LordAudio/IthacaPlayer/
   ```

2. **Vytvořit subdirectories:**
   ```
   decorators/      ← GUI background images
   core_logger/     ← Runtime log files
   ```

3. **Create optional empty config:**
   ```json
   {
     "sampleBankPath": "",
     "version": "1.0",
     "generatedBy": "Installer"
   }
   ```
   Save as: `samplebank-config-Itca.json`

**⚠️ IMPORTANT:** Config je **optional** - plugin funguje bez něj!

#### Krok 4: OPTIONAL - Instalace Sample Banks

**⚠️ NOVĚ:** Sample banks jsou **optional** během instalace!

**Installer workflow:**

```
┌──────────────────────────────────────────────────┐
│ Sample Bank Installation (Optional)               │
├──────────────────────────────────────────────────┤
│                                                   │
│ Ithaca can play sine wave test tones without     │
│ sample banks. Would you like to install sample   │
│ banks now?                                        │
│                                                   │
│ Available sample banks:                           │
│  [ ] VintageV Electric Piano (~2.5 GB)           │
│  [ ] Rhodes Mark II (~2.8 GB)                    │
│  [ ] Wurlitzer 200A (~2.3 GB)                    │
│  [ ] Acoustic Piano (~4.2 GB)                    │
│                                                   │
│ Installation path:                                │
│  [C:\SoundBanks\IthacaPlayer\   ] [Browse...]    │
│                                                   │
│ ⓘ You can add more sample banks later via        │
│   the plugin GUI folder picker.                  │
│                                                   │
│ [Skip] [Install Selected]                        │
└──────────────────────────────────────────────────┘
```

**Pokud user vybere install:**

1. **User si zvolí install path:**
   - Windows: `C:\SoundBanks\IthacaPlayer\`
   - macOS: `~/Library/Application Support/IthacaPlayer/`
   - Linux: `~/.local/share/IthacaPlayer/`

2. **Installer vytvoří subdirectories pro vybrané banky:**
   ```
   C:\SoundBanks\IthacaPlayer\
   ├── VintageV\
   │   ├── instrument-definition.json
   │   └── *.wav files (704)
   └── Rhodes\
       ├── instrument-definition.json
       └── *.wav files (704)
   ```

3. **OPTIONAL: Update config with default bank:**
   ```json
   {
     "sampleBankPath": "C:/SoundBanks/IthacaPlayer/VintageV",
     "version": "1.0",
     "generatedBy": "Installer"
   }
   ```

**Pokud user přeskočí install:**
- Plugin použije sine waves
- User může dodat sample banks později přes GUI

---

### Installer Workflow - Complete Flow

```
┌────────────────────────────────────────────────────────────┐
│                    INSTALLER START                          │
└────────────────────────────────────────────────────────────┘
                           ↓
┌────────────────────────────────────────────────────────────┐
│ STEP 1: Vítejte v instalaci Ithaca Player                  │
│ - Zobrazit licenci                                          │
│ - Uživatel souhlasí                                         │
└────────────────────────────────────────────────────────────┘
                           ↓
┌────────────────────────────────────────────────────────────┐
│ STEP 2: Výběr instalačního adresáře pro PLUGIN             │
│ Windows: C:\Program Files\Common Files\VST3\               │
│ macOS:   ~/Library/Audio/Plug-Ins/VST3/ (+ AU)            │
│ Linux:   ~/.vst3/                                           │
│                                                             │
│ [Změnit...]  [Pokračovat]                                  │
└────────────────────────────────────────────────────────────┘
                           ↓
┌────────────────────────────────────────────────────────────┐
│ STEP 3: Výběr instalačního adresáře pro STANDALONE         │
│ Windows: C:\Program Files\LordAudio\IthacaPlayer\          │
│ macOS:   /Applications/                                     │
│ Linux:   /opt/IthacaPlayer/                                 │
│                                                             │
│ [x] Vytvořit shortcut na Desktop                           │
│ [x] Přidat do Start Menu (Windows)                         │
│                                                             │
│ [Změnit...]  [Pokračovat]                                  │
└────────────────────────────────────────────────────────────┘
                           ↓
┌────────────────────────────────────────────────────────────┐
│ STEP 4: Sample Banks (OPTIONAL) ⓘ                         │
│                                                             │
│ Ithaca plays sine wave test tones by default.              │
│ Sample banks add realistic instrument sounds.              │
│                                                             │
│ Install sample banks now? (Optional)                       │
│  [ ] VintageV Electric Piano (~2.5 GB)                     │
│  [ ] Rhodes Mark II (~2.8 GB)                              │
│  [ ] Wurlitzer 200A (~2.3 GB)                              │
│  [ ] Acoustic Piano (~4.2 GB)                              │
│                                                             │
│ Install to: [C:\SoundBanks\IthacaPlayer\] [Browse...]     │
│                                                             │
│ ⓘ You can add sample banks later via GUI                  │
│                                                             │
│ [Skip - Use Sine Waves]  [Install Selected]                │
└────────────────────────────────────────────────────────────┘
                           ↓
┌────────────────────────────────────────────────────────────┐
│ STEP 5: Souhrn instalace                                   │
│                                                             │
│ Plugin:      <cesta z STEP 2>                              │
│ Standalone:  <cesta z STEP 3>                              │
│ Sample Banks: <vybrané banky z STEP 4 nebo "None">        │
│                                                             │
│ Celková velikost: ~X.X GB                                  │
│                                                             │
│ [Zpět]  [Instalovat]                                       │
└────────────────────────────────────────────────────────────┘
                           ↓
┌────────────────────────────────────────────────────────────┐
│ STEP 6: Instalace (Progress)                               │
│                                                             │
│ [▓▓▓▓▓▓░░░░] Kopírování pluginu...         60%             │
│                                                             │
│ Akce:                                                       │
│ 1. Kopírování VST3/AU pluginu              [██████] 100%   │
│ 2. Kopírování Standalone aplikace          [██████] 100%   │
│ 3. Vytvoření user data directories          [████░░]  80%   │
│ 4. Kopírování sample banks (optional)       [██░░░░]  40%   │
│ 5. Vytvoření shortcuts                      [░░░░░░]   0%   │
└────────────────────────────────────────────────────────────┘
                           ↓
┌────────────────────────────────────────────────────────────┐
│ STEP 7: Dokončení                                           │
│                                                             │
│ ✓ Ithaca Player byl úspěšně nainstalován!                 │
│                                                             │
│ Nainstalováno:                                              │
│   ✓ VST3 Plugin                                            │
│   ✓ Standalone aplikace                                    │
│   ✓ Sample banks: VintageV, Rhodes (optional)             │
│   ✓ User data directories                                  │
│                                                             │
│ ⓘ Plugin je připraven k použití se sine waves             │
│ ⓘ Vyberte sample bank přes GUI folder picker              │
│                                                             │
│ User Data: %APPDATA%\LordAudio\IthacaPlayer\              │
│ Logs: core_logger\core_logger.log                         │
│                                                             │
│ [x] Spustit Ithaca nyní                                    │
│                                                             │
│ [Dokončit]                                                  │
└────────────────────────────────────────────────────────────┘
```

---

## Řešení problémů

### Plugin hraje sine waves místo samples

**Symptom:**
- Plugin se spustí a hraje sine waves
- GUI zobrazuje "Sine Wave Test Tone"

**Možné příčiny:**

1. **Žádná sample bank nebyla vybrána** (normální stav)
   - **Řešení:** V GUI klikněte na "Load Sample Bank" button a vyberte sample bank directory

2. **Sample bank path v config je prázdný**
   ```json
   {
     "sampleBankPath": ""
   }
   ```
   - **Řešení:** Vyberte sample bank přes GUI nebo upravte config

3. **Sample bank path v config je invalid** (directory neexistuje)
   ```bash
   # Zkontrolovat log
   # Windows:
   type "%APPDATA%\LordAudio\IthacaPlayer\core_logger\core_logger.log" | findstr "Sample bank"

   # macOS/Linux:
   cat ~/Library/Application\ Support/LordAudio/IthacaPlayer/core_logger/core_logger.log | grep "Sample bank"
   ```
   - **Řešení:** Opravte cestu v config nebo vyberte novou přes GUI

4. **instrument-definition.json v sample bank je missing/invalid**
   - **Řešení:** Zkontrolujte že sample bank directory obsahuje validní `instrument-definition.json`

### Config file kontroly

**Zkontrolovat existenci:**
```bash
# Windows
dir "%APPDATA%\LordAudio\IthacaPlayer\samplebank-config-Itca.json"

# macOS
ls -la ~/Library/Application\ Support/LordAudio/IthacaPlayer/samplebank-config-Itca.json

# Linux
ls -la ~/.local/share/LordAudio/IthacaPlayer/samplebank-config-Itca.json
```

**Zkontrolovat JSON syntax:**
```bash
# Použijte JSON validator
cat samplebank-config-Itca.json | python -m json.tool
```

**Zkontrolovat forward slashes:**
```json
{
  "sampleBankPath": "C:/SoundBanks/VintageV"
                     ^-- MUSÍ být forward slash!
}
```

### Backslashes vs Forward Slashes

**❌ CHYBA:**
```json
{
  "sampleBankPath": "C:\\SoundBanks\\VintageV"
}
```

**✓ SPRÁVNĚ:**
```json
{
  "sampleBankPath": "C:/SoundBanks/VintageV"
}
```

**Proč?**
- JSON standard vyžaduje escapované backslashes: `\\`
- Plugin používá `std::filesystem::path()` který správně převede `/` na `\` ve Windows
- Forward slashes fungují na **všech platformách** včetně Windows

### Logs a diagnostika

**Log file location:**
- Windows: `%APPDATA%\LordAudio\IthacaPlayer\core_logger\core_logger.log`
- macOS: `~/Library/Application Support/LordAudio/IthacaPlayer/core_logger/core_logger.log`
- Linux: `~/.local/share/LordAudio/IthacaPlayer/core_logger/core_logger.log`

**Užitečné log entries:**
```
[INFO]: === SINE WAVE MODE: Starting initialization ===
[INFO]: === LOADING SAMPLE BANK ===
[ERROR]: Sample bank directory does not exist: <path>
[ERROR]: Failed to load sample bank: <error>
[WARNING]: Reverting to sine wave mode
```

**Sledování logu v real-time:**
```bash
# Windows
powershell Get-Content "%APPDATA%\LordAudio\IthacaPlayer\core_logger\core_logger.log" -Wait

# macOS/Linux
tail -f ~/Library/Application\ Support/LordAudio/IthacaPlayer/core_logger/core_logger.log
```

---

## Migrace z Multiple Instances (v2.x)

### Co se změnilo

**Version 2.x (Multiple Instances):**
- ❌ Multiple plugin binaries: `IthacaPlayer-VintageV.vst3`, `IthacaPlayer-Rhodes.vst3`
- ❌ Multiple user data directories: `IthacaPlayer-VintageV/`, `IthacaPlayer-Rhodes/`
- ❌ Build-time instrument selection: `-DINSTRUMENT_NAME=VintageV`
- ❌ Hardcoded sample bank paths in config
- ❌ Crash když sample bank missing

**Version 3.0 (Single Unified):**
- ✅ Single plugin binary: `IthacaPlayer.vst3`
- ✅ Single user data directory: `IthacaPlayer/`
- ✅ Runtime sample bank selection: GUI folder picker
- ✅ Sine wave fallback: Vždy hraje, i bez samples
- ✅ Graceful degradation: Missing bank → sine waves (no crash)

### Migrace kroků

1. **Uninstall old versions:**
   - Odstraňte staré plugin binaries: `IthacaPlayer-VintageV.vst3`, atd.
   - **ZACHOVAT** staré sample banks - jsou stále použitelné!

2. **Install novou verzi:**
   - Nainstalujte `IthacaPlayer.vst3` (unified plugin)

3. **Relink sample banks v DAW projektech:**
   - Otevřete starý project v DAW
   - Plugin se načte s sine waves (starý path je invalid)
   - V GUI vyberte sample bank přes folder picker
   - Save project - path se uloží do plugin state

4. **Optional: Update config file:**
   ```bash
   # Starý config (v2.x):
   %APPDATA%\LordAudio\IthacaPlayer-VintageV\samplebank-config-VntV.json

   # Nový config (v3.0):
   %APPDATA%\LordAudio\IthacaPlayer\samplebank-config-Itca.json
   ```

   Můžete zkopírovat `sampleBankPath` ze starého configu do nového.

5. **Sample banks zůstávají:**
   - Staré sample banks fungují bez úprav
   - Nemusíte je přesouvat ani měnit
   - Stačí je vybrat přes GUI

---

## Dodatečné poznámky pro vývojáře installeru

### 1. Platform paths helper

```python
import platform
import os

def get_platform_paths():
    system = platform.system()

    if system == "Windows":
        return {
            "plugin_vst3": "C:/Program Files/Common Files/VST3",
            "standalone": "C:/Program Files/LordAudio/IthacaPlayer",
            "user_data": os.path.join(os.environ["APPDATA"], "LordAudio", "IthacaPlayer"),
            "platform_name": "Windows"
        }
    elif system == "Darwin":
        home = os.environ["HOME"]
        return {
            "plugin_vst3": f"{home}/Library/Audio/Plug-Ins/VST3",
            "plugin_au": f"{home}/Library/Audio/Plug-Ins/Components",
            "standalone": "/Applications",
            "user_data": f"{home}/Library/Application Support/LordAudio/IthacaPlayer",
            "platform_name": "Darwin"
        }
    else:  # Linux
        home = os.environ["HOME"]
        return {
            "plugin_vst3": f"{home}/.vst3",
            "standalone": "/opt/IthacaPlayer/bin",
            "user_data": f"{home}/.local/share/LordAudio/IthacaPlayer",
            "platform_name": "Linux"
        }
```

### 2. Config generation helper (optional)

```python
import json
from datetime import datetime

def create_empty_config(platform_name):
    """
    Vytvoří prázdný config s optional default path
    """
    config = {
        "sampleBankPath": "",  # Empty - runtime selection
        "version": "1.0",
        "generatedBy": "Installer",
        "buildTimestamp": datetime.now().strftime("%y%m%d%H%M%S"),
        "platform": platform_name
    }

    return json.dumps(config, indent=2)
```

### 3. User data setup

```python
import os

def setup_user_data_directories(base_path):
    """
    Vytvoří user data directory structure
    """
    os.makedirs(base_path, exist_ok=True)
    os.makedirs(os.path.join(base_path, "decorators"), exist_ok=True)
    os.makedirs(os.path.join(base_path, "core_logger"), exist_ok=True)

    # Create optional empty config
    config_path = os.path.join(base_path, "samplebank-config-Itca.json")
    if not os.path.exists(config_path):
        config_content = create_empty_config(platform.system())
        with open(config_path, 'w') as f:
            f.write(config_content)
```

---

## Changelog

| Verze | Datum | Změny |
|-------|-------|-------|
| 3.0 | 2026-03-24 | **MAJOR CHANGE:** Single Unified Plugin architecture - odstranění Multiple Instances, sine wave fallback, runtime sample bank selection |
| 2.1 | 2025-11-01 | Config filename pattern: `samplebank-config-{PLUGIN_CODE}.json` |
| 2.0 | 2025-11-01 | Multiple Plugin Instances architecture - PLUGIN_CODE naming convention |
| 1.0 | 2025-11-01 | Initial version - kompletní dokumentace pro installer |

---

## Kontakt

Při problémech s instalací nebo konfigurací kontaktujte:
- **Email:** support@lordaudio.com
- **GitHub Issues:** https://github.com/alchy/Ithaca/issues

---

**© 2026 LordAudio - Ithaca Player**
