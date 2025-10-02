/**
 * @file IthacaPluginProcessor.h (Refactored with Async Loading)
 * @brief JUCE AudioProcessor with asynchronous sample loading
 */

#pragma once

#include "IthacaConfig.h"  // Must be first
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

// Async loading
#include "AsyncSampleLoader.h"

//==============================================================================
/**
 * @class IthacaPluginProcessor (Refactored with Async Loading)
 * @brief JUCE Audio Processor with non-blocking sample loading
 * 
 * Key Features:
 * - Asynchronous sample loading in background thread
 * - Non-blocking GUI initialization
 * - Audio processing returns silence during loading
 * - Graceful handling of multiple prepareToPlay() calls
 * - Automatic VoiceManager ownership transfer after loading
 * 
 * Responsibilities:
 * - Audio processing pipeline (JUCE integration)
 * - Async sample loading coordination
 * - MIDI event processing
 * - Sample directory management
 * - State save/load
 * 
 * Delegated Responsibilities:
 * - Parameter layout creation → ParameterManager
 * - Parameter pointer management → ParameterManager
 * - RT-safe parameter updates → ParameterManager
 * - Sample loading logic → AsyncSampleLoader
 * 
 * Thread Safety:
 * - processBlock() is RT-safe
 * - prepareToPlay/releaseResources are audio thread
 * - createEditor/state management are main thread
 * - Async loading runs in dedicated background thread
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
     * @return Pointer to VoiceManager (may be nullptr during loading)
     */
    VoiceManager* getVoiceManager() const { return voiceManager_.get(); }
    
    /**
     * @brief Get current sampler statistics for GUI display
     * @return SamplerStats structure with voice counts and sample rate
     */
    struct SamplerStats {
        int activeVoices = 0;
        int sustainingVoices = 0;
        int releasingVoices = 0;
        int currentSampleRate = 0;
        int totalLoadedSamples = 0;
    };
    SamplerStats getSamplerStats() const;
    
    /**
     * @brief Change sample directory and trigger reload
     * @param newPath New sample directory path
     * @note This will trigger async reload if sample rate is valid
     */
    void changeSampleDirectory(const juce::String& newPath);

    //==============================================================================
    // Async Loading - Public API for GUI
    
    /**
     * @brief Check if sample loading is currently in progress
     * @return true if loading is running in background
     * @note Thread-safe, can be called from GUI thread
     */
    bool isLoadingInProgress() const;
    
    /**
     * @brief Check if loading encountered an error
     * @return true if loading failed
     * @note Thread-safe, can be called from GUI thread
     */
    bool hasLoadingError() const;
    
    /**
     * @brief Get error message from failed loading
     * @return Error message string (empty if no error)
     * @note Thread-safe, can be called from GUI thread
     */
    std::string getLoadingErrorMessage() const;

    //==============================================================================
    // Parameter Management - Public API
    
    /**
     * @brief Get reference to parameter manager
     * @return Reference to ParameterManager instance
     */
    ParameterManager& getParameterManager() { return parameterManager_; }
    
    /**
     * @brief Get reference to APVTS for attachments
     * @return Reference to AudioProcessorValueTreeState
     */
    juce::AudioProcessorValueTreeState& getParameters() { return parameters_; }

private:
    //==============================================================================
    // Core Components
    
    std::unique_ptr<Logger> logger_;                    // IthacaCore logger
    std::unique_ptr<VoiceManager> voiceManager_;        // IthacaCore voice manager
    std::unique_ptr<AsyncSampleLoader> asyncLoader_;    // Async sample loader
    
    //==============================================================================
    // Parameter Management (delegated to ParameterManager)
    
    juce::AudioProcessorValueTreeState parameters_;     // JUCE parameter tree
    ParameterManager parameterManager_;                 // Parameter management delegate
    
    //==============================================================================
    // Sampler State
    
    bool samplerInitialized_;                          // Initialization flag
    double currentSampleRate_;                         // Current sample rate
    int currentBlockSize_;                             // Current buffer size
    
    //==============================================================================
    // Sample Management
    
    juce::String currentSampleDirectory_;              // Active sample directory
    
    //==============================================================================
    // Performance Monitoring
    
    mutable std::atomic<int> processBlockCallCount_;    // Process block counter
    mutable std::atomic<int> totalMidiEventsProcessed_; // MIDI event counter

    //==============================================================================
    // Private Methods - Audio Processing
    
    /**
     * @brief Check and transfer VoiceManager from async loader if ready
     * @note Called from processBlock() to detect loading completion
     */
    void checkAndTransferVoiceManager();
    
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