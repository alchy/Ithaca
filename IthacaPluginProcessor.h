#pragma once

#include "IthacaConfig.h"  // PŘIDÁNO - musí být první
#include <juce_audio_processors/juce_audio_processors.h>
#include <memory>

// IthacaCore includes
#include "ithaca-core/sampler/voice_manager.h"
#include "ithaca-core/sampler/envelopes/envelope_static_data.h"
#include "ithaca-core/sampler/core_logger.h"

//==============================================================================
/**
 * @class IthacaPluginProcessor
 * @brief JUCE Audio Processor integrating IthacaCore sampler engine
 * 
 * Integrates the IthacaCore VoiceManager with JUCE audio processing pipeline.
 * Handles MIDI input, audio output, and parameter management.
 */
class IthacaPluginProcessor final : public juce::AudioProcessor
{
public:
    //==============================================================================
    IthacaPluginProcessor();
    ~IthacaPluginProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    // IthacaCore specific methods
    
    /**
     * @brief Get reference to VoiceManager for GUI access
     */
    VoiceManager* getVoiceManager() const { return voiceManager_.get(); }
    
    /**
     * @brief Get current sampler statistics
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
     */
    void changeSampleDirectory(const juce::String& newPath);

    //==============================================================================
    // Parameter management
    juce::AudioProcessorValueTreeState& getParameters() { return parameters_; }

private:
    //==============================================================================
    // IthacaCore components
    std::unique_ptr<Logger> logger_;
    std::unique_ptr<VoiceManager> voiceManager_;
    
    // Sample directory management
    juce::String currentSampleDirectory_;
    
    // JUCE parameter management
    juce::AudioProcessorValueTreeState parameters_;
    std::atomic<float>* masterGainParam_;
    std::atomic<float>* masterPanParam_;
    
    // Audio processing state
    bool samplerInitialized_;
    double currentSampleRate_;
    int currentBlockSize_;
    
    // Performance monitoring
    mutable std::atomic<int> processBlockCallCount_;
    mutable std::atomic<int> totalMidiEventsProcessed_;

    //==============================================================================
    // Private methods
    
    /**
     * @brief Initialize IthacaCore sampler system
     */
    bool initializeSampler();
    
    /**
     * @brief Process MIDI events from JUCE MidiBuffer
     */
    void processMidiEvents(const juce::MidiBuffer& midiMessages);
    
    /**
     * @brief Create parameter layout for JUCE ValueTreeState
     */
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    /**
     * @brief RT-SAFE: Update sampler parameters from JUCE parameters
     * ZMĚNA: RT-safe verze bez loggingu pro použití v processBlock()
     */
    void updateSamplerParametersRTSafe();
    
    /**
     * @brief Safe logging wrapper for non-RT operations
     */
    void logSafe(const std::string& component, const std::string& severity, 
                const std::string& message) const;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IthacaPluginProcessor)
};