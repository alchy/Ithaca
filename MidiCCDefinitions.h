/**
 * @file MidiCCDefinitions.h
 * @brief Centralized MIDI Control Change definitions for IthacaPlayer
 */

#pragma once

#include <cstdint>
#include <string>

namespace MidiCC {
    
    // ===== STANDARD MIDI CC ASSIGNMENTS =====
    
    /**
     * @brief Standard MIDI CC numbers for IthacaPlayer parameters
     * 
     * Follows General MIDI standard where applicable:
     * - CC 7: Volume (0-127)
     * - CC 10: Pan (0=left, 64=center, 127=right)
     * - CC 71-79: Sound Controller range
     */
    
    // Master Controls (Standard MIDI)
    constexpr uint8_t MASTER_GAIN = 7;      // Standard Volume CC
    constexpr uint8_t MASTER_PAN = 10;      // Standard Pan CC
    
    // ADSR Envelope Controls (Sound Controller range)
    constexpr uint8_t ATTACK = 73;          // Sound Controller 4 (Attack Time)
    constexpr uint8_t RELEASE = 72;         // Sound Controller 3 (Release Time)
    constexpr uint8_t SUSTAIN_LEVEL = 71;   // Sound Controller 2 (Sustain Level)
    
    // LFO Controls (Extended Sound Controller range)
    constexpr uint8_t LFO_PAN_SPEED = 74;   // Sound Controller 5 (LFO Speed)
    constexpr uint8_t LFO_PAN_DEPTH = 75;   // Sound Controller 6 (LFO Depth)
    
    // ===== ALTERNATIVE CC ASSIGNMENTS =====
    
    /**
     * @brief Alternative CC assignments for different hardware controllers
     * Uncomment to use alternative mapping
     */
    namespace Alternative {
        // Korg NanoKontrol style mapping
        constexpr uint8_t MASTER_GAIN_ALT = 14;     // Slider 1
        constexpr uint8_t MASTER_PAN_ALT = 15;      // Slider 2
        constexpr uint8_t ATTACK_ALT = 16;          // Slider 3
        constexpr uint8_t RELEASE_ALT = 17;         // Slider 4
        constexpr uint8_t SUSTAIN_LEVEL_ALT = 18;   // Slider 5
        constexpr uint8_t LFO_PAN_SPEED_ALT = 19;   // Slider 6
        constexpr uint8_t LFO_PAN_DEPTH_ALT = 20;   // Slider 7
        
        // Behringer BCR2000 style mapping (rotary encoders)
        constexpr uint8_t MASTER_GAIN_BCR = 81;
        constexpr uint8_t MASTER_PAN_BCR = 82;
        constexpr uint8_t ATTACK_BCR = 83;
        constexpr uint8_t RELEASE_BCR = 84;
        constexpr uint8_t SUSTAIN_LEVEL_BCR = 85;
        constexpr uint8_t LFO_PAN_SPEED_BCR = 86;
        constexpr uint8_t LFO_PAN_DEPTH_BCR = 87;
    }
    
    // ===== PARAMETER MAPPING STRUCTURE =====
    
    /**
     * @brief Structure for MIDI CC to parameter mapping
     */
    struct CCMapping {
        uint8_t ccNumber;
        const char* parameterID;
        const char* displayName;
        const char* description;
    };
    
    /**
     * @brief Array of all CC mappings for easy iteration
     */
    constexpr CCMapping CC_MAPPINGS[] = {
        {MASTER_GAIN, "masterGain", "Master Gain", "Main volume control (CC 7)"},
        {MASTER_PAN, "masterPan", "Master Pan", "Stereo pan position (CC 10)"},
        {ATTACK, "attack", "Attack", "Envelope attack time (CC 73)"},
        {RELEASE, "release", "Release", "Envelope release time (CC 72)"},
        {SUSTAIN_LEVEL, "sustainLevel", "Sustain Level", "Envelope sustain level (CC 71)"},
        {LFO_PAN_SPEED, "lfoPanSpeed", "LFO Pan Speed", "Auto-pan LFO speed (CC 74)"},
        {LFO_PAN_DEPTH, "lfoPanDepth", "LFO Pan Depth", "Auto-pan LFO depth (CC 75)"}
    };
    
