#include "IthacaPluginProcessor.h"
#include "IthacaPluginEditor.h"

//==============================================================================
IthacaPluginProcessor::IthacaPluginProcessor()
    : AudioProcessor(BusesProperties()
                    .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      pluginInitialized_(false)
{
    // Create delegated components in dependency order
    // NOTE: Using temporary logger for initialization logging
    auto tempLogger = std::make_unique<Logger>(".");
    
    tempLogger->log("IthacaPluginProcessor/constructor", "info", 
                   "=== ITHACA PLUGIN PROCESSOR STARTING ===");
    tempLogger->log("IthacaPluginProcessor/constructor", "info", 
                   "Creating component managers...");
    
    try {
        // Create parameter manager first (no dependencies)
        tempLogger->log("IthacaPluginProcessor/constructor", "info", 
                       "Creating ParameterManager...");
        parameterManager_ = std::make_unique<ParameterManager>();
        tempLogger->log("IthacaPluginProcessor/constructor", "info", 
                       "ParameterManager created successfully");
        
        // Create sampler manager (independent)
        tempLogger->log("IthacaPluginProcessor/constructor", "info", 
                       "Creating SamplerManager...");
        samplerManager_ = std::make_unique<SamplerManager>();
        tempLogger->log("IthacaPluginProcessor/constructor", "info", 
                       "SamplerManager created successfully");
        
        // Create audio engine (depends on both managers)
        tempLogger->log("IthacaPluginProcessor/constructor", "info", 
                       "Creating AudioProcessingEngine...");
        audioEngine_ = std::make_unique<AudioProcessingEngine>(*samplerManager_, *parameterManager_);
        tempLogger->log("IthacaPluginProcessor/constructor", "info", 
                       "AudioProcessingEngine created successfully");
        
        pluginInitialized_ = true;
        
        tempLogger->log("IthacaPluginProcessor/constructor", "info", 
                       "=== PLUGIN PROCESSOR INITIALIZATION COMPLETED ===");
        tempLogger->log("IthacaPluginProcessor/constructor", "info", 
                       "Plugin ready for prepareToPlay() call from DAW");
        
    } catch (const std::exception& e) {
        tempLogger->log("IthacaPluginProcessor/constructor", "error", 
                       "Exception during initialization: " + std::string(e.what()));
        pluginInitialized_ = false;
        
    } catch (...) {
        tempLogger->log("IthacaPluginProcessor/constructor", "error", 
                       "Unknown exception during initialization");
        pluginInitialized_ = false;
    }
}

IthacaPluginProcessor::~IthacaPluginProcessor()
{
    // Create temporary logger for shutdown logging
    auto tempLogger = std::make_unique<Logger>(".");
    tempLogger->log("IthacaPluginProcessor/destructor", "info", 
                   "=== ITHACA PLUGIN PROCESSOR SHUTTING DOWN ===");
    
    // Cleanup in reverse order of creation
    tempLogger->log("IthacaPluginProcessor/destructor", "info", 
                   "Destroying AudioProcessingEngine...");
    audioEngine_.reset();
    
    tempLogger->log("IthacaPluginProcessor/destructor", "info", 
                   "Destroying SamplerManager...");
    samplerManager_.reset();
    
    tempLogger->log("IthacaPluginProcessor/destructor", "info", 
                   "Destroying ParameterManager...");
    parameterManager_.reset();
    
    tempLogger->log("IthacaPluginProcessor/destructor", "info", 
                   "=== PLUGIN PROCESSOR SHUTDOWN COMPLETED ===");
}

//==============================================================================
// JUCE AudioProcessor Interface - Core Methods

void IthacaPluginProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    auto tempLogger = std::make_unique<Logger>(".");
    tempLogger->log("IthacaPluginProcessor/prepareToPlay", "info", 
                   "=== PREPARING AUDIO PROCESSING ===");
    tempLogger->log("IthacaPluginProcessor/prepareToPlay", "info", 
                   "Sample rate: " + std::to_string(sampleRate) + " Hz");
    tempLogger->log("IthacaPluginProcessor/prepareToPlay", "info", 
                   "Buffer size: " + std::to_string(samplesPerBlock) + " samples");
    
    if (!pluginInitialized_) {
        tempLogger->log("IthacaPluginProcessor/prepareToPlay", "error", 
                       "Plugin not properly initialized - cannot prepare for playback");
        return;
    }
    
    try {
        // Delegate to audio engine
        tempLogger->log("IthacaPluginProcessor/prepareToPlay", "info", 
                       "Delegating to AudioProcessingEngine...");
        audioEngine_->prepareToPlay(sampleRate, samplesPerBlock);
        
        tempLogger->log("IthacaPluginProcessor/prepareToPlay", "info", 
                       "=== AUDIO PROCESSING READY ===");
        
    } catch (const std::exception& e) {
        tempLogger->log("IthacaPluginProcessor/prepareToPlay", "error", 
                       "Exception during prepareToPlay: " + std::string(e.what()));
    } catch (...) {
        tempLogger->log("IthacaPluginProcessor/prepareToPlay", "error", 
                       "Unknown exception during prepareToPlay");
    }
}

