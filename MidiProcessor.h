/**
 * @file MidiProcessor.h (s MIDI Learn podporou)
 * @brief MIDI event processing s MIDI Learn
 */

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>
#include <cstdint>

// Forward declarations
class VoiceManager;
class MidiLearnManager;

/**
 * @class MidiProcessor
 * @brief MIDI event processing s MIDI Learn podporou
 */
class MidiProcessor {
public:
    MidiProcessor();
    ~MidiProcessor() = default;
    
    // Disable copy/move
    MidiProcessor(const MidiProcessor&) = delete;
    MidiProcessor& operator=(const MidiProcessor&) = delete;
    
    // ========================================================================
    // Main MIDI Processing
    // ========================================================================
    
    /**
     * @brief Process MIDI buffer (s MIDI Learn podporou)
     * @param midiMessages JUCE MIDI buffer
     * @param voiceManager Pointer na VoiceManager
     * @param parameters Reference na APVTS
     * @param midiLearnManager Pointer na MidiLearnManager (nullable)
     */
    void processMidiBuffer(const juce::MidiBuffer& midiMessages,
                          VoiceManager* voiceManager,
                          juce::AudioProcessorValueTreeState& parameters,
                          MidiLearnManager* midiLearnManager = nullptr);
    
    // ========================================================================
    // Statistics
    // ========================================================================
    
    int getTotalMidiEventsProcessed() const;
    void resetStatistics();
    
private:
    // ========================================================================
    // Processing Methods
    // ========================================================================
    
    void processMidiControlChange(uint8_t ccNumber, 
                                  uint8_t ccValue,
                                  juce::AudioProcessorValueTreeState& parameters,
                                  MidiLearnManager* midiLearnManager);
    
    juce::RangedAudioParameter* getParameterForCC(uint8_t ccNumber,
                                                  juce::AudioProcessorValueTreeState& parameters,
                                                  MidiLearnManager* midiLearnManager);
    
    // ========================================================================
    // State
    // ========================================================================
    
    std::atomic<int> totalMidiEventsProcessed_{0};
};