    /**
     * @brief Number of CC mappings
     */
    constexpr size_t NUM_CC_MAPPINGS = sizeof(CC_MAPPINGS) / sizeof(CCMapping);
    
    // ===== UTILITY FUNCTIONS =====
    
    /**
     * @brief Get parameter ID for given CC number
     * @param ccNumber MIDI CC number
     * @return Parameter ID string or nullptr if not found
     */
    inline const char* getParameterIDForCC(uint8_t ccNumber) {
        for (const auto& mapping : CC_MAPPINGS) {
            if (mapping.ccNumber == ccNumber) {
                return mapping.parameterID;
            }
        }
        return nullptr;
    }
    
    /**
     * @brief Get display name for given CC number
     * @param ccNumber MIDI CC number
     * @return Display name or nullptr if not found
     */
    inline const char* getDisplayNameForCC(uint8_t ccNumber) {
        for (const auto& mapping : CC_MAPPINGS) {
            if (mapping.ccNumber == ccNumber) {
                return mapping.displayName;
            }
        }
        return nullptr;
    }
    
    /**
     * @brief Check if CC number is mapped to a parameter
     * @param ccNumber MIDI CC number to check
     * @return true if CC is mapped
     */
    inline bool isCCMapped(uint8_t ccNumber) {
        return getParameterIDForCC(ccNumber) != nullptr;
    }
    
    // ===== MIDI VALUE CONVERSION =====
    
    /**
     * @brief Convert MIDI CC value (0-127) to normalized parameter value (0.0-1.0)
     * @param ccValue MIDI CC value (0-127)
     * @return Normalized value (0.0-1.0)
     */
    inline float ccValueToNormalized(uint8_t ccValue) {
        return ccValue / 127.0f;
    }
    
    /**
     * @brief Convert MIDI pan CC value to GUI pan range
     * @param ccValue MIDI pan value (0-127, where 64=center)
     * @return Normalized pan value for GUI (-64 to +63 range)
     */
    inline float ccPanToNormalized(uint8_t ccValue) {
        // MIDI: 0=full left, 64=center, 127=full right
        // GUI: -64=full left, 0=center, +63=full right
        float panValue = static_cast<float>(ccValue) - 64.0f; // Convert to -64 to +63
        return (panValue + 64.0f) / 127.0f; // Normalize to 0-1
    }
    
    /**
     * @brief Convert normalized parameter value to MIDI CC value
     * @param normalizedValue Parameter value (0.0-1.0)
     * @return MIDI CC value (0-127)
     */
    inline uint8_t normalizedToCCValue(float normalizedValue) {
        return static_cast<uint8_t>(normalizedValue * 127.0f + 0.5f);
    }
    
} // namespace MidiCC

// ===== CONFIGURATION SWITCHES =====

/**
 * @brief Enable/disable MIDI CC processing
 * Set to 0 to disable all MIDI CC processing for performance
 */
#ifndef ENABLE_MIDI_CC_PROCESSING
#define ENABLE_MIDI_CC_PROCESSING 1
#endif

/**
 * @brief Enable/disable MIDI CC logging for debugging
 * Set to 1 to log all received CC messages (non-RT context only)
 */
#ifndef ENABLE_MIDI_CC_LOGGING
#define ENABLE_MIDI_CC_LOGGING 0
#endif

/**
 * @brief Maximum number of CC messages to process per audio block
 * Prevents CC flooding from causing audio dropouts
 */
#ifndef MAX_CC_MESSAGES_PER_BLOCK
#define MAX_CC_MESSAGES_PER_BLOCK 32
#endif