/**
 * @file MidiHelpers.h
 * @brief MIDI utility functions and helpers
 */

#pragma once

#include "ithaca/config/AppConstants.h"
#include <cstdint>

namespace MidiHelpers {

    /**
     * @brief Check if CC number is Damper Pedal (CC64)
     */
    inline bool isDamperPedal(uint8_t ccNumber) {
        return ccNumber == Constants::Midi::CC::DAMPER_PEDAL;
    }

    /**
     * @brief Convert MIDI CC value (0-127) to normalized parameter value (0.0-1.0)
     */
    inline float ccValueToNormalized(uint8_t ccValue) {
        return ccValue / 127.0f;
    }

    /**
     * @brief Convert MIDI pan CC value to GUI pan range
     * MIDI: 0=full left, 64=center, 127=full right
     * GUI: -64=full left, 0=center, +63=full right
     */
    inline float ccPanToNormalized(uint8_t ccValue) {
        float panValue = static_cast<float>(ccValue) - 64.0f;
        return (panValue + 64.0f) / 127.0f;
    }

    /**
     * @brief Convert MIDI damper pedal value to boolean state
     * @return true if pedal is down (≥64), false if up (≤63)
     */
    inline bool ccValueToPedalState(uint8_t ccValue) {
        return ccValue >= Constants::Midi::Values::CENTER;
    }

    /**
     * @brief Get parameter ID for given CC number
     * @return Parameter ID string or nullptr if not found
     */
    inline const char* getParameterIDForCC(uint8_t ccNumber) {
        // Map CC numbers to parameter IDs
        switch (ccNumber) {
            case Constants::Midi::CC::MASTER_GAIN:
                return Constants::Parameters::IDs::MASTER_GAIN;
            case Constants::Midi::CC::MASTER_PAN:
                return Constants::Parameters::IDs::MASTER_PAN;
            case Constants::Midi::CC::ATTACK:
                return Constants::Parameters::IDs::ATTACK;
            case Constants::Midi::CC::RELEASE:
                return Constants::Parameters::IDs::RELEASE;
            case Constants::Midi::CC::SUSTAIN_LEVEL:
                return Constants::Parameters::IDs::SUSTAIN_LEVEL;
            case Constants::Midi::CC::LFO_PAN_SPEED:
                return Constants::Parameters::IDs::LFO_PAN_SPEED;
            case Constants::Midi::CC::LFO_PAN_DEPTH:
                return Constants::Parameters::IDs::LFO_PAN_DEPTH;
            case Constants::Midi::CC::STEREO_FIELD:
                return Constants::Parameters::IDs::STEREO_FIELD;
            default:
                return nullptr;
        }
    }

} // namespace MidiHelpers
