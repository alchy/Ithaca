/**
 * @file AppConstants.h
 * @brief Centralized application constants for Ithaca Plugin
 *
 * ALL constants in one place - consolidation of:
 * - GuiConstants.h
 * - MidiCCDefinitions.h
 * - ParameterDefaults.h
 * - Hardcoded values from various files
 *
 * Benefits:
 * - Single source of truth
 * - Semantic organization
 * - Easy to find and modify
 * - No duplication
 */

#pragma once

#include <juce_core/juce_core.h>
#include <cstdint>

namespace Constants {

    // ========================================================================
    // GUI - Layout & Dimensions
    // ========================================================================
    namespace Gui {
        namespace Window {
            constexpr int DEFAULT_WIDTH = 480;
            constexpr int DEFAULT_HEIGHT = 650;
        }

        namespace Layout {
            constexpr float INFO_SECTION_HEIGHT_RATIO = 0.20f;
            constexpr float SLIDER_SECTION_HEIGHT_RATIO = 0.80f;
            constexpr int SECTION_PADDING = 0;
            constexpr int SECTION_SPACING = 0;
            constexpr int SECTION_GAP = 0;
            constexpr int COLUMN_SPACING = 8;
        }

        namespace Overlay {
            constexpr float INFO_ALPHA = 0.8f;
            constexpr float SLIDER_ALPHA = 0.6f;
            constexpr float CORNER_RADIUS = 0.0f;
            constexpr float SEPARATOR_ALPHA = 0.6f;
            constexpr int SEPARATOR_THICKNESS = 2;
        }

        namespace Fonts {
            constexpr float TITLE_SIZE = 18.0f;      // Large (instrument name)
            constexpr float INFO_SIZE = 14.0f;       // Medium (labels, version)
            constexpr float SMALL_SIZE = 11.0f;      // Small (stats, details)
        }

        namespace Slider {
            constexpr int HEIGHT_HORIZONTAL = 25;
            constexpr int LABEL_HEIGHT = 18;
            constexpr int LABEL_SPACING = 4;
            constexpr int ROW_HEIGHT = LABEL_HEIGHT + LABEL_SPACING + HEIGHT_HORIZONTAL;
        }

        namespace InfoSection {
            constexpr int TITLE_HEIGHT = 28;
            constexpr int VERSION_HEIGHT = 20;
            constexpr int SAMPLE_RATE_HEIGHT = 18;
            constexpr int VOICE_STATS_HEIGHT = 18;
            constexpr int ROW_SPACING = 4;
        }

        namespace Colors {
            // Production mode (overlay on background)
            constexpr juce::uint32 SLIDER_TRACK = 0xff444444;
            constexpr juce::uint32 SLIDER_THUMB = 0xffffffff;
            constexpr juce::uint32 SLIDER_TEXT = 0xffffffff;
            constexpr juce::uint32 TEXT = 0xffffffff;
            constexpr juce::uint32 ACCENT = 0xff0066cc;

            // Debug mode
            constexpr juce::uint32 DEBUG_BG = 0xff808080;
            constexpr juce::uint32 DEBUG_TEXT = 0xff000000;
            constexpr juce::uint32 DEBUG_SLIDER_TRACK = 0xff333333;
            constexpr juce::uint32 DEBUG_SLIDER_THUMB = 0xff0066cc;

            // Voice visualization
            constexpr juce::uint32 ACTIVE_VOICE = 0xff00cc66;
            constexpr juce::uint32 RELEASING_VOICE = 0xffff6600;
        }

        namespace Timing {
            constexpr int INFO_UPDATE_INTERVAL_MS = 300;
            constexpr int MIDI_LEARN_FLASH_MS = 200;
        }

        namespace Text {
            // Slider labels
            constexpr const char* MASTER_GAIN_LABEL = "Master Gain";
            constexpr const char* MASTER_PAN_LABEL = "Master Pan";
            constexpr const char* ATTACK_LABEL = "Attack";
            constexpr const char* RELEASE_LABEL = "Release";
            constexpr const char* SUSTAIN_LABEL = "Sustain";
            constexpr const char* LFO_SPEED_LABEL = "LFO Speed";
            constexpr const char* LFO_DEPTH_LABEL = "LFO Depth";
            constexpr const char* STEREO_FIELD_LABEL = "Stereo Field";

            // Info section prefixes
            constexpr const char* ACTIVE_VOICES_PREFIX = "Active: ";
            constexpr const char* SUSTAINING_VOICES_PREFIX = "Sustaining: ";
            constexpr const char* SAMPLE_RATE_PREFIX = "Sample Rate: ";
            constexpr const char* VERSION_PREFIX = "Version: ";

            // Status messages
            constexpr const char* LOADING_TEXT = "Loading samples...";
            constexpr const char* ERROR_TEXT = "Sample load error";
            constexpr const char* FALLBACK_VALUE = "--";
        }
    }

    // ========================================================================
    // Audio - Processing & Performance
    // ========================================================================
    namespace Audio {
        namespace SampleRates {
            constexpr int RATE_44100 = 44100;
            constexpr int RATE_48000 = 48000;
            constexpr int RATE_96000 = 96000;
        }

        namespace BufferSizes {
            constexpr int MIN_BUFFER = 64;
            constexpr int DEFAULT_BUFFER = 512;
            constexpr int MAX_BUFFER = 2048;
        }
    }

    // ========================================================================
    // Performance Monitoring
    // ========================================================================
    namespace Performance {
        constexpr int MONITORING_WINDOW_SIZE = 100;  // samples for averaging

        namespace Thresholds {
            constexpr double CPU_OK = 0.50;         // < 50% = OK
            constexpr double CPU_WARNING = 0.80;    // 50-80% = warning
            constexpr double CPU_CRITICAL = 0.80;   // > 80% = critical
            constexpr double CPU_DROPOUT = 1.00;    // >= 100% = dropout
        }

