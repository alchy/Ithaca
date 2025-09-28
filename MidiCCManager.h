#ifndef MIDI_CC_MANAGER_H
#define MIDI_CC_MANAGER_H

#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>
#include "ParameterDefaults.h"

// Forward declarations
class VoiceManager;

//==============================================================================
/**
 * @class MidiCCManager
 * @brief Handles MIDI Control Change messages for VoiceManager parameters
 */
class MidiCCManager
{
public:
    //==============================================================================
    /**
     * @brief Constructor
     * @param parameters Reference to JUCE parameter tree
     */
    explicit MidiCCManager(juce::AudioProcessorValueTreeState& parameters);
    
    /**
     * @brief Process single MIDI CC message
     * @param voiceManager Pointer to VoiceManager (can be nullptr)
     * @param channel MIDI channel (0-15)
     * @param controller CC number (0-127)
     * @param value CC value (0-127)
     */
    void processMidiControlChange(VoiceManager* voiceManager, 
                                 uint8_t channel, 
                                 uint8_t controller, 
                                 uint8_t value) noexcept;
    
    /**
     * @brief Set parameter pointers for direct access
     */
    void setParameterPointers(std::atomic<float>* masterGain,
                             std::atomic<float>* masterPan,
                             std::atomic<float>* attack,
                             std::atomic<float>* release,
                             std::atomic<float>* sustainLevel,
                             std::atomic<float>* lfoPanSpeed,
                             std::atomic<float>* lfoPanDepth);

private:
    //==============================================================================
    juce::AudioProcessorValueTreeState& parameters_;
    
    // Parameter pointers for RT-safe access
    std::atomic<float>* masterGainParam_;
    std::atomic<float>* masterPanParam_;
    std::atomic<float>* attackParam_;
    std::atomic<float>* releaseParam_;
    std::atomic<float>* sustainLevelParam_;
    std::atomic<float>* lfoPanSpeedParam_;
    std::atomic<float>* lfoPanDepthParam_;
    
    //==============================================================================
    // Helper methods for specific CC handling
    void handleMasterGainCC(uint8_t value) noexcept;
    void handleMasterPanCC(VoiceManager* voiceManager, uint8_t value) noexcept;
    void handleAttackCC(VoiceManager* voiceManager, uint8_t value) noexcept;
    void handleReleaseCC(VoiceManager* voiceManager, uint8_t value) noexcept;
    void handleSustainLevelCC(VoiceManager* voiceManager, uint8_t value) noexcept;
    void handleLfoPanSpeedCC(VoiceManager* voiceManager, uint8_t value) noexcept;
    void handleLfoPanDepthCC(VoiceManager* voiceManager, uint8_t value) noexcept;
    void handleSystemCC(VoiceManager* voiceManager, uint8_t controller, uint8_t value) noexcept;
};

#endif // MIDI_CC_MANAGER_H