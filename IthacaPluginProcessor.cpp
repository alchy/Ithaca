/**
 * @file IthacaPluginProcessor.cpp (Refactored)
 * @brief Zjednodušená implementace procesoru s delegací na ParameterManager
 */

#include "IthacaPluginProcessor.h"
#include "IthacaPluginEditor.h"

//==============================================================================
// Constructor - Zjednodušený s delegací parameter managementu

IthacaPluginProcessor::IthacaPluginProcessor()
    : AudioProcessor(BusesProperties()
                    .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters_(*this, nullptr, juce::Identifier("IthacaParameters"), 
                 ParameterManager::createParameterLayout()), // DELEGACE: parameter layout
      samplerInitialized_(false),
      currentSampleRate_(0.0),
      currentBlockSize_(0),
      processBlockCallCount_(0),
      totalMidiEventsProcessed_(0)
{
    // Initialize logger
    logger_ = std::make_unique<Logger>(".");
    logSafe("IthacaPluginProcessor/constructor", "info", "=== ITHACA PLUGIN STARTING ===");
    
    // DELEGACE: Initialize parameter pointers through ParameterManager
    if (!parameterManager_.initializeParameterPointers(parameters_)) {
        logSafe("IthacaPluginProcessor/constructor", "error", 
               "Failed to initialize parameter pointers");
    }
    
    // Set default sample directory
    currentSampleDirectory_ = DEFAULT_SAMPLE_DIR;
    
    logSafe("IthacaPluginProcessor/constructor", "info", "Plugin initialized");
    logSafe("IthacaPluginProcessor/constructor", "info", 
           "Default sample directory: " + currentSampleDirectory_.toStdString());
}

IthacaPluginProcessor::~IthacaPluginProcessor()
{
    logSafe("IthacaPluginProcessor/destructor", "info", "=== ITHACA PLUGIN SHUTTING DOWN ===");
    
    // Cleanup VoiceManager first (stops all voices)
    if (voiceManager_) {
        voiceManager_->stopAllVoices();
        voiceManager_->resetAllVoices(*logger_);
        voiceManager_.reset();
    }
    
    // Cleanup envelope static data
    EnvelopeStaticData::cleanup();
    
    logSafe("IthacaPluginProcessor/destructor", "info", "Plugin cleanup completed");
}

//==============================================================================
// JUCE AudioProcessor Interface - Unchanged

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
// Audio Processing Pipeline - Simplified with delegation

void IthacaPluginProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    logSafe("IthacaPluginProcessor/prepareToPlay", "info", "=== PREPARING AUDIO PROCESSING ===");
    logSafe("IthacaPluginProcessor/prepareToPlay", "info", 
           "Sample rate: " + std::to_string(sampleRate) + " Hz");
    logSafe("IthacaPluginProcessor/prepareToPlay", "info", 
           "Buffer size: " + std::to_string(samplesPerBlock) + " samples");
    
    currentSampleRate_ = sampleRate;
    currentBlockSize_ = samplesPerBlock;
    
    // Initialize sampler if not already done
    if (!samplerInitialized_) {
        if (!initializeSampler()) {
            logSafe("IthacaPluginProcessor/prepareToPlay", "error", 
                   "Failed to initialize sampler - audio processing will be disabled");
            return;
        }
    }
    
    // Prepare VoiceManager for new sample rate/block size
    if (voiceManager_) {
        if (voiceManager_->getCurrentSampleRate() != static_cast<int>(sampleRate)) {
            voiceManager_->changeSampleRate(static_cast<int>(sampleRate), *logger_);
        }
        voiceManager_->prepareToPlay(samplesPerBlock);
        voiceManager_->setRealTimeMode(true); // Enable RT mode for audio processing
        
        logSafe("IthacaPluginProcessor/prepareToPlay", "info", "VoiceManager prepared successfully");
    }
    
    logSafe("IthacaPluginProcessor/prepareToPlay", "info", "=== AUDIO PROCESSING READY ===");
}

void IthacaPluginProcessor::releaseResources()
{
    logSafe("IthacaPluginProcessor/releaseResources", "info", "=== RELEASING AUDIO RESOURCES ===");
    
    if (voiceManager_) {
        voiceManager_->setRealTimeMode(false); // Disable RT mode
        voiceManager_->stopAllVoices();
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
    
    // Clear output buffer
    buffer.clear();
    
    // Early exit if sampler not initialized
    if (!samplerInitialized_ || !voiceManager_) {
        return;
    }
    
    // DELEGACE: RT-safe parameter update through ParameterManager
    parameterManager_.updateSamplerParametersRTSafe(voiceManager_.get());
    
    // Process MIDI events
    processMidiEvents(midiMessages);
    
    // Process audio through VoiceManager
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();
    
    if (numChannels >= 2 && numSamples > 0) {
        float* leftChannel = buffer.getWritePointer(0);
        float* rightChannel = buffer.getWritePointer(1);
        
        // Process audio block through IthacaCore VoiceManager
        voiceManager_->processBlockUninterleaved(leftChannel, rightChannel, numSamples);
    }
}

//==============================================================================
// Editor Management - Unchanged

bool IthacaPluginProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* IthacaPluginProcessor::createEditor()
{
    logSafe("IthacaPluginProcessor/createEditor", "info", "Creating plugin editor");
    return new IthacaPluginEditor(*this);
}

//==============================================================================
// State Management - Unchanged

void IthacaPluginProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // Save plugin state
    auto state = parameters_.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
    
    logSafe("IthacaPluginProcessor/getStateInformation", "info", "Plugin state saved");
}

void IthacaPluginProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // Load plugin state
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    
    if (xmlState.get() != nullptr) {
        if (xmlState->hasTagName(parameters_.state.getType())) {
            parameters_.replaceState(juce::ValueTree::fromXml(*xmlState));
            logSafe("IthacaPluginProcessor/setStateInformation", "info", "Plugin state loaded");
        }
    }
}

//==============================================================================
// IthacaCore Integration - Simplified

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
    
    // Reinitialize sampler with new directory
    samplerInitialized_ = false;
    
    if (currentSampleRate_ > 0 && currentBlockSize_ > 0) {
        initializeSampler();
    }
}

//==============================================================================
// Private Methods - Unchanged implementation

bool IthacaPluginProcessor::initializeSampler()
{
    try {
        logSafe("IthacaPluginProcessor/initializeSampler", "info", "Initializing IthacaCore sampler...");
        
        // Initialize envelope static data first
        if (!EnvelopeStaticData::isInitialized()) {
            if (!EnvelopeStaticData::initialize(*logger_)) {
                logSafe("IthacaPluginProcessor/initializeSampler", "error", 
                       "Failed to initialize envelope static data");
                return false;
            }
        }
        
        // Create VoiceManager with exception handling
        try {
            voiceManager_ = std::make_unique<VoiceManager>(currentSampleDirectory_.toStdString(), *logger_);
        } catch (const std::exception& e) {
            logSafe("IthacaPluginProcessor/initializeSampler", "error", 
                   "Failed to create VoiceManager: " + std::string(e.what()));
            return false;
        }
        
        // Initialize system with exception handling
        try {
            voiceManager_->initializeSystem(*logger_);
            voiceManager_->loadForSampleRate(static_cast<int>(currentSampleRate_), *logger_);
        } catch (const std::exception& e) {
            logSafe("IthacaPluginProcessor/initializeSampler", "error", 
                   "Failed to initialize sampler system: " + std::string(e.what()));
            voiceManager_.reset();  // Clean up on failure
            return false;
        }
        
        // Prepare for current block size
        if (currentBlockSize_ > 0) {
            voiceManager_->prepareToPlay(currentBlockSize_);
        }
        
        samplerInitialized_ = true;
        
        logSafe("IthacaPluginProcessor/initializeSampler", "info", "Sampler initialized successfully");
        voiceManager_->logSystemStatistics(*logger_);
        
        return true;
        
    } catch (const std::exception& e) {
        logSafe("IthacaPluginProcessor/initializeSampler", "error", 
               "Exception during sampler initialization: " + std::string(e.what()));
        return false;
    } catch (...) {
        logSafe("IthacaPluginProcessor/initializeSampler", "error", 
               "Unknown exception during sampler initialization");
        return false;
    }
}

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
            // MIDI Control Change processing for parameter automation
            uint8_t ccNumber = static_cast<uint8_t>(message.getControllerNumber());
            uint8_t ccValue = static_cast<uint8_t>(message.getControllerValue());
            processMidiControlChange(ccNumber, ccValue);
        }
        // Future: můžeme přidat pitch bend, aftertouch, etc.
    }
}

void IthacaPluginProcessor::processMidiControlChange(uint8_t ccNumber, uint8_t ccValue)
{
#if ENABLE_MIDI_CC_PROCESSING
    // RT-SAFE: Používá centralizované MIDI CC definice z MidiCCDefinitions.h
    // Přímo nastavuje parameter hodnoty pro GUI sync a host automation
    
    // Najdi parameter ID pro daný CC number
    const char* parameterID = MidiCC::getParameterIDForCC(ccNumber);
    if (!parameterID) {
        // Neznámý CC - ignorujeme pro performance
#if ENABLE_MIDI_CC_LOGGING
        // Logging pouze v debug módu a ne v RT kontextu
        static_cast<void>(ccNumber); // Suppress unused warning in release
#endif
        return;
    }
    
    // Získej JUCE parameter
    auto* param = parameters_.getParameter(parameterID);
    if (!param) {
        return;
    }
    
    // Konverze hodnoty podle typu parametru
    float normalizedValue;
    
    if (ccNumber == MidiCC::MASTER_PAN) {
        // Speciální konverze pro pan parameter
        normalizedValue = MidiCC::ccPanToNormalized(ccValue);
    } else {
        // Standardní konverze pro ostatní parametry
        normalizedValue = MidiCC::ccValueToNormalized(ccValue);
    }
    
    // Nastav parameter hodnotu s host notification
    param->setValueNotifyingHost(normalizedValue);
    
#else
    // MIDI CC processing je vypnuté
    juce::ignoreUnused(ccNumber, ccValue);
#endif
}

void IthacaPluginProcessor::logSafe(const std::string& component, const std::string& severity, 
                                   const std::string& message) const
{
    if (logger_) {
        logger_->log(component, severity, message);
    }
}

//==============================================================================
// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new IthacaPluginProcessor();
}