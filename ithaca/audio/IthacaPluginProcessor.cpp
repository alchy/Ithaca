/**
 * @file IthacaPluginProcessor.cpp (COMPLETE with MIDI Learn)
 * @brief Implementation with non-blocking sample loading and MIDI Learn
 */

#include "ithaca/audio/IthacaPluginProcessor.h"
#include "ithaca/audio/SampleBankPathManager.h"
#include "ithaca/gui/IthacaPluginEditor.h"
#include <filesystem>
#include <iostream>

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
    // Initialize logger first - use plugin data directory (user roaming)
    // IMPORTANT: Ensure the directory exists before creating Logger, as Logger will call
    // std::exit(1) if the directory doesn't exist, causing immediate crash.
    std::filesystem::path loggerDir = SampleBankPathManager::getPluginDataDirectory();

    // Robustly ensure directory exists
    try {
        if (!std::filesystem::exists(loggerDir)) {
            std::filesystem::create_directories(loggerDir);
        }

        // Verify directory was created successfully
        if (!std::filesystem::exists(loggerDir) || !std::filesystem::is_directory(loggerDir)) {
            // Critical: Directory still doesn't exist after creation attempt
            // Fall back to temp directory to avoid crash
            std::cerr << "[IthacaPluginProcessor] ERROR: Failed to create logger directory: "
                      << loggerDir.string() << std::endl;
            std::cerr << "[IthacaPluginProcessor] Falling back to temp directory for logging" << std::endl;
            loggerDir = std::filesystem::temp_directory_path() / "IthacaPlayer" / "logs";
            std::filesystem::create_directories(loggerDir);
        }
    } catch (const std::filesystem::filesystem_error& e) {
        // Handle filesystem errors gracefully
        std::cerr << "[IthacaPluginProcessor] ERROR: Filesystem error while creating logger directory: "
                  << e.what() << std::endl;
        std::cerr << "[IthacaPluginProcessor] Falling back to temp directory for logging" << std::endl;
        loggerDir = std::filesystem::temp_directory_path() / "IthacaPlayer" / "logs";
        try {
            std::filesystem::create_directories(loggerDir);
        } catch (...) {
            std::cerr << "[IthacaPluginProcessor] CRITICAL: Cannot create any log directory!" << std::endl;
        }
    }

    std::string loggerPath = loggerDir.string();

    // Enable console logging in Debug builds for better visibility of errors
    #ifdef _DEBUG
        bool useConsole = true;  // Show logs on console in Debug mode
    #else
        bool useConsole = false; // File-only logging in Release mode
    #endif

    logger_ = std::make_unique<Logger>(loggerPath, LogSeverity::Info, useConsole, true);
    if (logger_) {
        logger_->log("IthacaPluginProcessor/constructor", LogSeverity::Info, "=== ITHACA PLUGIN STARTING ===");
        logger_->log("IthacaPluginProcessor/constructor", LogSeverity::Info,
                   "Logger directory: " + loggerPath);
        if (useConsole) {
            logger_->log("IthacaPluginProcessor/constructor", LogSeverity::Info,
                       "Console logging enabled (Debug build)");
        }
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

    // Initialize with sine waves immediately (fast, non-blocking)
    // Sample bank will be loaded later via GUI folder picker
    if (logger_) {
        logger_->log("IthacaPluginProcessor/constructor", LogSeverity::Info,
                   "Initializing with sine wave test tones...");
    }

    // Initialize with sine waves at 44100 Hz (will be updated in prepareToPlay)
    asyncLoader_->initializeWithSineWaves(44100, 512, *logger_, 8);

    // Transfer VoiceManager immediately (sine wave init is synchronous)
    if (asyncLoader_->getState() == AsyncSampleLoader::LoadingState::Completed) {
        voiceManager_ = asyncLoader_->takeVoiceManager();
        samplerInitialized_ = true;
        if (logger_) {
            logger_->log("IthacaPluginProcessor/constructor", LogSeverity::Info,
                       "Sine wave initialization complete - ready for audio");
        }
    }

    // No sample directory or sample bank loaded initially
    currentSampleDirectory_ = "";
    loadedSampleBankPath_ = "";

    if (logger_) {
        logger_->log("IthacaPluginProcessor/constructor", LogSeverity::Info, "Plugin initialized");
    }
    if (logger_) {
        logger_->log("IthacaPluginProcessor/constructor", LogSeverity::Info,
           "Sine wave mode active - use folder picker to load sample bank");
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
                logger_->log("IthacaPluginProcessor/prepareToPlay", LogSeverity::Warning,
                   "Sample rate changed from " +
                   std::to_string(voiceManager_->getCurrentSampleRate()) +
                   " to " + std::to_string(static_cast<int>(sampleRate)) + " Hz");
                logger_->log("IthacaPluginProcessor/prepareToPlay", LogSeverity::Info,
                   "Reinitializing with sine waves at new sample rate...");
            }

            // Reinitialize with sine waves at new sample rate
            asyncLoader_->initializeWithSineWaves(static_cast<int>(sampleRate), samplesPerBlock, *logger_, 8);

            if (asyncLoader_->getState() == AsyncSampleLoader::LoadingState::Completed) {
                voiceManager_ = asyncLoader_->takeVoiceManager();
                samplerInitialized_ = true;

                // If we had a sample bank loaded, reload it
                if (!loadedSampleBankPath_.isEmpty()) {
                    if (logger_) {
                        logger_->log("IthacaPluginProcessor/prepareToPlay", LogSeverity::Info,
                                   "Reloading sample bank at new sample rate...");
                    }
                    asyncLoader_->loadSampleBankAsync(loadedSampleBankPath_.toStdString(), *logger_);
                }
            }
        } else {
            // Just update block size
            voiceManager_->prepareToPlay(samplesPerBlock);
            if (logger_) {
                logger_->log("IthacaPluginProcessor/prepareToPlay", LogSeverity::Info,
                   "Audio settings updated (no reload needed)");
            }
        }
        return;
    }

    // If not initialized yet (shouldn't happen as constructor initializes with sine waves)
    if (!samplerInitialized_) {
        if (logger_) {
            logger_->log("IthacaPluginProcessor/prepareToPlay", LogSeverity::Warning,
                       "Sampler not initialized - initializing with sine waves now");
        }

        asyncLoader_->initializeWithSineWaves(static_cast<int>(sampleRate), samplesPerBlock, *logger_, 8);

        if (asyncLoader_->getState() == AsyncSampleLoader::LoadingState::Completed) {
            voiceManager_ = asyncLoader_->takeVoiceManager();
            samplerInitialized_ = true;
            if (logger_) {
                logger_->log("IthacaPluginProcessor/prepareToPlay", LogSeverity::Info,
                           "Sine wave initialization complete");
            }
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

    // Save state including sample bank path
    PluginStateManager::saveState(destData, parameters_, midiLearnManager_.get(),
                                  &loadedSampleBankPath_, logCallback);
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

    // Load state including sample bank path
    PluginStateManager::loadState(data, sizeInBytes, parameters_,
                                  midiLearnManager_.get(), &loadedSampleBankPath_, logCallback);

    // If a sample bank path was restored, load it asynchronously
    if (!loadedSampleBankPath_.isEmpty()) {
        if (logger_) {
            logger_->log("IthacaPluginProcessor/setStateInformation", LogSeverity::Info,
                       "Restored sample bank path from state: " + loadedSampleBankPath_.toStdString());
        }

        // Check if the saved path still exists
        std::filesystem::path bankPath(loadedSampleBankPath_.toStdString());
        if (std::filesystem::exists(bankPath) && std::filesystem::is_directory(bankPath)) {
            if (logger_) {
                logger_->log("IthacaPluginProcessor/setStateInformation", LogSeverity::Info,
                           "Sample bank directory exists - will load on next prepareToPlay or can be loaded manually via GUI");
            }
            // Note: We don't load here automatically because prepareToPlay might not have been called yet
            // The GUI can trigger loading via loadSampleBankFromDirectory()
        } else {
            if (logger_) {
                logger_->log("IthacaPluginProcessor/setStateInformation", LogSeverity::Warning,
                           "Saved sample bank path no longer exists - falling back to sine waves");
                logger_->log("IthacaPluginProcessor/setStateInformation", LogSeverity::Warning,
                           "Missing path: " + loadedSampleBankPath_.toStdString());
            }
            // Clear the path since it's invalid
            loadedSampleBankPath_ = "";
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

juce::String IthacaPluginProcessor::getInstrumentNameWithInfo() const
{
    if (!asyncLoader_) {
        return juce::String();
    }

    auto instrumentName = juce::String(asyncLoader_->getInstrumentName());
    int velocityLayers = asyncLoader_->getVelocityLayerCount();

    // If no velocity layers loaded yet, return just the name
    if (velocityLayers == 0) {
        return instrumentName;
    }

    // Format: "Instrument Name (N vel. layers)"
    return instrumentName + " (" + juce::String(velocityLayers) + " vel. layers)";
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

    // Save the new path to persistent storage
    std::filesystem::path pathToSave(newPath.toStdString());
    if (SampleBankPathManager::saveSampleBankPath(pathToSave)) {
        if (logger_) {
            logger_->log("IthacaPluginProcessor/changeSampleDirectory", LogSeverity::Info,
               "Sample bank path saved successfully");
        }
    } else {
        if (logger_) {
            logger_->log("IthacaPluginProcessor/changeSampleDirectory", LogSeverity::Warning,
               "Failed to save sample bank path to persistent storage");
        }
    }

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

void IthacaPluginProcessor::loadSampleBankFromDirectory(const juce::String& sampleBankPath)
{
    if (logger_) {
        logger_->log("IthacaPluginProcessor/loadSampleBankFromDirectory", LogSeverity::Info,
                   "=== LOADING SAMPLE BANK ===");
        logger_->log("IthacaPluginProcessor/loadSampleBankFromDirectory", LogSeverity::Info,
                   "Path: " + sampleBankPath.toStdString());
    }

    // Validate path exists and is a directory
    std::filesystem::path bankPath(sampleBankPath.toStdString());
    if (!std::filesystem::exists(bankPath)) {
        if (logger_) {
            logger_->log("IthacaPluginProcessor/loadSampleBankFromDirectory", LogSeverity::Error,
                       "Sample bank directory does not exist: " + sampleBankPath.toStdString());
        }
        return;
    }

    if (!std::filesystem::is_directory(bankPath)) {
        if (logger_) {
            logger_->log("IthacaPluginProcessor/loadSampleBankFromDirectory", LogSeverity::Error,
                       "Path is not a directory: " + sampleBankPath.toStdString());
        }
        return;
    }

    // Check if VoiceManager is initialized
    if (!voiceManager_) {
        if (logger_) {
            logger_->log("IthacaPluginProcessor/loadSampleBankFromDirectory", LogSeverity::Error,
                       "VoiceManager not initialized - cannot load sample bank");
        }
        return;
    }

    // Start async loading of sample bank into existing VoiceManager
    if (logger_) {
        logger_->log("IthacaPluginProcessor/loadSampleBankFromDirectory", LogSeverity::Info,
                   "Starting async sample bank loading...");
    }

    asyncLoader_->loadSampleBankAsync(sampleBankPath.toStdString(), *logger_);

    // Update loaded path (will be persisted in getStateInformation)
    loadedSampleBankPath_ = sampleBankPath;

    if (logger_) {
        logger_->log("IthacaPluginProcessor/loadSampleBankFromDirectory", LogSeverity::Info,
                   "Sample bank loading started in background");
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
    // Check if loading completed and there's a new VoiceManager ready
    if (asyncLoader_ &&
        asyncLoader_->getState() == AsyncSampleLoader::LoadingState::Completed &&
        asyncLoader_->hasVoiceManager()) {

        if (logger_) {
            logger_->log("IthacaPluginProcessor/checkAndTransfer", LogSeverity::Info,
               "Async loading completed - transferring new VoiceManager");
            if (samplerInitialized_) {
                logger_->log("IthacaPluginProcessor/checkAndTransfer", LogSeverity::Info,
                   "Replacing existing VoiceManager with newly loaded sample bank");
            }
        }

        // Transfer ownership of new VoiceManager (replaces old one if exists)
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