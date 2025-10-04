/**
 * @file IthacaPluginProcessor.cpp (COMPLETE with MIDI Learn)
 * @brief Implementation with non-blocking sample loading and MIDI Learn
 */

#include "ithaca/audio/IthacaPluginProcessor.h"
#include "ithaca/gui/IthacaPluginEditor.h"

//==============================================================================
// Constructor - Initialize with async loader and MIDI Learn

IthacaPluginProcessor::IthacaPluginProcessor()
    : AudioProcessor(BusesProperties()
                    .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters_(*this, nullptr, juce::Identifier("IthacaParameters"), 
                 ParameterManager::createParameterLayout()),
      samplerInitialized_(false),
      currentSampleRate_(0.0),
      currentBlockSize_(0),
      processBlockCallCount_(0)
{
    // Initialize logger first
    logger_ = std::make_unique<Logger>(".");
    logSafe("IthacaPluginProcessor/constructor", "info", "=== ITHACA PLUGIN STARTING ===");
    
    // Initialize parameter pointers through ParameterManager
    if (!parameterManager_.initializeParameterPointers(parameters_)) {
        logSafe("IthacaPluginProcessor/constructor", "error", 
               "Failed to initialize parameter pointers");
    }
    
    // Create async sample loader
    asyncLoader_ = std::make_unique<AsyncSampleLoader>();
    logSafe("IthacaPluginProcessor/constructor", "info", "Async sample loader created");
    
    // Create MIDI processor
    midiProcessor_ = std::make_unique<MidiProcessor>();
    logSafe("IthacaPluginProcessor/constructor", "info", "MIDI processor created");
    
    // Create MIDI Learn Manager
    midiLearnManager_ = std::make_unique<MidiLearnManager>();
    logSafe("IthacaPluginProcessor/constructor", "info", "MIDI Learn Manager created");
    
    // Set default sample directory
    currentSampleDirectory_ = DEFAULT_SAMPLE_DIR;
    
    logSafe("IthacaPluginProcessor/constructor", "info", "Plugin initialized");
    logSafe("IthacaPluginProcessor/constructor", "info", 
           "Default sample directory: " + currentSampleDirectory_.toStdString());
    logSafe("IthacaPluginProcessor/constructor", "info", 
           "Async loading enabled - samples will load in background");
    logSafe("IthacaPluginProcessor/constructor", "info", 
           "MIDI Learn enabled - right-click sliders to assign CC");
}

IthacaPluginProcessor::~IthacaPluginProcessor()
{
    logSafe("IthacaPluginProcessor/destructor", "info", "=== ITHACA PLUGIN SHUTTING DOWN ===");
    
    // Stop any ongoing async loading first
    if (asyncLoader_) {
        logSafe("IthacaPluginProcessor/destructor", "info", "Stopping async sample loading...");
        asyncLoader_->stopLoading();
        asyncLoader_.reset();
        logSafe("IthacaPluginProcessor/destructor", "info", "Async loader cleaned up");
    }
    
    // Cleanup VoiceManager (stops all voices)
    if (voiceManager_) {
        logSafe("IthacaPluginProcessor/destructor", "info", "Stopping all voices...");
        voiceManager_->stopAllVoices();
        voiceManager_->resetAllVoices(*logger_);
        voiceManager_.reset();
        logSafe("IthacaPluginProcessor/destructor", "info", "VoiceManager cleaned up");
    }
    
    // Cleanup MIDI Learn Manager
    if (midiLearnManager_) {
        midiLearnManager_.reset();
        logSafe("IthacaPluginProcessor/destructor", "info", "MIDI Learn Manager cleaned up");
    }
    
    // Cleanup envelope static data
    EnvelopeStaticData::cleanup();
    logSafe("IthacaPluginProcessor/destructor", "info", "Envelope data cleaned up");
    
    logSafe("IthacaPluginProcessor/destructor", "info", "=== PLUGIN CLEANUP COMPLETED ===");
}

//==============================================================================
// JUCE AudioProcessor Interface - Basic Metadata

const juce::String IthacaPluginProcessor::getName() const
{
    return JucePlugin_Name;
}

bool IthacaPluginProcessor::acceptsMidi() const { return true; }
bool IthacaPluginProcessor::producesMidi() const { return false; }
bool IthacaPluginProcessor::isMidiEffect() const { return false; }
double IthacaPluginProcessor::getTailLengthSeconds() const { return 0.0; }

int IthacaPluginProcessor::getNumPrograms() { return 1; }
int IthacaPluginProcessor::getCurrentProgram() { return 0; }
void IthacaPluginProcessor::setCurrentProgram(int index) { juce::ignoreUnused(index); }
const juce::String IthacaPluginProcessor::getProgramName(int index) { 
    juce::ignoreUnused(index); return {}; 
}
void IthacaPluginProcessor::changeProgramName(int index, const juce::String& newName) { 
    juce::ignoreUnused(index, newName); 
}

//==============================================================================
// Audio Processing Pipeline - Async Loading Integration

void IthacaPluginProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    logSafe("IthacaPluginProcessor/prepareToPlay", "info", "=== PREPARING AUDIO PROCESSING ===");
    logSafe("IthacaPluginProcessor/prepareToPlay", "info", 
           "Sample rate: " + std::to_string(sampleRate) + " Hz");
    logSafe("IthacaPluginProcessor/prepareToPlay", "info", 
           "Buffer size: " + std::to_string(samplesPerBlock) + " samples");
    
    // Store current audio settings
    currentSampleRate_ = sampleRate;
    currentBlockSize_ = samplesPerBlock;
    
    // If already initialized, just update settings
    if (samplerInitialized_ && voiceManager_) {
        // Check if sample rate changed
        if (voiceManager_->getCurrentSampleRate() != static_cast<int>(sampleRate)) {
            logSafe("IthacaPluginProcessor/prepareToPlay", "info", 
                   "Sample rate changed - triggering reload");
            samplerInitialized_ = false;  // Force reload
        } else {
            // Just update block size
            voiceManager_->prepareToPlay(samplesPerBlock);
            logSafe("IthacaPluginProcessor/prepareToPlay", "info", 
                   "Audio settings updated (no reload needed)");
            return;
        }
    }
    
    // Check if we need to start loading
    if (!samplerInitialized_) {
        // Check if already loading for this sample rate
        if (asyncLoader_->isInProgress() && 
            asyncLoader_->getTargetSampleRate() == static_cast<int>(sampleRate)) {
            logSafe("IthacaPluginProcessor/prepareToPlay", "info", 
                   "Already loading for sample rate " + std::to_string(sampleRate) + " Hz - skipping");
            return;
        }
        
        // Start async loading
        logSafe("IthacaPluginProcessor/prepareToPlay", "info", 
               "Starting async sample loading...");
        
        asyncLoader_->startLoading(
            currentSampleDirectory_.toStdString(),
            static_cast<int>(sampleRate),
            samplesPerBlock,
            *logger_
        );
        
        logSafe("IthacaPluginProcessor/prepareToPlay", "info", 
               "Async loading started - GUI remains responsive");
    }
    
    logSafe("IthacaPluginProcessor/prepareToPlay", "info", 
           "=== PREPARE TO PLAY COMPLETED ===");
}

