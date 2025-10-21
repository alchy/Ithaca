/**
 * @file IthacaPluginProcessor.h (COMPLETE with MIDI Learn)
 * @brief JUCE AudioProcessor with asynchronous sample loading and MIDI Learn
 */

#pragma once

#include "ithaca/config/IthacaConfig.h"  // Must be first
#include <juce_audio_processors/juce_audio_processors.h>
#include <memory>
#include <atomic>

// IthacaCore includes
#include "ithaca-core/sampler/voice_manager.h"
#include "ithaca-core/sampler/envelopes/envelope_static_data.h"
#include "ithaca-core/sampler/core_logger.h"

// Parameter management
#include "ithaca/parameters/ParameterManager.h"

// Async loading
#include "ithaca/audio/AsyncSampleLoader.h"

// Performance monitoring
#include "ithaca/audio/PerformanceMonitor.h"

// State management
#include "ithaca/audio/PluginStateManager.h"

// MIDI processing
#include "ithaca/midi/MidiProcessor.h"

// MIDI Learn
#include "ithaca/midi/MidiLearnManager.h"

//==============================================================================
/**
 * @class IthacaPluginProcessor (Complete with MIDI Learn)
 * @brief JUCE Audio Processor with non-blocking sample loading and MIDI Learn
 * 
 * Key Features:
 * - Asynchronous sample loading in background thread
 * - Non-blocking GUI initialization
 * - MIDI Learn functionality for all parameters
 * - Audio processing returns silence during loading
 * - Graceful handling of multiple prepareToPlay() calls
 * - Automatic VoiceManager ownership transfer after loading
 * - Save/Load MIDI Learn mappings with plugin state
 * 
 * Responsibilities:
 * - Audio processing pipeline (JUCE integration)
 * - Async sample loading coordination
 * - MIDI event processing
 * - MIDI Learn management
 * - Sample directory management
 * - State save/load (parameters + MIDI mappings)
 * 
 * Delegated Responsibilities:
 * - Parameter layout creation → ParameterManager
 * - Parameter pointer management → ParameterManager
 * - RT-safe parameter updates → ParameterManager
 * - Sample loading logic → AsyncSampleLoader
 * - MIDI CC processing → MidiProcessor
 * - MIDI Learn logic → MidiLearnManager
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
     * @return SamplerStats structure with voice counts, sample rate, and performance metrics
     */
    struct SamplerStats {
        int activeVoices = 0;
        int sustainingVoices = 0;
        int releasingVoices = 0;
        int currentSampleRate = 0;
        int totalLoadedSamples = 0;

        // Performance metrics
        double avgProcessingTimeMs = 0.0;
        double maxProcessingTimeMs = 0.0;
        double cpuUsagePercent = 0.0;
        int dropoutCount = 0;
        bool isDropoutRisk = false;
    };
    SamplerStats getSamplerStats() const;

    /**
     * @brief Get loaded instrument name from metadata
     * @return Instrument name (empty string if not loaded)
     * @note Thread-safe, can be called from GUI thread
     */
    juce::String getInstrumentName() const;

    /**
     * @brief Get formatted instrument name with velocity layer info
     * @return Formatted string: "Instrument Name (N vel. layers)"
     *         or just "Instrument Name" if velocity count is 0
     * @note Thread-safe, can be called from GUI thread
     */
    juce::String getInstrumentNameWithInfo() const;

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

    //==============================================================================
    // MIDI Learn - Public API for GUI
    
    /**
     * @brief Get pointer to MIDI Learn Manager
     * @return Pointer to MidiLearnManager (never nullptr)
     * @note Used by GUI components to access MIDI Learn functionality
     */
    MidiLearnManager* getMidiLearnManager() { return midiLearnManager_.get(); }

private:
    //==============================================================================
    // Core Components
    
    std::unique_ptr<Logger> logger_;                    // IthacaCore logger
    std::unique_ptr<VoiceManager> voiceManager_;        // IthacaCore voice manager
    std::unique_ptr<AsyncSampleLoader> asyncLoader_;    // Async sample loader
    std::unique_ptr<MidiProcessor> midiProcessor_;      // MIDI event processor
    std::unique_ptr<MidiLearnManager> midiLearnManager_; // MIDI Learn manager
    std::unique_ptr<PerformanceMonitor> perfMonitor_;   // Performance monitor
    
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

    //==============================================================================
    // Private Methods - Audio Processing
    
    /**
     * @brief Check and transfer VoiceManager from async loader if ready
     * @note Called from processBlock() to detect loading completion
     */
    void checkAndTransferVoiceManager();

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(IthacaPluginProcessor)
};