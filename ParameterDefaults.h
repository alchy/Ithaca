#ifndef PARAMETER_DEFAULTS_H
#define PARAMETER_DEFAULTS_H

//==============================================================================
// ITHACA PLUGIN PARAMETER DEFAULTS
// Konfigurace výchozích hodnot, rozsahů, ID a názvů parametrů pro plugin
//==============================================================================

// === VÝCHOZÍ HODNOTY ===
// Definují výchozí hodnoty pro MIDI parametry (0-127)
#define DEFAULT_MASTER_GAIN         100  // Výchozí hlasitost (100 z 127)
#define DEFAULT_MASTER_PAN          64   // Výchozí panorama (střed)
#define DEFAULT_ATTACK              0    // Výchozí rychlost náběhu (attack)
#define DEFAULT_RELEASE             4    // Výchozí rychlost dozvuku (release)
#define DEFAULT_SUSTAIN_LEVEL       127  // Výchozí úroveň udržení (sustain)
#define DEFAULT_LFO_PAN_SPEED       32   // Výchozí rychlost LFO pro panorama
#define DEFAULT_LFO_PAN_DEPTH       127  // Výchozí hloubka LFO pro panorama
#define DEFAULT_STEREO_FIELD        0    // Výchozí stereo field (vypnuto)

// === ROZSAHY PARAMETRŮ ===
// Definují rozsahy hodnot pro MIDI parametry
#define MIDI_MIN_VALUE              0.0f   // Minimální hodnota MIDI
#define MIDI_MAX_VALUE              127.0f // Maximální hodnota MIDI
#define MIDI_STEP_VALUE             1.0f   // Krok změny hodnoty MIDI

// === ID PARAMETRŮ ===
// Jedinečná ID pro propojení s JUCE AudioProcessorValueTreeState
#define PARAM_ID_MASTER_GAIN        "masterGain"
#define PARAM_ID_MASTER_PAN         "masterPan"
#define PARAM_ID_ATTACK             "attack"
#define PARAM_ID_RELEASE            "release"
#define PARAM_ID_SUSTAIN_LEVEL      "sustainLevel"
#define PARAM_ID_LFO_PAN_SPEED      "lfoPanSpeed"
#define PARAM_ID_LFO_PAN_DEPTH      "lfoPanDepth"
#define PARAM_ID_STEREO_FIELD       "stereoField"

// === NÁZVY PARAMETRŮ ===
// Uživatelsky přívětivé názvy pro GUI
#define PARAM_NAME_MASTER_GAIN      "Master Gain"
#define PARAM_NAME_MASTER_PAN       "Master Pan"
#define PARAM_NAME_ATTACK           "Attack"
#define PARAM_NAME_RELEASE          "Release"
#define PARAM_NAME_SUSTAIN_LEVEL    "Sustain Level"
#define PARAM_NAME_LFO_PAN_SPEED    "LFO Pan Speed"
#define PARAM_NAME_LFO_PAN_DEPTH    "LFO Pan Depth"
#define PARAM_NAME_STEREO_FIELD     "Stereo Field"

// === MAPOVÁNÍ MIDI CC ===
// MIDI Control Change čísla pro ovládání parametrů
#define CC_MASTER_GAIN              7   // Standardní MIDI CC pro hlasitost
#define CC_MASTER_PAN               10  // Standardní MIDI CC pro panorama
#define CC_ATTACK                   73  // MIDI CC pro rychlost náběhu
#define CC_RELEASE                  72  // MIDI CC pro rychlost dozvuku
#define CC_SUSTAIN_LEVEL            71  // MIDI CC pro úroveň udržení
#define CC_LFO_PAN_SPEED            20  // Vlastní MIDI CC pro rychlost LFO
#define CC_LFO_PAN_DEPTH            21  // Vlastní MIDI CC pro hloubku LFO
#define CC_ALL_SOUND_OFF            120 // MIDI CC pro vypnutí všech zvuků
#define CC_ALL_NOTES_OFF            123 // MIDI CC pro vypnutí všech not
#define CC_STEREO_FIELD             76  // MIDI CC pro stereo field (Sound Controller 7)

// === ROZMÍSTĚNÍ GUI ===
// Rozměry a mezery pro GUI komponenty (slidery a labely)
#define SLIDER_HEIGHT               25  // Výška slideru v pixelech
#define LABEL_HEIGHT                16  // Výška labelu v pixelech
#define SLIDER_SPACING              8   // Mezera mezi slidery
#define LABEL_SPACING               2   // Mezera mezi labelem a sliderem

// === BARVY ===
// Barevné schéma pro GUI komponenty
#define SLIDER_TRACK_COLOR          0xff444444 // Barva dráhy slideru
#define SLIDER_THUMB_COLOR          0xff0066cc // Barva táhla slideru
#define SLIDER_TEXT_COLOR           0xffffffff // Barva textu slideru
#define ACTIVE_VOICE_COLOR          0xff00cc66 // Barva aktivních hlasů v mřížce
#define RELEASING_VOICE_COLOR       0xffff6600 // Barva uvolňujících se hlasů v mřížce

#endif // PARAMETER_DEFAULTS_H