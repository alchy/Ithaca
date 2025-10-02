/**
 * @file MidiProcessor.h
 * @brief Dedicated MIDI event processing for IthacaPlayer
 * 
 * Separates MIDI processing logic from IthacaPluginProcessor to improve
 * code organization and testability.
 */

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>
#include <cstdint>

// Forward declarations
class VoiceManager;

/**
 * @class MidiProcessor
 * @brief Handles all MIDI event processing for the plugin
 * 
 * Responsibilities:
 * - Process MIDI note on/off events
 * - Process MIDI Control Change messages
 * - Map MIDI CC to plugin parameters
 * - Track MIDI event statistics
 * 
 * Thread Safety:
 * - All methods are RT-safe (designed for audio thread)
 * - No allocations in RT methods
 * - Uses atomic counters for statistics
 */
class MidiProcessor {
public:
    /**
     * @brief Constructor
     */
    MidiProcessor();
    
    /**
     * @brief Destructor
     */
    ~MidiProcessor() = default;
    
    // Disable copy/move
    MidiProcessor(const MidiProcessor&) = delete;
    MidiProcessor& operator=(const MidiProcessor&) = delete;
    MidiProcessor(MidiProcessor&&) = delete;
    MidiProcessor& operator=(MidiProcessor&&) = delete;
    
    //==========================================================================
    // Main MIDI Processing Interface
    
    /**
     * @brief Process all MIDI events in buffer
     * @param midiMessages JUCE MIDI buffer from processBlock
     * @param voiceManager Pointer to VoiceManager for note events
     * @param parameters Reference to APVTS for CC parameter updates
     * 
     * Processes:
     * - Note On events → VoiceManager::setNoteStateMIDI()
     * - Note Off events → VoiceManager::setNoteStateMIDI()
     * - Control Change events → parameter updates
     * 
     * @note RT-safe: no allocations, no blocking operations
     */
    void processMidiBuffer(const juce::MidiBuffer& midiMessages,
                          VoiceManager* voiceManager,
                          juce::AudioProcessorValueTreeState& parameters);
    
    //==========================================================================
    // Statistics
    
    /**
     * @brief Get total number of MIDI events processed
     * @return Total event count since plugin start
     * @note Thread-safe
     */
    int getTotalMidiEventsProcessed() const;
    
    /**
     * @brief Reset MIDI event counter
     * @note Thread-safe
     */
    void resetStatistics();
    
private:
    //==========================================================================
    // Private Processing Methods
    
    /**
     * @brief Process individual MIDI Control Change message
     * @param ccNumber MIDI CC number (0-127)
     * @param ccValue MIDI CC value (0-127)
     * @param parameters Reference to APVTS for parameter updates
     * 
     * Maps MIDI CC to plugin parameters using MidiCCDefinitions.h
     * Updates parameter values and notifies host.
     * 
     * @note RT-safe: direct parameter updates only
     */
    void processMidiControlChange(uint8_t ccNumber, 
                                  uint8_t ccValue,
                                  juce::AudioProcessorValueTreeState& parameters);
    
    /**
     * @brief Get JUCE parameter for given MIDI CC number
     * @param ccNumber MIDI CC number
     * @param parameters APVTS to search
     * @return Pointer to parameter or nullptr if not mapped
     * 
     * Uses MidiCCDefinitions to map CC numbers to parameter IDs.
     * 
     * @note RT-safe: no allocations
     */
    juce::RangedAudioParameter* getParameterForCC(uint8_t ccNumber,
                                                  juce::AudioProcessorValueTreeState& parameters);
    
    //==========================================================================
    // Statistics
    
    std::atomic<int> totalMidiEventsProcessed_{0};  ///< Total MIDI events counter
};