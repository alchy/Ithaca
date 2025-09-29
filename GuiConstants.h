/**
 * @file GuiConstants.h
 * @brief GUI konstanty pro IthacaPluginEditor refactoring
 */

#pragma once

#include <juce_core/juce_core.h>

namespace GuiConstants {
    // Window sizes
    constexpr int DEFAULT_WINDOW_WIDTH = 480;
    constexpr int DEFAULT_WINDOW_HEIGHT = 650;
    
    // Horizontal slider dimensions - ZMĚNA z vertikálních
    constexpr int SLIDER_HEIGHT_HORIZONTAL = 25;  // Sníženo z 60px
    constexpr int SLIDER_WIDTH_STANDARD = 120;
    constexpr int SLIDER_TEXT_BOX_WIDTH = 40;
    constexpr int SLIDER_TEXT_BOX_HEIGHT = 16;
    
    // Layout spacing
    constexpr int COMPONENT_PADDING = 8;
    constexpr int SLIDER_SPACING = 4;
    constexpr int LABEL_HEIGHT = 16;
    constexpr int LABEL_SPACING = 2;
    
    // Colors - ZACHOVAT původní hodnoty
    constexpr juce::uint32 SLIDER_TRACK_COLOR = 0xff444444;
    constexpr juce::uint32 SLIDER_THUMB_COLOR = 0xff0066cc;
    constexpr juce::uint32 SLIDER_TEXT_COLOR = 0xffffffff;
    constexpr juce::uint32 BG_COLOR = 0xffffffff;
    constexpr juce::uint32 TEXT_COLOR = 0xff333333;
    constexpr juce::uint32 ACCENT_COLOR = 0xff0066cc;
    constexpr juce::uint32 BORDER_COLOR = 0xffcccccc;
    
    // Debug mode colors
    constexpr juce::uint32 DEBUG_BG_COLOR = 0xff808080;
    constexpr juce::uint32 DEBUG_TEXT_COLOR = 0xff000000;
    constexpr juce::uint32 DEBUG_SLIDER_TRACK = 0xff333333;
    constexpr juce::uint32 DEBUG_SLIDER_THUMB = 0xff0066cc;
    
    // Control area dimensions
    constexpr int CONTROL_AREA_HEIGHT = 350;
    constexpr int SLIDERS_AREA_WIDTH = 320;
    constexpr int INFO_AREA_MIN_WIDTH = 130;
    
    // Text constants
    namespace TextConstants {
        constexpr const char* MASTER_GAIN_LABEL = "Master Gain";
        constexpr const char* MASTER_PAN_LABEL = "Master Pan";
        constexpr const char* ATTACK_LABEL = "Attack";
        constexpr const char* RELEASE_LABEL = "Release";
        constexpr const char* SUSTAIN_LABEL = "Sustain";
        constexpr const char* LFO_SPEED_LABEL = "LFO Speed";
        constexpr const char* LFO_DEPTH_LABEL = "LFO Depth";
        constexpr const char* STEREO_FIELD_LABEL = "Stereo Field";
        
        constexpr const char* ACTIVE_VOICES_PREFIX = "Active: ";
        constexpr const char* SUSTAINING_VOICES_PREFIX = "Sustaining: ";
        constexpr const char* RELEASING_VOICES_PREFIX = "Releasing: ";
        constexpr const char* SAMPLE_RATE_PREFIX = "Rate: ";
        constexpr const char* INSTRUMENT_PREFIX = "Instrument: ";
        constexpr const char* VERSION_PREFIX = "Version: ";
        
        constexpr const char* ERROR_VALUE = "ERR";
        constexpr const char* FALLBACK_VALUE = "--";
    }
    
    // Font sizes
    constexpr float LABEL_FONT_SIZE = 11.0f;
    constexpr float SMALL_LABEL_FONT_SIZE = 10.0f;
    constexpr float INFO_LABEL_FONT_SIZE = 14.0f;
    constexpr float TITLE_FONT_SIZE = 18.0f;
}