void IthacaPluginProcessor::releaseResources()
{
    logSafe("IthacaPluginProcessor/releaseResources", "info", "=== RELEASING AUDIO RESOURCES ===");
    
    if (voiceManager_) {
        voiceManager_->setRealTimeMode(false);
        voiceManager_->stopAllVoices();
        logSafe("IthacaPluginProcessor/releaseResources", "info", "All voices stopped");
    }
    
    logSafe("IthacaPluginProcessor/releaseResources", "info", "Audio resources released");
}

bool IthacaPluginProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    // We only support stereo output
    auto mainOutput = layouts.getMainOutputChannelSet();
    return (mainOutput == juce::AudioChannelSet::stereo());
}

void IthacaPluginProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                         juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    
    // Increment process block counter
    processBlockCallCount_.fetch_add(1);
    
    // Always clear buffer first
    buffer.clear();
    
    // Check if async loading has completed and transfer VoiceManager
    checkAndTransferVoiceManager();
    
    // If not initialized, return silence
    if (!samplerInitialized_ || !voiceManager_) {
        return;  // Silent output during loading
    }
    
    // Update VoiceManager parameters (RT-safe through ParameterManager)
    parameterManager_.updateSamplerParametersRTSafe(voiceManager_.get());
    
    // Process MIDI events (delegated to MidiProcessor with MIDI Learn)
    if (midiProcessor_) {
        midiProcessor_->processMidiBuffer(
            midiMessages, 
            voiceManager_.get(), 
            parameters_,
            midiLearnManager_.get()  // Pass MIDI Learn Manager
        );
    }
    
    // Render audio through VoiceManager
    if (voiceManager_) {
        voiceManager_->processBlockUninterleaved(
            buffer.getWritePointer(0),
            buffer.getWritePointer(1),
            buffer.getNumSamples()
        );
    }
}

//==============================================================================
// Editor Management

juce::AudioProcessorEditor* IthacaPluginProcessor::createEditor()
{
    return new IthacaPluginEditor(*this);
}

bool IthacaPluginProcessor::hasEditor() const
{
    return true;
}

//==============================================================================
// State Management - WITH MIDI Learn Mappings

void IthacaPluginProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // Create root XML element
    auto rootXml = std::make_unique<juce::XmlElement>("IthacaPluginState");
    
    // 1. Save parameter state
    auto parameterState = parameters_.copyState();
    auto parameterXml = parameterState.createXml();
    if (parameterXml) {
        rootXml->addChildElement(parameterXml.release());
    }
    
    // 2. Save MIDI Learn mappings
    if (midiLearnManager_) {
        auto midiLearnXml = midiLearnManager_->saveToXml();
        if (midiLearnXml) {
            rootXml->addChildElement(midiLearnXml.release());
        }
    }
    
    // Convert to binary
    copyXmlToBinary(*rootXml, destData);
    
    logSafe("IthacaPluginProcessor/getStateInformation", "info", 
           "Plugin state saved (including MIDI Learn mappings)");
}

void IthacaPluginProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    
    if (xmlState.get() != nullptr) {
        // Check for root element (new format with MIDI Learn)
        if (xmlState->hasTagName("IthacaPluginState")) {
            // 1. Restore parameter state
            auto* parameterXml = xmlState->getChildByName(parameters_.state.getType());
            if (parameterXml != nullptr) {
                parameters_.replaceState(juce::ValueTree::fromXml(*parameterXml));
                logSafe("IthacaPluginProcessor/setStateInformation", "info", 
                       "Parameters restored");
            }
            
            // 2. Restore MIDI Learn mappings
            auto* midiLearnXml = xmlState->getChildByName("MidiLearnMappings");
            if (midiLearnXml != nullptr && midiLearnManager_) {
                midiLearnManager_->loadFromXml(midiLearnXml);
                logSafe("IthacaPluginProcessor/setStateInformation", "info", 
                       "MIDI Learn mappings restored");
            }
        }
        // Legacy format compatibility (old saves without MIDI Learn)
        else if (xmlState->hasTagName(parameters_.state.getType())) {
            parameters_.replaceState(juce::ValueTree::fromXml(*xmlState));
            logSafe("IthacaPluginProcessor/setStateInformation", "info", 
                   "Legacy state restored (no MIDI Learn data)");
        }
    }
}

//==============================================================================
// IthacaCore Integration - Public API for GUI

IthacaPluginProcessor::SamplerStats IthacaPluginProcessor::getSamplerStats() const
{
    SamplerStats stats;
    
    if (voiceManager_) {
        stats.activeVoices = voiceManager_->getActiveVoicesCount();
        stats.sustainingVoices = voiceManager_->getSustainingVoicesCount();
        stats.releasingVoices = voiceManager_->getReleasingVoicesCount();
        stats.currentSampleRate = voiceManager_->getCurrentSampleRate();
    }
    
    return stats;
}

juce::String IthacaPluginProcessor::getInstrumentName() const
{
    if (asyncLoader_) {
        return juce::String(asyncLoader_->getInstrumentName());
    }
    return juce::String();
}

void IthacaPluginProcessor::changeSampleDirectory(const juce::String& newPath)
{
    if (newPath == currentSampleDirectory_) {
        return; // No change needed
    }
    
    logSafe("IthacaPluginProcessor/changeSampleDirectory", "info", 
           "Changing sample directory to: " + newPath.toStdString());
    
    currentSampleDirectory_ = newPath;
    
    // Mark as uninitialized to trigger reload
    samplerInitialized_ = false;
    
    // If we have valid audio settings, start loading
    if (currentSampleRate_ > 0 && currentBlockSize_ > 0) {
        asyncLoader_->startLoading(
            currentSampleDirectory_.toStdString(),
            static_cast<int>(currentSampleRate_),
            currentBlockSize_,
            *logger_
        );
    }
}

//==============================================================================
// Async Loading - Public API for GUI

bool IthacaPluginProcessor::isLoadingInProgress() const
{
    return asyncLoader_ && asyncLoader_->isInProgress();
}

bool IthacaPluginProcessor::hasLoadingError() const
{
    return asyncLoader_ && asyncLoader_->hasError();
}

std::string IthacaPluginProcessor::getLoadingErrorMessage() const
{
    if (asyncLoader_) {
        return asyncLoader_->getErrorMessage();
    }
    return "";
}

//==============================================================================
// Private Methods - Async Loading Integration

void IthacaPluginProcessor::checkAndTransferVoiceManager()
{
    // Check if loading completed
    if (asyncLoader_ && 
        asyncLoader_->getState() == AsyncSampleLoader::LoadingState::Completed && 
        !samplerInitialized_) {
        
        logSafe("IthacaPluginProcessor/checkAndTransfer", "info", 
               "Async loading completed - transferring VoiceManager");
        
        // Transfer ownership of VoiceManager
        voiceManager_ = asyncLoader_->takeVoiceManager();
        
        if (voiceManager_) {
            samplerInitialized_ = true;
            
            logSafe("IthacaPluginProcessor/checkAndTransfer", "info", 
                   "VoiceManager transferred successfully");
            
            // Log system statistics
            voiceManager_->logSystemStatistics(*logger_);
            
            logSafe("IthacaPluginProcessor/checkAndTransfer", "info", 
                   "=== SAMPLER NOW READY FOR AUDIO PROCESSING ===");
        } else {
            logSafe("IthacaPluginProcessor/checkAndTransfer", "error", 
                   "Failed to transfer VoiceManager (nullptr)");
        }
    }
}

//==============================================================================
// Private Methods - Logging

void IthacaPluginProcessor::logSafe(const std::string& component, 
                                    const std::string& severity, 
                                    const std::string& message) const
{
    if (logger_) {
        logger_->log(component, severity, message);
    }
}

//==============================================================================
// Plugin Entry Point

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new IthacaPluginProcessor();
}