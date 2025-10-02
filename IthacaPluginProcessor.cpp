/**
 * @file IthacaPluginProcessor.cpp (Refactored with Async Loading)
 * @brief Implementation with non-blocking sample loading
 */

#include "IthacaPluginProcessor.h"
#include "IthacaPluginEditor.h"

//==============================================================================
// Constructor - Initialize with async loader

IthacaPluginProcessor::IthacaPluginProcessor()
    : AudioProcessor(BusesProperties()
                    .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters_(*this, nullptr, juce::Identifier("IthacaParameters"), 
                 ParameterManager::createParameterLayout()),
      samplerInitialized_(false),
      currentSampleRate_(0.0),
      currentBlockSize_(0),
      processBlockCallCount_(0),
      totalMidiEventsProcessed_(0)
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
    
    // Set default sample directory
    currentSampleDirectory_ = DEFAULT_SAMPLE_DIR;
    
    logSafe("IthacaPluginProcessor/constructor", "info", "Plugin initialized");
    logSafe("IthacaPluginProcessor/constructor", "info", 
           "Default sample directory: " + currentSampleDirectory_.toStdString());
    logSafe("IthacaPluginProcessor/constructor", "info", 
           "Async loading enabled - samples will load in background");
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
    // Uses original method: ParameterManager directly calls VoiceManager methods
    parameterManager_.updateSamplerParametersRTSafe(voiceManager_.get());
    
    // Process MIDI events
    processMidiEvents(midiMessages);
    
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
// State Management

void IthacaPluginProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // Save parameter state
    auto state = parameters_.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
    
    logSafe("IthacaPluginProcessor/getStateInformation", "info", "Plugin state saved");
}

void IthacaPluginProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // Restore parameter state
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    
    if (xmlState.get() != nullptr) {
        if (xmlState->hasTagName(parameters_.state.getType())) {
            parameters_.replaceState(juce::ValueTree::fromXml(*xmlState));
            logSafe("IthacaPluginProcessor/setStateInformation", "info", "Plugin state restored");
        }
    }
}

//==============================================================================
// IthacaCore Integration - Public API

IthacaPluginProcessor::SamplerStats IthacaPluginProcessor::getSamplerStats() const
{
    SamplerStats stats;
    
    if (voiceManager_) {
        stats.activeVoices = voiceManager_->getActiveVoicesCount();
        stats.sustainingVoices = voiceManager_->getSustainingVoicesCount();
        stats.releasingVoices = voiceManager_->getReleasingVoicesCount();
        stats.currentSampleRate = voiceManager_->getCurrentSampleRate();
        // Note: totalLoadedSamples would need to be exposed from InstrumentLoader
    }
    
    return stats;
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
// Private Methods - MIDI Processing

void IthacaPluginProcessor::processMidiEvents(const juce::MidiBuffer& midiMessages)
{
    if (!voiceManager_) return;
    
    for (const auto& midiMetadata : midiMessages) {
        auto message = midiMetadata.getMessage();
        totalMidiEventsProcessed_.fetch_add(1);
        
        if (message.isNoteOn()) {
            uint8_t midiNote = static_cast<uint8_t>(message.getNoteNumber());
            uint8_t velocity = static_cast<uint8_t>(message.getVelocity());
            voiceManager_->setNoteStateMIDI(midiNote, true, velocity);
        }
        else if (message.isNoteOff()) {
            uint8_t midiNote = static_cast<uint8_t>(message.getNoteNumber());
            voiceManager_->setNoteStateMIDI(midiNote, false);
        }
        else if (message.isController()) {
            uint8_t ccNumber = static_cast<uint8_t>(message.getControllerNumber());
            uint8_t ccValue = static_cast<uint8_t>(message.getControllerValue());
            processMidiControlChange(ccNumber, ccValue);
        }
    }
}

void IthacaPluginProcessor::processMidiControlChange(uint8_t ccNumber, uint8_t ccValue)
{
    // Map MIDI CC to parameters
    juce::RangedAudioParameter* param = nullptr;
    
    switch (ccNumber) {
        case MidiCC::MASTER_GAIN:
            param = parameters_.getParameter("masterGain");
            break;
        case MidiCC::MASTER_PAN:
            param = parameters_.getParameter("masterPan");
            break;
        case MidiCC::ATTACK:
            param = parameters_.getParameter("attack");
            break;
        case MidiCC::RELEASE:
            param = parameters_.getParameter("release");
            break;
        case MidiCC::SUSTAIN_LEVEL:  // Changed from SUSTAIN
            param = parameters_.getParameter("sustainLevel");
            break;
        case MidiCC::LFO_PAN_SPEED:  // Changed from LFO_SPEED
            param = parameters_.getParameter("lfoPanSpeed");
            break;
        case MidiCC::LFO_PAN_DEPTH:  // Changed from LFO_DEPTH
            param = parameters_.getParameter("lfoPanDepth");
            break;
        case MidiCC::STEREO_FIELD:
            param = parameters_.getParameter("stereoField");
            break;
        default:
            return;
    }
    
    // Convert MIDI CC value (0-127) to normalized parameter value (0.0-1.0)
    if (param) {
        float normalizedValue = static_cast<float>(ccValue) / 127.0f;
        param->setValueNotifyingHost(normalizedValue);
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