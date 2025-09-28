/**
 * @file IthacaPluginProcessor.h (Refactored)
 * @brief Zjednodušený JUCE AudioProcessor s delegací parameter managementu
 */

#pragma once

#include "IthacaConfig.h"  // musí být první
#include <juce_audio_processors/juce_audio_processors.h>
#include <memory>
#include <atomic>

// IthacaCore includes
#include "ithaca-core/sampler/voice_manager.h"
#include "ithaca-core/sampler/envelopes/envelope_static_data.h"
#include "ithaca-core/sampler/core_logger.h"

// Parameter management
#include "ParameterManager.h"

// MIDI CC definitions
#include "MidiCCDefinitions.h"

//==============================================================================
/**
 * @class IthacaPluginProcessor (Refactored)
 * @brief JUCE Audio Processor integrating IthacaCore sampler engine
 * 
 * REFAKTOR: Zjednodušený procesor s delegací responsibilities:
 * - Audio processing pipeline (JUCE integration)
 * - IthacaCore VoiceManager lifecycle management
 * - MIDI event processing
 * - Sample directory management
 * - State save/load
 * 
 * Delegované responsibility:
 * - Parameter layout creation → ParameterManager
 * - Parameter pointer management → ParameterManager
 * - RT-safe parameter updates → ParameterManager
 * - Parameter value conversion → ParameterManager
 * 
 * Thread Safety:
 * - processBlock() je RT-safe
 * - prepareToPlay/releaseResources jsou audio thread
 * - createEditor/state management jsou main thread
 */
class IthacaPluginProcessor final : public juce::AudioProcessor
{
public:
    //==============================================================================
    IthacaPluginProcessor();
    ~IthacaPluginProcessor() override;

    //==============================================================================
    // JUCE AudioProcessor Interface
    
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    //==============================================================================
    // Editor Management
    
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    // Plugin Metadata
    
    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    // Program Management (unused)
    
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    // State Management
    
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    //==============================================================================
    // IthacaCore Integration - Public API for GUI
    
    /**
     * @brief Get reference to VoiceManager for GUI access
     * @return Pointer to VoiceManager (může být nullptr)
     */
    VoiceManager* getVoiceManager() const { return voiceManager_.get(); }
    
    /**
     * @brief Get current sampler statistics for GUI display
     */
    struct SamplerStats {
        int activeVoices = 0;
        int sustainingVoices = 0;
        int releasingVoices = 0;
        int totalLoadedSamples = 0;
        int currentSampleRate = 0;
    };
    
    SamplerStats getSamplerStats() const;
    
    /**
     * @brief Change sample directory and reload samples
     * @param newPath New sample directory path
     * @note Non-RT: may allocate and log
     */
    void changeSampleDirectory(const juce::String& newPath);

    //==============================================================================
    // Parameter Management - Delegated to ParameterManager
    
    /**
     * @brief Get reference to JUCE parameter state
     * @return Reference to ValueTreeState for GUI attachments
     */
    juce::AudioProcessorValueTreeState& getParameters() { return parameters_; }
    
    /**
     * @brief Get current parameter values in MIDI format
     * @return ParameterManager for advanced parameter access
     */
    const ParameterManager& getParameterManager() const { return parameterManager_; }

private:
    //==============================================================================
    // Core Components
    
    std::unique_ptr<Logger> logger_;                    // IthacaCore logging system
    std::unique_ptr<VoiceManager> voiceManager_;       // IthacaCore audio engine
    ParameterManager parameterManager_;                // JUCE parameter management
    juce::AudioProcessorValueTreeState parameters_;    // JUCE parameter state
    
    //==============================================================================
    // Audio Processing State
    
    double currentSampleRate_;     // Current audio sample rate
    int currentBlockSize_;         // Current audio buffer size
    bool samplerInitialized_;      // Sampler initialization status
    
    //==============================================================================
    // Sample Management
    
    juce::String currentSampleDirectory_;  // Active sample directory
    
    //==============================================================================
    // Performance Monitoring
    
    mutable std::atomic<int> processBlockCallCount_;    // Process block counter
    mutable std::atomic<int> totalMidiEventsProcessed_; // MIDI event counter

    //==============================================================================
    // Private Methods - IthacaCore Management
    
    /**
     * @brief Initialize IthacaCore sampler system
     * @return true if initialization successful
     * @note Non-RT: may allocate and log
     */
    bool initializeSampler();
    
    /**
     * @brief Process MIDI events from JUCE MidiBuffer
     * @param midiMessages MIDI buffer from processBlock
     * @note RT-safe: no allocations, includes MIDI CC support
     */
    void processMidiEvents(const juce::MidiBuffer& midiMessages);
    
    /**
     * @brief Process MIDI Control Change messages for parameter automation
     * @param ccNumber MIDI CC number (0-127)
     * @param ccValue MIDI CC value (0-127)
     * @note RT-safe: directly updates parameters via JUCE system
     */
    void processMidiControlChange(uint8_t ccNumber, uint8_t ccValue);
    
    /**
     * @brief Safe logging wrapper for non-RT operations
     * @param component Component name for logging
     * @param severity Log severity level
     * @param message Log message content
     */
    void logSafe(const std::string& component, const std::string& severity, 
                const std::string& message) const;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(IthacaPluginProcessor)
};