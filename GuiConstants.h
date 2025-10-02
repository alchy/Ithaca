/**
 * @file GuiConstants.h
 * @brief GUI konstanty pro IthacaPluginEditor
 * 
 * ============================================================================
 * GUI REFACTORING - HIERARCHICAL LAYOUT
 * ============================================================================
 * 
 * Změny v této verzi:
 * - Hierarchický layout (30% info header / 70% slider panel)
 * - Font hierarchy (18px title / 14px medium / 11px small)
 * - Rounded overlays (6px radius, 80%/60% alpha)
 * - Transparent separators (60% alpha)
 * - 50/50 column layout pro všechny slider řádky
 * - Odstraněn veškerý legacy kód
 * 
 * Layout struktura:
 * ┌──────────────────────────────────────────┐
 * │ ▓▓ INFO HEADER (30%, 80% alpha)        ▓▓ │
 * │ ▓▓ - Ithaca Grand Piano (18px bold)    ▓▓ │
 * │ ▓▓ - Version: 1.0.0 (14px)             ▓▓ │
 * │ ▓▓ - Sample Rate: 48000 Hz (11px)      ▓▓ │
 * │ ▓▓ - Active: 5 | Sustaining: 2 (11px)  ▓▓ │
 * ├──────────────────────────────────────────┤
 * │ ▒▒ SLIDER PANEL (70%, 60% alpha)       ▒▒ │
 * │ ▒▒ Row 1: Master Gain | Stereo Field   ▒▒ │
 * │ ▒▒ Row 2: LFO Depth | LFO Speed        ▒▒ │
 * │ ▒▒ Row 3: Attack | Release             ▒▒ │
 * │ ▒▒ Row 4: Sustain | Master Pan         ▒▒ │
 * └──────────────────────────────────────────┘
 * ============================================================================
 */

#pragma once

#include <juce_core/juce_core.h>

namespace GuiConstants {
    
    // =========================================================================
    // WINDOW DIMENSIONS
    // =========================================================================
    
    constexpr int DEFAULT_WINDOW_WIDTH = 480;
    constexpr int DEFAULT_WINDOW_HEIGHT = 650;
    
    // =========================================================================
    // LAYOUT PROPORTIONS - Hierarchický layout
    // =========================================================================
    
    /// Info header výška (~30% okna)
    constexpr float INFO_SECTION_HEIGHT_RATIO = 0.30f;
    
    /// Slider panel výška (~70% okna)
    constexpr float SLIDER_SECTION_HEIGHT_RATIO = 0.70f;
    
    /// Vnitřní padding sekcí (okraje)
    constexpr int SECTION_PADDING = 12;
    
    /// Mezera mezi řádky sliderů
    constexpr int SECTION_SPACING = 8;
    
    /// Mezera mezi info a slider sekcí
    constexpr int SECTION_GAP = 8;
    
    // =========================================================================
    // OVERLAY STYLING - Rounded corners a alpha
    // =========================================================================
    
    /// Alpha pro info header (tmavší overlay)
    constexpr float INFO_OVERLAY_ALPHA = 0.8f;
    
    /// Alpha pro slider panel (průhlednější overlay)
    constexpr float SLIDER_OVERLAY_ALPHA = 0.6f;
    
    /// Poloměr zaoblení rohů overlay panelů
    constexpr float PANEL_CORNER_RADIUS = 6.0f;
    
    /// Alpha pro separátory mezi slider řádky
    constexpr float SEPARATOR_ALPHA = 0.6f;
    
    /// Tloušťka separační čáry
    constexpr int SEPARATOR_THICKNESS = 1;
    
    // =========================================================================
    // FONT SIZES - Hierarchie fontů
    // =========================================================================
    
    /// Velký font pro instrument name (18px, bold)
    constexpr float TITLE_FONT_SIZE = 18.0f;
    
    /// Střední font pro slider labels a version info (14px)
    constexpr float INFO_LABEL_FONT_SIZE = 14.0f;
    
    /// Malý font pro detail info a statistiky (11px)
    constexpr float SMALL_LABEL_FONT_SIZE = 11.0f;
    
