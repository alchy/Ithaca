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
    if (logger_) {
        logger_->log("IthacaPluginProcessor/constructor", LogSeverity::Info, "=== ITHACA PLUGIN STARTING ===");
    }

    // Initialize parameter pointers through ParameterManager
    if (!parameterManager_.initializeParameterPointers(parameters_)) {
        if (logger_) {
            logger_->log("IthacaPluginProcessor/constructor", LogSeverity::Error,
                       "Failed to initialize parameter pointers");
        }
    }
    
    // Create async sample loader
    asyncLoader_ = std::make_unique<AsyncSampleLoader>();
    if (logger_) {
        logger_->log("IthacaPluginProcessor/constructor", LogSeverity::Info, "Async sample loader created");
    }

    // Create MIDI processor
    midiProcessor_ = std::make_unique<MidiProcessor>();
    if (logger_) {
        logger_->log("IthacaPluginProcessor/constructor", LogSeverity::Info, "MIDI processor created");
    }

    // Create MIDI Learn Manager
    midiLearnManager_ = std::make_unique<MidiLearnManager>(logger_.get());
    if (logger_) {
        logger_->log("IthacaPluginProcessor/constructor", LogSeverity::Info, "MIDI Learn Manager created");
    }

    // Create Performance Monitor
    perfMonitor_ = std::make_unique<PerformanceMonitor>();
    if (logger_) {
        logger_->log("IthacaPluginProcessor/constructor", LogSeverity::Info, "Performance Monitor created");
    }

    // Set default sample directory
    currentSampleDirectory_ = DEFAULT_SAMPLE_DIR;

    if (logger_) {
        logger_->log("IthacaPluginProcessor/constructor", LogSeverity::Info, "Plugin initialized");
    }
    if (logger_) {
        logger_->log("IthacaPluginProcessor/constructor", LogSeverity::Info,
           "Default sample directory: " + currentSampleDirectory_.toStdString());
    }
    if (logger_) {
        logger_->log("IthacaPluginProcessor/constructor", LogSeverity::Info,
           "Async loading enabled - samples will load in background");
    }
    if (logger_) {
        logger_->log("IthacaPluginProcessor/constructor", LogSeverity::Info,
           "MIDI Learn enabled - right-click sliders to assign CC");
    }
}

IthacaPluginProcessor::~IthacaPluginProcessor()
{
    if (logger_) {
        logger_->log("IthacaPluginProcessor/destructor", LogSeverity::Info, "=== ITHACA PLUGIN SHUTTING DOWN ===");
    }

    // Stop any ongoing async loading first
    if (asyncLoader_) {
        if (logger_) {
            logger_->log("IthacaPluginProcessor/destructor", LogSeverity::Info, "Stopping async sample loading...");
        }
        asyncLoader_->stopLoading();
        asyncLoader_.reset();
        if (logger_) {
            logger_->log("IthacaPluginProcessor/destructor", LogSeverity::Info, "Async loader cleaned up");
        }
    }

    // Cleanup VoiceManager (stops all voices)
    if (voiceManager_) {
        if (logger_) {
            logger_->log("IthacaPluginProcessor/destructor", LogSeverity::Info, "Stopping all voices...");
        }
        voiceManager_->stopAllVoices();
        voiceManager_->resetAllVoices(*logger_);
        voiceManager_.reset();
        if (logger_) {
            logger_->log("IthacaPluginProcessor/destructor", LogSeverity::Info, "VoiceManager cleaned up");
        }
    }

    // Cleanup MIDI Learn Manager
    if (midiLearnManager_) {
        midiLearnManager_.reset();
        if (logger_) {
            logger_->log("IthacaPluginProcessor/destructor", LogSeverity::Info, "MIDI Learn Manager cleaned up");
        }
    }

    // Cleanup envelope static data
    EnvelopeStaticData::cleanup();
    if (logger_) {
        logger_->log("IthacaPluginProcessor/destructor", LogSeverity::Info, "Envelope data cleaned up");
    }

    if (logger_) {
        logger_->log("IthacaPluginProcessor/destructor", LogSeverity::Info, "=== PLUGIN CLEANUP COMPLETED ===");
    }
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
    if (logger_) {
        logger_->log("IthacaPluginProcessor/prepareToPlay", LogSeverity::Info, "=== PREPARING AUDIO PROCESSING ===");
    }
    if (logger_) {
        logger_->log("IthacaPluginProcessor/prepareToPlay", LogSeverity::Info,
           "Sample rate: " + std::to_string(sampleRate) + " Hz");
    }
    if (logger_) {
        logger_->log("IthacaPluginProcessor/prepareToPlay", LogSeverity::Info,
           "Buffer size: " + std::to_string(samplesPerBlock) + " samples");
    }

    // Store current audio settings
    currentSampleRate_ = sampleRate;
    currentBlockSize_ = samplesPerBlock;

    // Update performance monitor settings
    if (perfMonitor_) {
        perfMonitor_->setAudioSettings(sampleRate, samplesPerBlock);
    }

    // If already initialized, just update settings
    if (samplerInitialized_ && voiceManager_) {
        // Check if sample rate changed
        if (voiceManager_->getCurrentSampleRate() != static_cast<int>(sampleRate)) {
            if (logger_) {
                logger_->log("IthacaPluginProcessor/prepareToPlay", LogSeverity::Info,
                   "Sample rate changed - triggering reload");
            }
            samplerInitialized_ = false;  // Force reload
        } else {
            // Just update block size
            voiceManager_->prepareToPlay(samplesPerBlock);
            if (logger_) {
                logger_->log("IthacaPluginProcessor/prepareToPlay", LogSeverity::Info,
                   "Audio settings updated (no reload needed)");
            }
            return;
        }
    }

    // Check if we need to start loading
    if (!samplerInitialized_) {
        // Check if already loading for this sample rate
        if (asyncLoader_->isInProgress() &&
            asyncLoader_->getTargetSampleRate() == static_cast<int>(sampleRate)) {
            if (logger_) {
                logger_->log("IthacaPluginProcessor/prepareToPlay", LogSeverity::Info,
                   "Already loading for sample rate " + std::to_string(sampleRate) + " Hz - skipping");
            }
            return;
        }

        // Start async loading
        if (logger_) {
            logger_->log("IthacaPluginProcessor/prepareToPlay", LogSeverity::Info,
               "Starting async sample loading...");
        }

        asyncLoader_->startLoading(
            currentSampleDirectory_.toStdString(),
            static_cast<int>(sampleRate),
            samplesPerBlock,
            *logger_
        );

        if (logger_) {
            logger_->log("IthacaPluginProcessor/prepareToPlay", LogSeverity::Info,
               "Async loading started - GUI remains responsive");
        }
    }

    if (logger_) {
        logger_->log("IthacaPluginProcessor/prepareToPlay", LogSeverity::Info,
           "=== PREPARE TO PLAY COMPLETED ===");
    }
}

