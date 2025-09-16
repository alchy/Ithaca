#pragma once

#include <memory>
#include <juce_audio_processors/juce_audio_processors.h>

#include "IthacaConfig.h"
#include "ParameterManager.h"
#include "SamplerManager.h"
#include "AudioProcessingEngine.h"

/**
 * @class IthacaPluginProcessor
 * @brief REFACTORED: Main JUCE Audio Processor with delegated responsibilities
 * 
 * REFACTORING CHANGES:
 * - Extracted parameter management to ParameterManager
 * - Extracted sampler operations to SamplerManager  
 * - Extracted audio processing to AudioProcessingEngine
 * - Simplified to coordinator/facade pattern
 * - Improved separation of concerns and testability
 * 
 * RESPONSIBILITIES:
 * - JUCE AudioProcessor interface implementation
 * - Component coordination and lifecycle management
 * - State persistence (getStateInformation/setStateInformation)
 * - Plugin metadata and capabilities
 * 
 * DESIGN PATTERN:
 * Uses Facade pattern to provide simplified interface to complex subsystems.
 * Each manager handles a specific domain of functionality while this class
 * coordinates their interactions and provides the JUCE integration layer.
 */
class IthacaPluginProcessor final : public juce::AudioProcessor
{
public:
    //==============================================================================
    // Constructor & Destructor
    IthacaPluginProcessor();
    ~IthacaPluginProcessor() override;

    //==============================================================================
    // JUCE AudioProcessor Interface - Core Methods
    
    /**
     * @brief Prepare plugin for audio processing
     * @param sampleRate Sample rate from DAW
     * @param samplesPerBlock Buffer size from DAW
     */
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    
    /**
     * @brief Release audio processing resources
     */
    void releaseResources() override;
    
    /**
     * @brief RT-SAFE: Process audio block with MIDI events
     * @param buffer Audio buffer (stereo output expected)
     * @param midiMessages MIDI events for this block
     */
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;
    using AudioProcessor::processBlock;

    //==============================================================================
    // JUCE AudioProcessor Interface - Configuration
    
    /**
     * @brief Check if bus layout is supported
     * @param layouts Proposed bus configuration
     * @return true if layout is supported (stereo output only)
     */
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    
    //==============================================================================
    // JUCE AudioProcessor Interface - Editor Management
    
    /**
     * @brief Create plugin editor GUI
     * @return Pointer to new editor instance
     */
    juce::AudioProcessorEditor* createEditor() override;
    
    /**
     * @brief Check if plugin has editor GUI
     * @return Always true for this plugin
     */
    bool hasEditor() const override;

    //==============================================================================
    // JUCE AudioProcessor Interface - Plugin Metadata
    
    /**
     * @brief Get plugin name
     * @return Plugin name from JucePlugin_Name
     */
    const juce::String getName() const override;
    
    /**
     * @brief Check if plugin accepts MIDI input
     * @return Always true - this is a MIDI-controlled sampler
     */
    bool acceptsMidi() const override;
    
    /**
     * @brief Check if plugin produces MIDI output
     * @return Always false - audio output only
     */
    bool producesMidi() const override;
    
    /**
     * @brief Check if plugin is MIDI effect
     * @return Always false - this is an instrument plugin
     */
    bool isMidiEffect() const override;
    
    /**
     * @brief Get tail length in seconds
     * @return 0.0 - no tail for sample playback
     */
    double getTailLengthSeconds() const override;

    //==============================================================================
    // JUCE AudioProcessor Interface - Program Management
    
    /**
     * @brief Get number of programs
     * @return Always 1 - single program mode
     */
    int getNumPrograms() override;
    
    /**
     * @brief Get current program index
     * @return Always 0 - single program
     */
    int getCurrentProgram() override;
    
    /**
     * @brief Set current program
     * @param index Program index (ignored - single program)
     */
    void setCurrentProgram(int index) override;
    
    /**
     * @brief Get program name
     * @param index Program index (ignored)
     * @return Empty string - no program names
     */
    const juce::String getProgramName(int index) override;
    
    /**
     * @brief Change program name
     * @param index Program index (ignored)
     * @param newName New name (ignored)
     */
    void changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    // JUCE AudioProcessor Interface - State Management
    
    /**
     * @brief Save plugin state to binary data
     * @param destData Target memory block for state data
     */
    void getStateInformation(juce::MemoryBlock& destData) override;
    
    /**
     * @brief Load plugin state from binary data
     * @param data Source state data
     * @param sizeInBytes Size of state data
     */
    void setStateInformation(const void* data, int sizeInBytes) override;

    //==============================================================================
    // Public API for GUI and External Access
    
    /**
     * @brief Get parameter manager for GUI binding
     * @return Reference to parameter manager
     * @note Use this for GUI parameter binding and automation
     */
    ParameterManager& getParameterManager() { return *parameterManager_; }
    
    /**
     * @brief Get sampler manager for sample operations
     * @return Reference to sampler manager
     * @note Use this for sample loading and voice monitoring
     */
    SamplerManager& getSamplerManager() { return *samplerManager_; }
    
    /**
     * @brief Get current sampler statistics for monitoring
     * @return SamplerStats structure with current values
     * @note Safe to call from GUI thread for display updates
     */
    SamplerManager::SamplerStats getSamplerStats() const;
    
    /**
     * @brief Change sample directory and reload samples
     * @param newPath New sample directory path
     * @note This will trigger reinitialization - may cause brief audio dropout
     */
    void changeSampleDirectory(const juce::String& newPath);

private:
    //==============================================================================
    // Delegated Components (composition over inheritance)
    
    /**
     * @brief Parameter management system
     * Handles JUCE ValueTreeState, parameter definitions, and state persistence
     */
    std::unique_ptr<ParameterManager> parameterManager_;
    
    /**
     * @brief Sampler management system  
     * Handles IthacaCore integration, sample loading, and voice management
     */
    std::unique_ptr<SamplerManager> samplerManager_;
    
    /**
     * @brief Audio processing engine
     * Handles RT-safe audio processing, MIDI events, and parameter updates
     */
    std::unique_ptr<AudioProcessingEngine> audioEngine_;

    //==============================================================================
    // Basic State Tracking
    
    /**
     * @brief Plugin initialization status
     * Used to prevent operations before components are ready
     */
    bool pluginInitialized_;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(IthacaPluginProcessor)
};