    // =========================================================================
    // SLIDER DIMENSIONS - Horizontální layout
    // =========================================================================
    
    /// Výška horizontálního slideru
    constexpr int SLIDER_HEIGHT_HORIZONTAL = 25;
    
    /// Výška labelu nad sliderem
    constexpr int SLIDER_LABEL_HEIGHT = 18;
    
    /// Mezera mezi labelem a sliderem
    constexpr int SLIDER_LABEL_SPACING = 4;
    
    /// Celková výška slider řádku (label + spacing + slider)
    constexpr int SLIDER_ROW_HEIGHT = SLIDER_LABEL_HEIGHT + 
                                     SLIDER_LABEL_SPACING + 
                                     SLIDER_HEIGHT_HORIZONTAL;
    
    /// Mezera mezi levým a pravým sloupcem (50/50 split)
    constexpr int COLUMN_SPACING = 8;
    
    // =========================================================================
    // INFO SECTION LAYOUT - Výšky jednotlivých řádků
    // =========================================================================
    
    /// Výška řádku s instrument name
    constexpr int INFO_TITLE_HEIGHT = 28;
    
    /// Výška řádku s version
    constexpr int INFO_VERSION_HEIGHT = 20;
    
    /// Výška řádku s sample rate
    constexpr int INFO_SAMPLE_RATE_HEIGHT = 18;
    
    /// Výška řádku s voice statistics
    constexpr int INFO_VOICE_STATS_HEIGHT = 18;
    
    /// Mezera mezi řádky v info sekci
    constexpr int INFO_ROW_SPACING = 4;
    
    // =========================================================================
    // COLORS - Background mode (overlay na background image)
    // =========================================================================
    
    constexpr juce::uint32 SLIDER_TRACK_COLOR = 0xff444444;
    constexpr juce::uint32 SLIDER_THUMB_COLOR = 0xff0066cc;
    constexpr juce::uint32 SLIDER_TEXT_COLOR = 0xffffffff;
    
    constexpr juce::uint32 TEXT_COLOR = 0xffffffff;        // Bílý text
    constexpr juce::uint32 ACCENT_COLOR = 0xff0066cc;      // Modrý accent
    
    // =========================================================================
    // COLORS - Debug mode (BACKGROUND_PICTURE_OFF)
    // =========================================================================
    
    constexpr juce::uint32 DEBUG_BG_COLOR = 0xff808080;
    constexpr juce::uint32 DEBUG_TEXT_COLOR = 0xff000000;
    constexpr juce::uint32 DEBUG_SLIDER_TRACK = 0xff333333;
    constexpr juce::uint32 DEBUG_SLIDER_THUMB = 0xff0066cc;
    
    // =========================================================================
    // TEXT CONSTANTS - Všechny textové řetězce pro GUI
    // =========================================================================
    
    namespace TextConstants {
        // Slider labels (střední font)
        constexpr const char* MASTER_GAIN_LABEL = "Master Gain";
        constexpr const char* MASTER_PAN_LABEL = "Master Pan";
        constexpr const char* ATTACK_LABEL = "Attack";
        constexpr const char* RELEASE_LABEL = "Release";
        constexpr const char* SUSTAIN_LABEL = "Sustain";
        constexpr const char* LFO_SPEED_LABEL = "LFO Speed";
        constexpr const char* LFO_DEPTH_LABEL = "LFO Depth";
        constexpr const char* STEREO_FIELD_LABEL = "Stereo Field";
        
        // Info section prefixy
        constexpr const char* ACTIVE_VOICES_PREFIX = "Active: ";
        constexpr const char* SUSTAINING_VOICES_PREFIX = "Sustaining: ";
        constexpr const char* SAMPLE_RATE_PREFIX = "Sample Rate: ";
        constexpr const char* VERSION_PREFIX = "Version: ";
        
        // Status messages (async loading support)
        constexpr const char* LOADING_TEXT = "Loading samples...";
        constexpr const char* ERROR_TEXT = "Sample load error";
        constexpr const char* FALLBACK_VALUE = "--";
    }
    
} // namespace GuiConstants