void IthacaPluginProcessor::releaseResources()
{
    if (logger_) {
        logger_->log("IthacaPluginProcessor/releaseResources", LogSeverity::Info, "=== RELEASING AUDIO RESOURCES ===");
    }

    if (voiceManager_) {
        voiceManager_->setRealTimeMode(false);
        voiceManager_->stopAllVoices();
        if (logger_) {
            logger_->log("IthacaPluginProcessor/releaseResources", LogSeverity::Info, "All voices stopped");
        }
    }

    if (logger_) {
        logger_->log("IthacaPluginProcessor/releaseResources", LogSeverity::Info, "Audio resources released");
    }
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

    // Start performance measurement
    if (perfMonitor_) {
        perfMonitor_->startMeasurement();
    }

    // Increment process block counter
    processBlockCallCount_.fetch_add(1);

    // Always clear buffer first
    buffer.clear();

    // Check if async loading has completed and transfer VoiceManager
    checkAndTransferVoiceManager();

    // If not initialized, return silence
    if (!samplerInitialized_ || !voiceManager_) {
        // End measurement even if not processing
        if (perfMonitor_) {
            perfMonitor_->endMeasurement();
        }
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

    // End performance measurement
    if (perfMonitor_) {
        perfMonitor_->endMeasurement();
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
    // Delegate to PluginStateManager
    auto logCallback = [this](const std::string& component,
                              LogSeverity severity,
                              const std::string& message) {
        if (logger_) {
            logger_->log(component, severity, message);
        }
    };

    PluginStateManager::saveState(destData, parameters_, midiLearnManager_.get(), logCallback);
}

void IthacaPluginProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // Delegate to PluginStateManager
    auto logCallback = [this](const std::string& component,
                              LogSeverity severity,
                              const std::string& message) {
        if (logger_) {
            logger_->log(component, severity, message);
        }
    };

    PluginStateManager::loadState(data, sizeInBytes, parameters_,
                                  midiLearnManager_.get(), logCallback);
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

    // Add performance metrics
    if (perfMonitor_) {
        auto perfMetrics = perfMonitor_->getMetrics();
        stats.avgProcessingTimeMs = perfMetrics.avgProcessingTimeMs;
        stats.maxProcessingTimeMs = perfMetrics.maxProcessingTimeMs;
        stats.cpuUsagePercent = perfMetrics.cpuUsagePercent;
        stats.dropoutCount = perfMetrics.dropoutCount;
        stats.isDropoutRisk = perfMetrics.isDropoutRisk;
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

    if (logger_) {
        logger_->log("IthacaPluginProcessor/changeSampleDirectory", LogSeverity::Info,
           "Changing sample directory to: " + newPath.toStdString());
    }

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

        if (logger_) {
            logger_->log("IthacaPluginProcessor/checkAndTransfer", LogSeverity::Info,
               "Async loading completed - transferring VoiceManager");
        }

        // Transfer ownership of VoiceManager
        voiceManager_ = asyncLoader_->takeVoiceManager();

        if (voiceManager_) {
            samplerInitialized_ = true;

            if (logger_) {
                logger_->log("IthacaPluginProcessor/checkAndTransfer", LogSeverity::Info,
                   "VoiceManager transferred successfully");
            }

            // Log system statistics
            voiceManager_->logSystemStatistics(*logger_);

            if (logger_) {
                logger_->log("IthacaPluginProcessor/checkAndTransfer", LogSeverity::Info,
                   "=== SAMPLER NOW READY FOR AUDIO PROCESSING ===");
            }
        } else {
            if (logger_) {
                logger_->log("IthacaPluginProcessor/checkAndTransfer", LogSeverity::Error,
                   "Failed to transfer VoiceManager (nullptr)");
            }
        }
    }
}

//==============================================================================
// Plugin Entry Point

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new IthacaPluginProcessor();
}