void IthacaPluginProcessor::releaseResources()
{
    auto tempLogger = std::make_unique<Logger>(".");
    tempLogger->log("IthacaPluginProcessor/releaseResources", "info", 
                   "=== RELEASING AUDIO RESOURCES ===");
    
    if (audioEngine_) {
        audioEngine_->releaseResources();
        tempLogger->log("IthacaPluginProcessor/releaseResources", "info", 
                       "Audio resources released successfully");
    } else {
        tempLogger->log("IthacaPluginProcessor/releaseResources", "warn", 
                       "AudioEngine not available - no resources to release");
    }
}

void IthacaPluginProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    // RT-SAFE: No logging in processBlock!
    juce::ScopedNoDenormals noDenormals;
    
    if (!pluginInitialized_ || !audioEngine_) {
        buffer.clear();
        return;
    }
    
    // Delegate to audio engine (RT-safe)
    audioEngine_->processBlock(buffer, midiMessages);
}

bool IthacaPluginProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    // We only support stereo output
    auto mainOutput = layouts.getMainOutputChannelSet();
    return (mainOutput == juce::AudioChannelSet::stereo());
}

//==============================================================================
// JUCE AudioProcessor Interface - Editor Management

bool IthacaPluginProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* IthacaPluginProcessor::createEditor()
{
    auto tempLogger = std::make_unique<Logger>(".");
    tempLogger->log("IthacaPluginProcessor/createEditor", "info", 
                   "Creating plugin editor GUI");
    
    try {
        auto* editor = new IthacaPluginEditor(*this);
        tempLogger->log("IthacaPluginProcessor/createEditor", "info", 
                       "Plugin editor created successfully");
        return editor;
        
    } catch (const std::exception& e) {
        tempLogger->log("IthacaPluginProcessor/createEditor", "error", 
                       "Exception creating editor: " + std::string(e.what()));
        return nullptr;
        
    } catch (...) {
        tempLogger->log("IthacaPluginProcessor/createEditor", "error", 
                       "Unknown exception creating editor");
        return nullptr;
    }
}

//==============================================================================
// JUCE AudioProcessor Interface - Plugin Metadata

const juce::String IthacaPluginProcessor::getName() const
{
    return JucePlugin_Name;
}

bool IthacaPluginProcessor::acceptsMidi() const
{
    return true;
}

bool IthacaPluginProcessor::producesMidi() const
{
    return false;
}

bool IthacaPluginProcessor::isMidiEffect() const
{
    return false;
}

double IthacaPluginProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

//==============================================================================
// JUCE AudioProcessor Interface - Program Management

int IthacaPluginProcessor::getNumPrograms()
{
    return 1;
}

int IthacaPluginProcessor::getCurrentProgram()
{
    return 0;
}

void IthacaPluginProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String IthacaPluginProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return {};
}

void IthacaPluginProcessor::changeProgramName(int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

//==============================================================================
// JUCE AudioProcessor Interface - State Management

void IthacaPluginProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto tempLogger = std::make_unique<Logger>(".");
    tempLogger->log("IthacaPluginProcessor/getStateInformation", "info", 
                   "Saving plugin state...");
    
    if (parameterManager_) {
        parameterManager_->saveState(destData);
        tempLogger->log("IthacaPluginProcessor/getStateInformation", "info", 
                       "Plugin state saved successfully");
    } else {
        tempLogger->log("IthacaPluginProcessor/getStateInformation", "error", 
                       "ParameterManager not available - cannot save state");
    }
}

void IthacaPluginProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    auto tempLogger = std::make_unique<Logger>(".");
    tempLogger->log("IthacaPluginProcessor/setStateInformation", "info", 
                   "Loading plugin state (" + std::to_string(sizeInBytes) + " bytes)...");
    
    if (parameterManager_) {
        parameterManager_->loadState(data, sizeInBytes);
        tempLogger->log("IthacaPluginProcessor/setStateInformation", "info", 
                       "Plugin state loaded successfully");
    } else {
        tempLogger->log("IthacaPluginProcessor/setStateInformation", "error", 
                       "ParameterManager not available - cannot load state");
    }
}

//==============================================================================
// Public API Implementation

SamplerManager::SamplerStats IthacaPluginProcessor::getSamplerStats() const
{
    if (samplerManager_) {
        return samplerManager_->getStats();
    }
    return SamplerManager::SamplerStats{};
}

void IthacaPluginProcessor::changeSampleDirectory(const juce::String& newPath)
{
    auto tempLogger = std::make_unique<Logger>(".");
    tempLogger->log("IthacaPluginProcessor/changeSampleDirectory", "info", 
                   "Changing sample directory to: " + newPath.toStdString());
    
    if (samplerManager_) {
        bool success = samplerManager_->changeSampleDirectory(newPath);
        if (success) {
            tempLogger->log("IthacaPluginProcessor/changeSampleDirectory", "info", 
                           "Sample directory changed successfully");
        } else {
            tempLogger->log("IthacaPluginProcessor/changeSampleDirectory", "error", 
                           "Failed to change sample directory");
        }
    } else {
        tempLogger->log("IthacaPluginProcessor/changeSampleDirectory", "error", 
                       "SamplerManager not available");
    }
}

//==============================================================================
// Plugin Factory
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new IthacaPluginProcessor();
}