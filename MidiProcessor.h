/**
 * @file MidiProcessor.h (s MIDI Learn podporou a Sustain Pedal)
 * @brief MIDI event processing s MIDI Learn a Sustain Pedal support
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
 * @brief MIDI event processing s MIDI Learn podporou a Sustain Pedal
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
     * @brief Process MIDI buffer (s MIDI Learn a Sustain Pedal podporou)
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
    
    /**
     * @brief Process MIDI Control Change messages
     * @param ccNumber MIDI CC number (0-127)
     * @param ccValue MIDI CC value (0-127)
     * @param parameters Reference na APVTS
     * @param midiLearnManager Pointer na MidiLearnManager (nullable)
     */
    void processMidiControlChange(uint8_t ccNumber, 
                                  uint8_t ccValue,
                                  juce::AudioProcessorValueTreeState& parameters,
                                  MidiLearnManager* midiLearnManager);
    
    /**
     * @brief Process Sustain Pedal (CC64) messages
     * @param ccValue MIDI CC value (0-127, ≤63=off, ≥64=on)
     * @param voiceManager Pointer na VoiceManager
     * @note NOVĚ PŘIDÁNO: Separátní handling pro CC64 damper pedal
     * @note RT-safe: deleguje na VoiceManager::setSustainPedalMIDI()
     */
    void processSustainPedal(uint8_t ccValue, VoiceManager* voiceManager);
    
    /**
     * @brief Get parameter for given CC number
     * @param ccNumber MIDI CC number
     * @param parameters Reference na APVTS
     * @param midiLearnManager Pointer na MidiLearnManager (nullable)
     * @return Pointer na RangedAudioParameter nebo nullptr
     */
    juce::RangedAudioParameter* getParameterForCC(uint8_t ccNumber,
                                                  juce::AudioProcessorValueTreeState& parameters,
                                                  MidiLearnManager* midiLearnManager);
    
    // ========================================================================
    // State
    // ========================================================================
    
    std::atomic<int> totalMidiEventsProcessed_{0};
};