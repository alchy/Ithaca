# Sample Bank Paths Configuration Guide

**Document Version:** 2.1
**Last Updated:** 2025-11-01
**Purpose:** Guide pro vytvoření automatického installeru pro Ithaca VST/Standalone a sample banky

**⚠️ IMPORTANT:** Ithaca používá **Multiple Plugin Instances** přístup - každý nástroj je samostatný plugin binary.

---

## 📋 Obsah

1. [Přehled systému](#přehled-systému)
2. [Multiple Plugin Instances Architecture](#multiple-plugin-instances-architecture)
3. [Struktura souborů](#struktura-souborů)
4. [Konfigurace podle platforem](#konfigurace-podle-platforem)
5. [Build proces](#build-proces)
6. [Instalace - Zadání pro installer](#instalace---zadání-pro-installer)
7. [Řešení problémů](#řešení-problémů)

---

## Přehled systému

Ithaca používá **Multiple Plugin Instances** přístup:
- Každý nástroj (VintageV, Rhodes, Wurlitzer) je **samostatný plugin binary**
- Každý plugin má vlastní config adresář a sample bank
- DAW vidí každý nástroj jako samostatný plugin
- Více nástrojů může běžet současně v jedné DAW session

### Architektura

```
┌─────────────────────────────────────────────────────────────┐
│              MULTIPLE PLUGIN INSTANCES                       │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│  Plugin Binary 1: IthacaPlayer-VintageV.vst3 (Code: VntV)   │
│      ↓                                                       │
│  Config: %APPDATA%/LordAudio/IthacaPlayer-VintageV/         │
│      └─ samplebank_config-VntV.json                         │
│          → Sample Bank: C:/SoundBanks/IthacaPlayer/VntV/    │
│                                                              │
│  Plugin Binary 2: IthacaPlayer-Rhodes.vst3 (Code: Rhds)     │
│      ↓                                                       │
│  Config: %APPDATA%/LordAudio/IthacaPlayer-Rhodes/           │
│      └─ samplebank_config-Rhds.json                         │
│          → Sample Bank: C:/SoundBanks/IthacaPlayer/Rhds/    │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

### Klíčové principy

- **Multiple Plugin Instances**: Každý nástroj = samostatný plugin binary
- **Oddělené konfigurace**: Každý plugin má vlastní config adresář
- **Samostatné sample banky**: Struktura `Samplebank-<InstrumentName>/`
- **Platform-agnostic paths**: Cesty používají forward slashes `/` (JSON standard)
- **Build-time configuration**: Instrument name je nastaven při CMake konfiguraci

---

## Multiple Plugin Instances Architecture

### Přístup

Každý nástroj je buildnut jako **samostatný plugin binary** s unikátním názvem:
- `IthacaPlayer-VintageV.vst3`
- `IthacaPlayer-Rhodes.vst3`
- `IthacaPlayer-Wurlitzer.vst3`

### Build Process

**Build jednotlivých nástrojů:**

```bash
# Build VintageV
cmake -B build-vintagev -S . -DINSTRUMENT_NAME=VintageV
cmake --build build-vintagev --config Release

# Build Rhodes
cmake -B build-rhodes -S . -DINSTRUMENT_NAME=Rhodes
cmake --build build-rhodes --config Release

# Build Wurlitzer
cmake -B build-wurli -S . -DINSTRUMENT_NAME=Wurlitzer
cmake --build build-wurli --config Release
```

**Output:**
```
build-vintagev/IthacaPlayer-VintageV_artefacts/Release/
├── VST3/IthacaPlayer-VintageV.vst3
└── Standalone/IthacaPlayer-VintageV.exe

build-rhodes/IthacaPlayer-Rhodes_artefacts/Release/
├── VST3/IthacaPlayer-Rhodes.vst3
└── Standalone/IthacaPlayer-Rhodes.exe
```

### User Data Directory Structure

Každý instrument instance má vlastní user data adresář v roaming profile s kompletní strukturou:

```
Windows:
C:\Users\<user>\AppData\Roaming\LordAudio\
├── IthacaPlayer-VintageV\
│   ├── samplebank-config-VntV.json → C:/SoundBanks/IthacaPlayer/VntV/
│   ├── decorators\
│   │   └── background.jpg
│   └── core_logger\
│       └── core_logger.log
├── IthacaPlayer-Rhodes\
│   ├── samplebank-config-Rhds.json → C:/SoundBanks/IthacaPlayer/Rhds/
│   ├── decorators\
│   │   └── background.jpg
│   └── core_logger\
│       └── core_logger.log
└── IthacaPlayer-Wurlitzer\
    ├── samplebank-config-Wrlz.json → C:/SoundBanks/IthacaPlayer/Wrlz/
    ├── decorators\
    │   └── background.jpg
    └── core_logger\
        └── core_logger.log

macOS:
~/Library/Application Support/LordAudio/
├── IthacaPlayer-VintageV/
│   ├── samplebank-config-VntV.json
│   ├── decorators/
│   │   └── background.jpg
│   └── core_logger/
│       └── core_logger.log
└── ...

Linux:
~/.local/share/LordAudio/
├── IthacaPlayer-VintageV/
│   ├── samplebank-config-VntV.json
│   ├── decorators/
│   │   └── background.jpg
│   └── core_logger/
│       └── core_logger.log
└── ...
```

**User Data Files:**
- `samplebank-config-{PLUGIN_CODE}.json` - Sample bank path configuration
- `decorators/background.jpg` - GUI background image
- `core_logger/core_logger.log` - Runtime log file

**Naming Convention:**
- Config file: `samplebank-config-{PLUGIN_CODE}.json`
- Matches pattern used by `instrument-definition.json`
- Each instrument instance has unique config file based on 4-char PLUGIN_CODE
- All user data is isolated per instrument instance

### Sample Bank Structure

**Naming Convention:** Sample bank directories use **PLUGIN_CODE** (4-char identifiers)

```
C:\SoundBanks\IthacaPlayer\
├── VntV\                    ← VintageV (PLUGIN_CODE: VntV)
│   ├── instrument-definition.json
│   ├── 21_1.wav
│   └── ... (704 WAV files)
├── Rhds\                    ← Rhodes (PLUGIN_CODE: Rhds)
│   ├── instrument-definition.json
│   ├── 21_1.wav
│   └── ... (704 WAV files)
└── Wrlz\                    ← Wurlitzer (PLUGIN_CODE: Wrlz)
    ├── instrument-definition.json
    ├── 21_1.wav
    └── ... (704 WAV files)
```

**Plugin Code Registry (LordAudio/Lau0):**
- `VntV` - VintageV Electric Piano
- `Rhds` - Rhodes Mark II Electric Piano
- `Wrlz` - Wurlitzer 200A Electric Piano
- `Pian` - Acoustic Piano
- `Itca` - Legacy/fallback code

### Benefits

✅ **DAW Integration**: Každý nástroj se zobrazí samostatně v DAW plugin listu
✅ **Concurrent Use**: Více nástrojů může běžet současně v jedné session
✅ **Separate Presets**: Každý nástroj má vlastní presety
✅ **Simple Distribution**: Installer může nabídnout výběr nástrojů k instalaci
✅ **No Code Changes in ithaca-core**: Core engine dostává cestu v konstruktoru

### Implementation Details

**CMakeLists.txt variables:**
- `INSTRUMENT_NAME`: Název nástroje (např. `"VintageV"`)
- `PLUGIN_CODE`: 4-char identifikátor (např. `"VntV"`)
- `PLUGIN_TARGET_NAME`: `IthacaPlayer-${INSTRUMENT_NAME}`
- `PLUGIN_PRODUCT_NAME`: `"IthacaPlayer ${INSTRUMENT_NAME}"` (zobrazí se v DAW)
- `SAMPLE_BANK_PATH`: `C:/SoundBanks/IthacaPlayer/${PLUGIN_CODE}`

**Plugin Code Mapping:**
```cmake
if(INSTRUMENT_NAME STREQUAL "VintageV")
    set(PLUGIN_CODE "VntV")
elseif(INSTRUMENT_NAME STREQUAL "Rhodes")
    set(PLUGIN_CODE "Rhds")
elseif(INSTRUMENT_NAME STREQUAL "Wurlitzer")
    set(PLUGIN_CODE "Wrlz")
# ... další nástroje
endif()
```

**Runtime detection:**
Plugin používá preprocessor define `ITHACA_PLUGIN_TARGET_NAME` pro určení vlastního config adresáře.

---

## Struktura souborů

### 1. Runtime Config (`samplebank-config-{PLUGIN_CODE}.json`)

**Formát (example pro VintageV - `samplebank-config-VntV.json`):**
```json
{
  "sampleBankPath": "C:/SoundBanks/IthacaPlayer/VntV",
  "version": "1.0",
  "generatedBy": "CMake",
  "buildTimestamp": "251101192141",
  "platform": "Windows"
}
```

**⚠️ ZMĚNA:** Path nyní používá **PLUGIN_CODE** (4-char identifier)
- VintageV: `C:/SoundBanks/IthacaPlayer/VntV`
- Rhodes: `C:/SoundBanks/IthacaPlayer/Rhds`
- Wurlitzer: `C:/SoundBanks/IthacaPlayer/Wrlz`

**Pole:**

| Pole | Typ | Povinné | Popis |
|------|-----|---------|-------|
| `sampleBankPath` | string | **ANO** | Absolutní cesta k adresáři se samply (forward slashes!) |
| `version` | string | NE | Verze formátu konfigurace (pro budoucí kompatibilitu) |
| `generatedBy` | string | NE | Zdroj konfigurace (`"CMake"`, `"Installer"`, `"User"`) |
| `buildTimestamp` | string | NE | Časové razítko vytvoření (format: `YYMMDDHHMMSS`) |
| `platform` | string | NE | Platforma (`"Windows"`, `"Darwin"`, `"Linux"`) |

**⚠️ DŮLEŽITÉ:**
- Plugin čte **pouze** pole `sampleBankPath`
- Cesta **MUSÍ** používat forward slashes: `C:/Path/To/Samples` (NE `C:\Path\To\Samples`)
- Cesta **MUSÍ** být absolutní
- Ostatní pole jsou informativní

### 2. Instrument Metadata (`instrument-definition.json`)

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
- Pokud soubor chybí nebo je invalid, použije se fallback (název adresáře, 8 layers)

### 3. Sample Files (WAV)

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
instrument/
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

#### Výchozí cesty (example pro VintageV)

| Co | Cesta |
|----|-------|
| **Runtime config** | `C:\Users\<user>\AppData\Roaming\LordAudio\IthacaPlayer-VintageV\samplebank-config-VntV.json` |
| **Sample bank** | `C:\SoundBanks\IthacaPlayer\VntV\` |
| **VST3 plugin** | `C:\Program Files\Common Files\VST3\IthacaPlayer-VintageV.vst3\` |
| **Standalone exe** | `C:\Program Files\LordAudio\IthacaPlayer\IthacaPlayer-VintageV.exe` |

#### Environment variables
```cmd
%APPDATA%                = C:\Users\<user>\AppData\Roaming
%ProgramFiles%           = C:\Program Files
%CommonProgramFiles%     = C:\Program Files\Common Files
```

#### Příklad konfigurace (VintageV)
```json
{
  "sampleBankPath": "C:/SoundBanks/IthacaPlayer/VntV",
  "version": "1.0",
  "generatedBy": "Installer",
  "buildTimestamp": "251101120000",
  "platform": "Windows"
}
```

---

### macOS

#### Výchozí cesty (example pro VintageV)

| Co | Cesta |
|----|-------|
| **Runtime config** | `~/Library/Application Support/LordAudio/IthacaPlayer-VintageV/samplebank-config-VntV.json` |
| **Sample bank** | `~/Library/Application Support/IthacaPlayer/VntV/` |
| **VST3 plugin** | `~/Library/Audio/Plug-Ins/VST3/IthacaPlayer-VintageV.vst3/` |
| **AU plugin** | `~/Library/Audio/Plug-Ins/Components/IthacaPlayer-VintageV.component/` |
| **Standalone app** | `/Applications/IthacaPlayer-VintageV.app` |

#### Environment variables
```bash
$HOME                    = /Users/<username>
```

#### Příklad konfigurace (VintageV)
```json
{
  "sampleBankPath": "/Users/jindra/Library/Application Support/IthacaPlayer/VntV",
  "version": "1.0",
  "generatedBy": "Installer",
  "buildTimestamp": "251101120000",
  "platform": "Darwin"
}
```

---

### Linux

#### Výchozí cesty (example pro VintageV)

| Co | Cesta |
|----|-------|
| **Runtime config** | `~/.local/share/LordAudio/IthacaPlayer-VintageV/samplebank-config-VntV.json` |
| **Sample bank** | `~/.local/share/IthacaPlayer/VntV/` |
| **VST3 plugin** | `~/.vst3/IthacaPlayer-VintageV.vst3/` |
| **Standalone binary** | `/opt/IthacaPlayer/bin/IthacaPlayer-VintageV` |

#### Environment variables
```bash
$HOME                    = /home/<username>
$XDG_DATA_HOME          = ~/.local/share (default)
```

#### Příklad konfigurace (VintageV)
```json
{
  "sampleBankPath": "/home/jindra/.local/share/IthacaPlayer/VntV",
  "version": "1.0",
  "generatedBy": "Installer",
  "buildTimestamp": "251101120000",
  "platform": "Linux"
}
```

---

## Build proces

### Fáze 1: CMAKE Configuration

**Soubory:**
- `samplebank_config.json.in` - Template s placeholdery
- `CMakeLists.txt` - Build script

**Krok 1: Nastavení výchozí cesty s PLUGIN_CODE** (CMakeLists.txt)
```cmake
# PLUGIN_CODE je nastaven podle INSTRUMENT_NAME (VntV, Rhds, Wrlz, etc.)
if(WIN32)
    set(SAMPLE_BANK_PATH "C:/SoundBanks/IthacaPlayer/${PLUGIN_CODE}")
elseif(APPLE)
    set(SAMPLE_BANK_PATH "$ENV{HOME}/Library/Application Support/IthacaPlayer/${PLUGIN_CODE}")
else()
    set(SAMPLE_BANK_PATH "$ENV{HOME}/.local/share/IthacaPlayer/${PLUGIN_CODE}")
endif()
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

**Krok 3: Generování JSON** (CMakeLists.txt:102-107)
```cmake
configure_file(
    samplebank_config.json.in
    build/generated/samplebank_config.json
    @ONLY
)
```

**Template (`samplebank_config.json.in`):**
```json
{
  "sampleBankPath": "@SAMPLE_BANK_PATH@",
  "version": "1.0",
  "generatedBy": "CMake",
  "buildTimestamp": "@BUILD_TIMESTAMP@",
  "platform": "@CMAKE_SYSTEM_NAME@"
}
```

**Výstup (`build/generated/samplebank_config.json`) - example pro VintageV:**
```json
{
  "sampleBankPath": "C:/SoundBanks/IthacaPlayer/VntV",
  "version": "1.0",
  "generatedBy": "CMake",
  "buildTimestamp": "251101192141",
  "platform": "Windows"
}
```

### Fáze 2: Build

Kompilace a linkování pluginu (VST3, Standalone, AU).

### Fáze 3: POST_BUILD

**Soubor:** `cmake/InstallSampleBankConfig.cmake`

**Krok 1: Detekce roaming directory (s PLUGIN_NAME a PLUGIN_CODE)**
```cmake
if(WIN32)
    # C:\Users\<user>\AppData\Roaming
    file(TO_CMAKE_PATH "$ENV{APPDATA}" ROAMING_DIR)
elseif(APPLE)
    # ~/Library/Application Support
    file(TO_CMAKE_PATH "$ENV{HOME}/Library/Application Support" ROAMING_DIR)
else()
    # ~/.local/share
    file(TO_CMAKE_PATH "$ENV{HOME}/.local/share" ROAMING_DIR)
endif()

# PLUGIN_NAME je např. "IthacaPlayer-VintageV"
set(PLUGIN_DATA_DIR "${ROAMING_DIR}/LordAudio/${PLUGIN_NAME}")

# Config filename s PLUGIN_CODE (např. "samplebank-config-VntV.json")
set(CONFIG_FILENAME "samplebank-config-${PLUGIN_CODE}.json")
```

**Krok 2: Vytvoření adresáře**
```cmake
file(MAKE_DIRECTORY "${PLUGIN_DATA_DIR}")
```

**Krok 3: Instalace (pouze pokud neexistuje)**
```cmake
set(TARGET_FILE "${PLUGIN_DATA_DIR}/${CONFIG_FILENAME}")

if(NOT EXISTS "${TARGET_FILE}")
    # Čte build/generated/samplebank_config.json
    file(READ "${SOURCE_JSON}" JSON_CONTENT)

    # Oprava backslashes → forward slashes
    string(REPLACE "\\" "/" JSON_CONTENT_FIXED "${JSON_CONTENT}")

    # Zapíše do roaming profile (např. samplebank-config-VntV.json)
    file(WRITE "${TARGET_FILE}" "${JSON_CONTENT_FIXED}")
else()
    # PŘESKOČÍ - zachová user settings!
    message(STATUS "Config exists, preserving user settings")
endif()
```

**Example výsledná cesta:**
- Windows: `C:\Users\<user>\AppData\Roaming\LordAudio\IthacaPlayer-VintageV\samplebank-config-VntV.json`
- macOS: `~/Library/Application Support/LordAudio/IthacaPlayer-VintageV/samplebank-config-VntV.json`
- Linux: `~/.local/share/LordAudio/IthacaPlayer-VintageV/samplebank-config-VntV.json`

**⚠️ KLÍČOVÁ VLASTNOST:**
- Build **NIKDY** nepřepíše existující konfiguraci
- Uživatelská nastavení zůstávají zachována při rebuildech

---

## Instalace - Zadání pro installer

### Přehled instalace

Installer musí nainstalovat:
1. **VST3/AU plugin** binárky
2. **Standalone** aplikaci
3. **Sample bank** (WAV soubory + metadata)
4. **Runtime config** (JSON soubor s cestou)

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

#### Krok 3: Instalace Sample Bank

**Výchozí cíl podle platformy:**

| Platforma | Výchozí cesta | Customizable? |
|-----------|---------------|---------------|
| Windows | `C:\SoundBanks\IthacaPlayer\instrument\` | **ANO** |
| macOS | `~/Library/Application Support/IthacaPlayer/instrument/` | **ANO** |
| Linux | `~/.local/share/IthacaPlayer/instrument/` | **ANO** |

**Obsah:**
```
instrument/
├── instrument-definition.json
├── 21_1.wav
├── 21_2.wav
├── ...
└── 108_8.wav

Celkem: ~704 souborů (88 not × 8 layers)
Velikost: Závisí na sample rate a délce
```

**Instalační akce:**
1. Uživatel si může **vybrat vlastní cestu** (dialog v installeru)
2. Installer vytvoří adresář pokud neexistuje
3. Zkopíruje všechny WAV soubory
4. Zkopíruje `instrument-definition.json`
5. Uloží vybranou cestu pro použití v kroku 4

**Permissions:**
- Windows: User má právo zapisovat do `C:\SoundBanks\` (vytvořit při instalaci)
- macOS/Linux: User-level adresáře, žádný admin potřebný

#### Krok 4: Vytvoření Runtime Config

**Výchozí cíl podle platformy (example pro VintageV):**

| Platforma | Cesta |
|-----------|-------|
| Windows | `%APPDATA%\LordAudio\IthacaPlayer-VintageV\samplebank-config-VntV.json` |
| macOS | `~/Library/Application Support/LordAudio/IthacaPlayer-VintageV/samplebank-config-VntV.json` |
| Linux | `~/.local/share/LordAudio/IthacaPlayer-VintageV/samplebank-config-VntV.json` |

**Instalační akce:**

1. **Vytvořit adresář (příklad pro VintageV):**
   ```
   Windows: %APPDATA%\LordAudio\IthacaPlayer-VintageV\
   macOS:   ~/Library/Application Support/LordAudio/IthacaPlayer-VintageV/
   Linux:   ~/.local/share/LordAudio/IthacaPlayer-VintageV/
   ```

2. **Zkontrolovat existenci (příklad pro VintageV - samplebank-config-VntV.json):**
   ```
   if (samplebank-config-VntV.json NEEXISTUJE):
       VYTVOŘ nový soubor
   else:
       DOTAZ NA UŽIVATELE:
       - "Config již existuje. Přepsat?"
       - [Ano] → Přepíše s novou cestou
       - [Ne]  → Zachová existující config
   ```

3. **Generovat JSON content:**
   ```json
   {
     "sampleBankPath": "<cesta_zvolena_v_kroku3>",
     "version": "1.0",
     "generatedBy": "Installer",
     "buildTimestamp": "<timestamp_instalace>",
     "platform": "<Windows|Darwin|Linux>"
   }
   ```

4. **⚠️ KRITICKÉ: Forward slashes!**
   ```
   Windows uživatel zvolil:  C:\SoundBanks\IthacaPlayer\instrument

   JSON musí obsahovat:      C:/SoundBanks/IthacaPlayer/instrument
                             ^                         ^
                             Forward slash!
   ```

**Pseudokód pro installer (example pro VintageV):**
```python
def create_runtime_config(sample_bank_path, platform, plugin_name, plugin_code):
    # Example: plugin_name="IthacaPlayer-VintageV", plugin_code="VntV"

    # 1. Detekce roaming directory
    if platform == "Windows":
        config_dir = os.path.join(os.environ["APPDATA"], "LordAudio", plugin_name)
    elif platform == "Darwin":
        config_dir = os.path.join(os.environ["HOME"], "Library", "Application Support", "LordAudio", plugin_name)
    else:  # Linux
        config_dir = os.path.join(os.environ["HOME"], ".local", "share", "LordAudio", plugin_name)

    # 2. Vytvořit adresář
    os.makedirs(config_dir, exist_ok=True)

    # 3. Cesta k config souboru (s PLUGIN_CODE)
    config_filename = f"samplebank-config-{plugin_code}.json"
    config_file = os.path.join(config_dir, config_filename)

    # 4. Zkontrolovat existenci
    if os.path.exists(config_file):
        if not ask_user_overwrite():
            return  # Zachovat existující

    # 5. Opravit cesty (forward slashes)
    sample_bank_path_fixed = sample_bank_path.replace("\\", "/")

    # 6. Vytvořit JSON
    config = {
        "sampleBankPath": sample_bank_path_fixed,
        "version": "1.0",
        "generatedBy": "Installer",
        "buildTimestamp": datetime.now().strftime("%y%m%d%H%M%S"),
        "platform": platform
    }

    # 7. Zapsat soubor
    with open(config_file, 'w') as f:
        json.dump(config, f, indent=2)
```

---

### Installer Workflow - Complete Flow

```
┌────────────────────────────────────────────────────────────┐
│                    INSTALLER START                          │
└────────────────────────────────────────────────────────────┘
                           ↓
┌────────────────────────────────────────────────────────────┐
│ STEP 1: Vítejte v instalaci Ithaca                         │
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
│ STEP 4: Výběr adresáře pro SAMPLE BANK ⚠️ DŮLEŽITÉ!       │
│                                                             │
│ Sample banka obsahuje 704 WAV souborů (~2-5 GB)            │
│                                                             │
│ Výchozí:                                                    │
│ Windows: C:\SoundBanks\IthacaPlayer\instrument\            │
│ macOS:   ~/Library/Application Support/IthacaPlayer/...    │
│ Linux:   ~/.local/share/IthacaPlayer/instrument/           │
│                                                             │
│ [Změnit...]  [Pokračovat]                                  │
└────────────────────────────────────────────────────────────┘
                           ↓
┌────────────────────────────────────────────────────────────┐
│ STEP 5: Souhrn instalace                                   │
│                                                             │
│ Plugin:      <cesta z STEP 2>                              │
│ Standalone:  <cesta z STEP 3>                              │
│ Sample Bank: <cesta z STEP 4>                              │
│                                                             │
│ Celková velikost: ~2.5 GB                                  │
│                                                             │
│ [Zpět]  [Instalovat]                                       │
└────────────────────────────────────────────────────────────┘
                           ↓
┌────────────────────────────────────────────────────────────┐
│ STEP 6: Instalace (Progress)                               │
│                                                             │
│ [▓▓▓▓▓▓░░░░] Kopírování pluginu...         25%             │
│                                                             │
│ Akce:                                                       │
│ 1. Kopírování VST3/AU pluginu              [████░░] 80%    │
│ 2. Kopírování Standalone aplikace          [██░░░░] 40%    │
│ 3. Kopírování sample banky (704 souborů)   [░░░░░░]  0%    │
│ 4. Vytvoření config souboru                 [░░░░░░]  0%    │
│ 5. Vytvoření shortcuts                      [░░░░░░]  0%    │
└────────────────────────────────────────────────────────────┘
                           ↓
┌────────────────────────────────────────────────────────────┐
│ STEP 7: Dokončení                                           │
│                                                             │
│ ✓ Ithaca byl úspěšně nainstalován!                        │
│                                                             │
│ Nainstalováno:                                              │
│   ✓ VST3 Plugin                                            │
│   ✓ Standalone aplikace                                    │
│   ✓ Sample banka (704 WAV souborů)                        │
│   ✓ Konfigurace                                            │
│                                                             │
│ Konfigurace uložena (example pro VintageV):                │
│   %APPDATA%\LordAudio\IthacaPlayer-VintageV\               │
│   samplebank-config-VntV.json                              │
│                                                             │
│ [x] Spustit Ithaca nyní                                    │
│                                                             │
│ [Dokončit]                                                  │
└────────────────────────────────────────────────────────────┘
```

---

### Kontrolní seznam pro instalaci

**Po instalaci zkontrolovat:**

- [ ] **VST3 plugin existuje:**
  - Windows: `C:\Program Files\Common Files\VST3\IthacaPlayer.vst3\Contents\x86_64-win\IthacaPlayer.vst3`
  - macOS: `~/Library/Audio/Plug-Ins/VST3/IthacaPlayer.vst3/Contents/MacOS/IthacaPlayer`
  - Linux: `~/.vst3/IthacaPlayer.vst3/Contents/x86_64-linux/IthacaPlayer.so`

- [ ] **Standalone existuje:**
  - Windows: `C:\Program Files\LordAudio\IthacaPlayer\IthacaPlayer.exe`
  - macOS: `/Applications/IthacaPlayer.app`
  - Linux: `/opt/IthacaPlayer/bin/IthacaPlayer`

- [ ] **Sample bank adresář obsahuje:**
  - [ ] `instrument-definition.json` (validní JSON)
  - [ ] 704 WAV souborů (21_1.wav až 108_8.wav)
  - [ ] Celková velikost odpovídá očekávání

- [ ] **Runtime config existuje:**
  - Windows: `%APPDATA%\LordAudio\IthacaPlayer\samplebank_config.json`
  - macOS: `~/Library/Application Support/LordAudio/IthacaPlayer/samplebank_config.json`
  - Linux: `~/.local/share/LordAudio/IthacaPlayer/samplebank_config.json`

- [ ] **Runtime config je validní:**
  ```bash
  # Ověřit JSON syntax
  cat <config_file> | python -m json.tool

  # Ověřit forward slashes
  grep "sampleBankPath" <config_file>
  # Očekávaný výstup: "sampleBankPath": "C:/SoundBanks/..."
  #                                        ^-- Forward slash!
  ```

- [ ] **Sample bank cesta v config je správná:**
  ```bash
  # Extrahovat cestu z JSON
  cat <config_file> | grep sampleBankPath

  # Ověřit že adresář existuje
  # Windows: dir "C:\SoundBanks\IthacaPlayer\instrument"
  # macOS/Linux: ls -la ~/Library/Application\ Support/IthacaPlayer/instrument/
  ```

---

## Řešení problémů

### Plugin nenachází samply při startu

**Symptom:**
- Plugin se spustí, ale nepřehrává žádný zvuk
- V logu: `"No sample bank config found!"`

**Řešení:**

1. **Zkontrolovat existenci config souboru (example pro VintageV):**
   ```bash
   # Windows
   dir "%APPDATA%\LordAudio\IthacaPlayer-VintageV\samplebank-config-VntV.json"

   # macOS
   ls -la ~/Library/Application\ Support/LordAudio/IthacaPlayer-VintageV/samplebank-config-VntV.json

   # Linux
   ls -la ~/.local/share/LordAudio/IthacaPlayer-VintageV/samplebank-config-VntV.json
   ```

2. **Zkontrolovat JSON syntax:**
   ```bash
   # Použijte JSON validator
   cat samplebank-config-VntV.json | python -m json.tool
   ```

3. **Zkontrolovat cestu v JSON:**
   ```json
   {
     "sampleBankPath": "C:/SoundBanks/IthacaPlayer/instrument"
                        ^-- MUSÍ být forward slash!
   }
   ```

4. **Ověřit že sample bank adresář existuje:**
   ```bash
   # Windows
   dir "C:\SoundBanks\IthacaPlayer\instrument\instrument-definition.json"

   # macOS/Linux
   ls -la ~/Library/Application\ Support/IthacaPlayer/instrument/instrument-definition.json
   ```

### Backslashes vs Forward Slashes

**❌ CHYBA:**
```json
{
  "sampleBankPath": "C:\\SoundBanks\\IthacaPlayer\\instrument"
}
```

**✓ SPRÁVNĚ:**
```json
{
  "sampleBankPath": "C:/SoundBanks/IthacaPlayer/instrument"
}
```

**Proč?**
- JSON standard vyžaduje escapované backslashes: `\\`
- Plugin používá `std::filesystem::path()` který správně převede `/` na `\` ve Windows
- Forward slashes fungují na **všech platformách** včetně Windows

### instrument-definition.json je invalid

**Symptom:**
- Plugin načte samply, ale používá fallback metadata
- V GUI se zobrazí název adresáře místo názvu nástroje

**Řešení:**

1. **Ověřit JSON syntax:**
   ```bash
   cat instrument-definition.json | python -m json.tool
   ```

2. **Zkontrolovat povinná pole:**
   ```json
   {
     "instrumentName": "VintageV Electric Piano",  ← POVINNÉ
     "velocityMaps": "8"                           ← POVINNÉ (string!)
   }
   ```

3. **Ověřit rozsah velocityMaps:**
   - Validní: `"1"`, `"2"`, ... `"8"` (string)
   - Invalid: `"0"`, `"9"`, `8` (number), `"osm"` (text)

### Permission denied při instalaci

**Windows:**
- VST3 do `Program Files` → **Vyžaduje admin práva**
- Installer musí požádat o UAC elevation

**macOS:**
- `/Applications` → Může vyžadovat heslo
- `~/Library` → User-level, bez admin

**Linux:**
- `/opt` → Vyžaduje sudo
- `~/.vst3` → User-level, bez sudo

---

## Dodatečné poznámky pro vývojáře installeru

### 1. Multi-platform handling

```python
import platform
import os

def get_platform_paths():
    system = platform.system()

    if system == "Windows":
        return {
            "plugin_vst3": "C:/Program Files/Common Files/VST3",
            "standalone": "C:/Program Files/LordAudio/IthacaPlayer",
            "sample_bank": "C:/SoundBanks/IthacaPlayer/instrument",
            "config": os.path.join(os.environ["APPDATA"], "LordAudio", "IthacaPlayer"),
            "platform_name": "Windows"
        }
    elif system == "Darwin":
        home = os.environ["HOME"]
        return {
            "plugin_vst3": f"{home}/Library/Audio/Plug-Ins/VST3",
            "plugin_au": f"{home}/Library/Audio/Plug-Ins/Components",
            "standalone": "/Applications",
            "sample_bank": f"{home}/Library/Application Support/IthacaPlayer/instrument",
            "config": f"{home}/Library/Application Support/LordAudio/IthacaPlayer",
            "platform_name": "Darwin"
        }
    else:  # Linux
        home = os.environ["HOME"]
        return {
            "plugin_vst3": f"{home}/.vst3",
            "standalone": "/opt/IthacaPlayer/bin",
            "sample_bank": f"{home}/.local/share/IthacaPlayer/instrument",
            "config": f"{home}/.local/share/LordAudio/IthacaPlayer",
            "platform_name": "Linux"
        }
```

### 2. JSON generation helper

```python
import json
from datetime import datetime

def create_samplebank_config(sample_bank_path, platform_name):
    """
    Vytvoří samplebank_config.json s correct formatting
    """
    # Opravit backslashes → forward slashes
    sample_bank_path_fixed = sample_bank_path.replace("\\", "/")

    config = {
        "sampleBankPath": sample_bank_path_fixed,
        "version": "1.0",
        "generatedBy": "Installer",
        "buildTimestamp": datetime.now().strftime("%y%m%d%H%M%S"),
        "platform": platform_name
    }

    return json.dumps(config, indent=2)
```

### 3. Validation helper

```python
def validate_installation(paths):
    """
    Ověří že všechny soubory byly správně nainstalovány
    """
    errors = []

    # 1. Plugin binary
    if not os.path.exists(paths["plugin_binary"]):
        errors.append(f"Plugin binary not found: {paths['plugin_binary']}")

    # 2. Standalone binary
    if not os.path.exists(paths["standalone_binary"]):
        errors.append(f"Standalone not found: {paths['standalone_binary']}")

    # 3. Sample bank
    instrument_json = os.path.join(paths["sample_bank"], "instrument-definition.json")
    if not os.path.exists(instrument_json):
        errors.append(f"instrument-definition.json not found: {instrument_json}")

    # 4. Runtime config (with PLUGIN_CODE)
    # Example: plugin_code = "VntV" for VintageV
    config_filename = f"samplebank-config-{paths['plugin_code']}.json"
    config_file = os.path.join(paths["config"], config_filename)
    if not os.path.exists(config_file):
        errors.append(f"Runtime config not found: {config_file}")
    else:
        # Validate JSON
        try:
            with open(config_file, 'r') as f:
                config = json.load(f)

            if "sampleBankPath" not in config:
                errors.append("Config missing 'sampleBankPath' field")

            # Check for backslashes (should use forward slashes)
            if "\\" in config.get("sampleBankPath", ""):
                errors.append("Config uses backslashes instead of forward slashes")
        except json.JSONDecodeError as e:
            errors.append(f"Config JSON syntax error: {e}")

    return errors
```

---

## Changelog

| Verze | Datum | Změny |
|-------|-------|-------|
| 2.1 | 2025-11-01 | Config filename pattern: `samplebank-config-{PLUGIN_CODE}.json` (konzistence s `instrument-definition.json`) |
| 2.0 | 2025-11-01 | Multiple Plugin Instances architecture - PLUGIN_CODE naming convention |
| 1.0 | 2025-11-01 | Initial version - kompletní dokumentace pro installer |

---

## Kontakt

Při problémech s instalací nebo konfigurací kontaktujte:
- **Email:** support@lordaudio.com
- **GitHub Issues:** https://github.com/alchy/Ithaca/issues

---

**© 2025 LordAudio - Ithaca Player**
