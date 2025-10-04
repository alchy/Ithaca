/**
 * @file PluginStateManager.h
 * @brief Centralized plugin state save/load management
 *
 * Handles XML serialization/deserialization of:
 * - AudioProcessor parameters (APVTS)
 * - MIDI Learn mappings
 * - Future: sample directory, user preferences, etc.
 */

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <functional>
#include "ithaca-core/sampler/core_logger.h"

// Forward declarations
class MidiLearnManager;

/**
 * @class PluginStateManager
 * @brief Static utility class for plugin state management
 *
 * Features:
 * - XML-based serialization
 * - Parameter state save/load
 * - MIDI Learn mappings save/load
 * - Legacy format compatibility
 * - Logging callback support
 */
class PluginStateManager {
public:
    /**
     * @brief Logging callback type
     * @param component Component name for logging
     * @param severity Log severity (LogSeverity enum)
     * @param message Log message
     */
    using LogCallback = std::function<void(const std::string&, LogSeverity, const std::string&)>;

    /**
     * @brief Save plugin state to binary data
     * @param destData Destination memory block for binary data
     * @param parameters APVTS containing all parameters
     * @param midiLearnManager Optional MIDI Learn manager (can be nullptr)
     * @param logCallback Optional logging callback
     */
    static void saveState(juce::MemoryBlock& destData,
                         juce::AudioProcessorValueTreeState& parameters,
                         MidiLearnManager* midiLearnManager = nullptr,
                         LogCallback logCallback = nullptr);

    /**
     * @brief Load plugin state from binary data
     * @param data Source binary data
     * @param sizeInBytes Size of binary data
     * @param parameters APVTS to restore parameters into
     * @param midiLearnManager Optional MIDI Learn manager (can be nullptr)
     * @param logCallback Optional logging callback
     * @return true if state was loaded successfully
     */
    static bool loadState(const void* data,
                         int sizeInBytes,
                         juce::AudioProcessorValueTreeState& parameters,
                         MidiLearnManager* midiLearnManager = nullptr,
                         LogCallback logCallback = nullptr);

private:
    /**
     * @brief Create root XML element with all state data
     * @param parameters APVTS containing parameters
     * @param midiLearnManager MIDI Learn manager (can be nullptr)
     * @return Unique pointer to root XML element
     */
    static std::unique_ptr<juce::XmlElement> createStateXml(
        juce::AudioProcessorValueTreeState& parameters,
        MidiLearnManager* midiLearnManager);

    /**
     * @brief Restore state from XML element
     * @param xmlState Root XML element
     * @param parameters APVTS to restore into
     * @param midiLearnManager MIDI Learn manager (can be nullptr)
     * @param logCallback Optional logging callback
     * @return true if restoration was successful
     */
    static bool restoreFromXml(juce::XmlElement* xmlState,
                               juce::AudioProcessorValueTreeState& parameters,
                               MidiLearnManager* midiLearnManager,
                               LogCallback logCallback);

    /**
     * @brief Check if XML is in new format (with MIDI Learn)
     * @param xmlState XML element to check
     * @return true if new format detected
     */
    static bool isNewFormat(const juce::XmlElement* xmlState);

    /**
     * @brief Check if XML is in legacy format (parameters only)
     * @param xmlState XML element to check
     * @param parameters APVTS for type checking
     * @return true if legacy format detected
     */
    static bool isLegacyFormat(const juce::XmlElement* xmlState,
                              const juce::AudioProcessorValueTreeState& parameters);

    // Root XML tag name
    static constexpr const char* ROOT_TAG = "IthacaPluginState";
    static constexpr const char* MIDI_LEARN_TAG = "MidiLearnMappings";
};