        namespace Colors {
            constexpr juce::uint32 OK = 0xff90ee90;        // lightgreen
            constexpr juce::uint32 WARNING = 0xffffa500;   // orange
            constexpr juce::uint32 CRITICAL = 0xffff0000;  // red
        }
    }

    // ========================================================================
    // MIDI - Control Changes & Mappings
    // ========================================================================
    namespace Midi {
        namespace CC {
            // Standard MIDI CC
            constexpr uint8_t MODULATION = 1;
            constexpr uint8_t BREATH = 2;
            constexpr uint8_t VOLUME = 7;
            constexpr uint8_t PAN = 10;
            constexpr uint8_t EXPRESSION = 11;

            // Pedals
            constexpr uint8_t DAMPER_PEDAL = 64;    // Sustain
            constexpr uint8_t SOSTENUTO = 66;
            constexpr uint8_t SOFT_PEDAL = 67;

            // Sound Controllers (71-79)
            constexpr uint8_t SUSTAIN_LEVEL = 71;
            constexpr uint8_t RELEASE = 72;
            constexpr uint8_t ATTACK = 73;
            constexpr uint8_t LFO_PAN_SPEED = 74;
            constexpr uint8_t LFO_PAN_DEPTH = 75;
            constexpr uint8_t STEREO_FIELD = 76;

            // System Messages
            constexpr uint8_t ALL_SOUND_OFF = 120;
            constexpr uint8_t ALL_NOTES_OFF = 123;

            // Ithaca Default Mappings
            constexpr uint8_t MASTER_GAIN = VOLUME;
            constexpr uint8_t MASTER_PAN = PAN;
        }

        namespace Channels {
            constexpr int MIN = 1;
            constexpr int MAX = 16;
        }

        namespace Values {
            constexpr uint8_t MIN = 0;
            constexpr uint8_t MAX = 127;
            constexpr uint8_t CENTER = 64;  // For pan, pedals
        }

        namespace Processing {
            constexpr int MAX_MESSAGES_PER_BLOCK = 32;
            constexpr bool ENABLE_CC_PROCESSING = true;
            constexpr bool ENABLE_CC_LOGGING = false;
        }
    }

    // ========================================================================
    // Parameters - Defaults, Ranges, IDs
    // ========================================================================
    namespace Parameters {
        namespace IDs {
            constexpr const char* MASTER_GAIN = "masterGain";
            constexpr const char* MASTER_PAN = "masterPan";
            constexpr const char* ATTACK = "attack";
            constexpr const char* RELEASE = "release";
            constexpr const char* SUSTAIN_LEVEL = "sustainLevel";
            constexpr const char* LFO_PAN_SPEED = "lfoPanSpeed";
            constexpr const char* LFO_PAN_DEPTH = "lfoPanDepth";
            constexpr const char* STEREO_FIELD = "stereoField";
        }

        namespace Names {
            constexpr const char* MASTER_GAIN = "Master Gain";
            constexpr const char* MASTER_PAN = "Master Pan";
            constexpr const char* ATTACK = "Attack";
            constexpr const char* RELEASE = "Release";
            constexpr const char* SUSTAIN_LEVEL = "Sustain Level";
            constexpr const char* LFO_PAN_SPEED = "LFO Pan Speed";
            constexpr const char* LFO_PAN_DEPTH = "LFO Pan Depth";
            constexpr const char* STEREO_FIELD = "Stereo Field";
        }

        namespace Defaults {
            constexpr float MASTER_GAIN = 100.0f;   // MIDI value
            constexpr float MASTER_PAN = 64.0f;     // Center
            constexpr float ATTACK = 0.0f;
            constexpr float RELEASE = 4.0f;
            constexpr float SUSTAIN_LEVEL = 127.0f;
            constexpr float LFO_PAN_SPEED = 32.0f;
            constexpr float LFO_PAN_DEPTH = 127.0f;
            constexpr float STEREO_FIELD = 0.0f;
        }

        namespace Ranges {
            constexpr float MIDI_MIN = 0.0f;
            constexpr float MIDI_MAX = 127.0f;
            constexpr float MIDI_STEP = 1.0f;

            constexpr float MIN_GAIN = 0.0f;
            constexpr float MAX_GAIN = 1.0f;
            constexpr float MIN_TIME = 0.0f;
            constexpr float MAX_TIME = 10.0f;
        }
    }

    // ========================================================================
    // Files & Paths
    // ========================================================================
    namespace Files {
        constexpr const char* INSTRUMENT_METADATA = "instrument-definition.json";
        constexpr const char* LOG_DIRECTORY = "core_logger";
        constexpr const char* EXPORT_DIRECTORY = "exports";

        namespace Extensions {
            constexpr const char* WAV = ".wav";
            constexpr const char* JSON = ".json";
        }
    }

    // ========================================================================
    // Logging
    // ========================================================================
    namespace Logging {
        namespace Components {
            constexpr const char* PROCESSOR = "IthacaPluginProcessor";
            constexpr const char* ASYNC_LOADER = "AsyncSampleLoader";
            constexpr const char* PERF_MONITOR = "PerformanceMonitor";
            constexpr const char* STATE_MANAGER = "PluginStateManager";
            constexpr const char* MIDI_PROCESSOR = "MidiProcessor";
            constexpr const char* VOICE_MANAGER = "VoiceManager";
        }

        namespace Severity {
            constexpr const char* INFO = "info";
            constexpr const char* WARNING = "warning";
            constexpr const char* ERROR = "error";
            constexpr const char* DEBUG = "debug";
        }
    }

} // namespace Constants
