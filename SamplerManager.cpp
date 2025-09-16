#include "SamplerManager.h"
#include "sampler/envelopes/envelope_static_data.h"
#include <juce_core/juce_core.h>

//==============================================================================
SamplerManager::SamplerManager()
    : currentSampleRate_(0.0),
      currentBlockSize_(0)
{
    // Initialize logger first for all subsequent operations
    logger_ = std::make_unique<Logger>(".");
    logSafe("SamplerManager/constructor", "info", 
           "=== SAMPLER MANAGER INITIALIZING ===");
    
    // Log system information
    logSafe("SamplerManager/constructor", "info", 
           "IthacaCore Version: " + std::string(ITHACA_CORE_VERSION_STRING));
    logSafe("SamplerManager/constructor", "info", 
           "Max voices: " + std::to_string(ITHACA_MAX_VOICES));
    logSafe("SamplerManager/constructor", "info", 
           "Supported sample rates: 44100 Hz, 48000 Hz");
    
    // Determine and validate sample directory
    logSafe("SamplerManager/constructor", "info", 
           "Determining sample directory...");
    currentSampleDirectory_ = determineSampleDirectory();
    
    logSafe("SamplerManager/constructor", "info", 
           "Selected sample directory: " + currentSampleDirectory_.toStdString());
    
    // Validate directory accessibility
    if (isDirectoryAccessible(currentSampleDirectory_)) {
        logSafe("SamplerManager/constructor", "info", 
               "Sample directory is accessible and ready");
    } else {
        logSafe("SamplerManager/constructor", "warn", 
               "Sample directory not accessible - will attempt to use anyway");
        logSafe("SamplerManager/constructor", "warn", 
               "This may cause initialization failure - consider checking sample paths");
    }
    
    clearLastError();
    logSafe("SamplerManager/constructor", "info", 
           "=== SAMPLER MANAGER CREATED (ready for initialize()) ===");
}

SamplerManager::~SamplerManager()
{
    logSafe("SamplerManager/destructor", "info", 
           "=== SAMPLER MANAGER SHUTTING DOWN ===");
    
    // Ensure clean shutdown
    shutdown();
    
    logSafe("SamplerManager/destructor", "info", 
           "=== SAMPLER MANAGER DESTROYED ===");
}

//==============================================================================
// Lifecycle Management

bool SamplerManager::initialize(double sampleRate, int maxBlockSize)
{
    initializationAttempts_.fetch_add(1);
    
    logSafe("SamplerManager/initialize", "info", 
           "=== INITIALIZING SAMPLER SYSTEM ===");
    logSafe("SamplerManager/initialize", "info", 
           "Attempt #" + std::to_string(initializationAttempts_.load()));
    logSafe("SamplerManager/initialize", "info", 
           "Target configuration: " + std::to_string(sampleRate) + " Hz, " + 
           std::to_string(maxBlockSize) + " samples");
    
    // Validate configuration before proceeding
    if (!validateConfiguration()) {
        setLastError("Invalid configuration for initialization");
        logSafe("SamplerManager/initialize", "error", 
               "Configuration validation failed");
        return false;
    }
    
    // Store configuration
    currentSampleRate_ = sampleRate;
    currentBlockSize_ = maxBlockSize;
    
    // Phase 1: Initialize global envelope data
    logSafe("SamplerManager/initialize", "info", 
           "Phase 1: Initializing envelope static data...");
    if (!initializeEnvelopeData()) {
        setLastError("Failed to initialize envelope static data");
        logSafe("SamplerManager/initialize", "error", 
               "Phase 1 failed - cannot proceed with sampler initialization");
        return false;
    }
    logSafe("SamplerManager/initialize", "info", 
           "Phase 1 completed successfully");
    
    // Phase 2: Initialize VoiceManager
    logSafe("SamplerManager/initialize", "info", 
           "Phase 2: Creating and initializing VoiceManager...");
    if (!initializeVoiceManager()) {
        setLastError("Failed to initialize VoiceManager");
        logSafe("SamplerManager/initialize", "error", 
               "Phase 2 failed - VoiceManager initialization unsuccessful");
        return false;
    }
    logSafe("SamplerManager/initialize", "info", 
           "Phase 2 completed successfully");
    
    // Phase 3: Prepare for audio processing
    logSafe("SamplerManager/initialize", "info", 
           "Phase 3: Preparing for audio processing...");
    if (voiceManager_) {
        try {
            voiceManager_->prepareToPlay(maxBlockSize);
            voiceManager_->setRealTimeMode(true);
            
            logSafe("SamplerManager/initialize", "info", 
                   "Audio processing preparation completed");
            logSafe("SamplerManager/initialize", "info", 
                   "Real-time mode enabled for audio thread safety");
        } catch (const std::exception& e) {
            setLastError("Exception during audio preparation: " + std::string(e.what()));
            logSafe("SamplerManager/initialize", "error", 
                   "Phase 3 failed: " + std::string(e.what()));
            return false;
        }
    } else {
        setLastError("VoiceManager is null after initialization");
        logSafe("SamplerManager/initialize", "error", 
               "Phase 3 failed - VoiceManager is unexpectedly null");
        return false;
    }
    
    // Mark as successfully initialized
    isInitialized_.store(true);
    clearLastError();
    
    logSafe("SamplerManager/initialize", "info", 
           "=== SAMPLER SYSTEM INITIALIZATION COMPLETED ===");
    logSafe("SamplerManager/initialize", "info", 
           "System ready for audio processing at " + std::to_string(sampleRate) + " Hz");
    
    // Log system statistics after successful initialization
    logSystemStatistics();
    
    return true;
}

void SamplerManager::shutdown()
{
    logSafe("SamplerManager/shutdown", "info", 
           "=== SHUTTING DOWN SAMPLER SYSTEM ===");
    
    // Mark as no longer initialized
    isInitialized_.store(false);
    
    // Phase 1: Stop audio processing
    if (voiceManager_) {
        logSafe("SamplerManager/shutdown", "info", 
               "Phase 1: Stopping audio processing...");
        try {
            voiceManager_->setRealTimeMode(false);
            voiceManager_->stopAllVoices();
            logSafe("SamplerManager/shutdown", "info", 
                   "All voices stopped, real-time mode disabled");
        } catch (const std::exception& e) {
            logSafe("SamplerManager/shutdown", "warn", 
                   "Exception during voice stopping: " + std::string(e.what()));
        }
    }
    
    // Phase 2: Cleanup VoiceManager
    if (voiceManager_) {
        logSafe("SamplerManager/shutdown", "info", 
               "Phase 2: Cleaning up VoiceManager...");
        try {
            voiceManager_->resetAllVoices(*logger_);
            voiceManager_.reset();
            logSafe("SamplerManager/shutdown", "info", 
                   "VoiceManager destroyed successfully");
        } catch (const std::exception& e) {
            logSafe("SamplerManager/shutdown", "warn", 
                   "Exception during VoiceManager cleanup: " + std::string(e.what()));
            voiceManager_.reset(); // Force cleanup even if exception occurred
        }
    }
    
    // Phase 3: Cleanup global envelope data (optional - other instances might still need it)
    logSafe("SamplerManager/shutdown", "info", 
           "Phase 3: Cleaning up global envelope data...");
    try {
        EnvelopeStaticData::cleanup();
        logSafe("SamplerManager/shutdown", "info", 
               "Global envelope data cleanup completed");
    } catch (const std::exception& e) {
        logSafe("SamplerManager/shutdown", "warn", 
               "Exception during envelope data cleanup: " + std::string(e.what()));
    }
    
    clearLastError();
    logSafe("SamplerManager/shutdown", "info", 
           "=== SAMPLER SYSTEM SHUTDOWN COMPLETED ===");
}

//==============================================================================
// Sample Directory Management

bool SamplerManager::changeSampleDirectory(const juce::String& newPath)
{
    logSafe("SamplerManager/changeSampleDirectory", "info", 
           "Changing sample directory to: " + newPath.toStdString());
    
    if (newPath == currentSampleDirectory_) {
        logSafe("SamplerManager/changeSampleDirectory", "info", 
               "Directory unchanged - no action needed");
        return true;
    }
    
    // Validate new directory
    if (!validateSampleDirectory(newPath)) {
        setLastError("Invalid or inaccessible sample directory: " + newPath.toStdString());
        logSafe("SamplerManager/changeSampleDirectory", "error", 
               "Directory validation failed: " + newPath.toStdString());
        return false;
    }
    
    // Store new directory
    juce::String oldDirectory = currentSampleDirectory_;
    currentSampleDirectory_ = newPath;
    
    logSafe("SamplerManager/changeSampleDirectory", "info", 
           "Directory path updated, reinitializing sampler...");
    
    // Reinitialize if system was already running
    if (isInitialized_.load() && currentSampleRate_ > 0) {
        isInitialized_.store(false);
        
        logSafe("SamplerManager/changeSampleDirectory", "info", 
               "Triggering reinitialization with new sample directory...");
        
        bool success = initialize(currentSampleRate_, currentBlockSize_);
        if (success) {
            logSafe("SamplerManager/changeSampleDirectory", "info", 
                   "Sample directory changed and reinitialized successfully");
        } else {
            // Rollback on failure
            currentSampleDirectory_ = oldDirectory;
            logSafe("SamplerManager/changeSampleDirectory", "error", 
                   "Reinitialization failed - rolled back to previous directory");
            
            // Try to reinitialize with old directory
            if (initialize(currentSampleRate_, currentBlockSize_)) {
                logSafe("SamplerManager/changeSampleDirectory", "info", 
                       "Rollback successful - system restored to previous state");
            } else {
                logSafe("SamplerManager/changeSampleDirectory", "error", 
                       "Rollback failed - system may be in unstable state");
            }
        }
        return success;
    }
    
    logSafe("SamplerManager/changeSampleDirectory", "info", 
           "Directory changed successfully (will take effect on next initialization)");
    return true;
}

juce::String SamplerManager::getCurrentSampleDirectory() const
{
    return currentSampleDirectory_;
}

bool SamplerManager::validateSampleDirectory(const juce::String& path) const
{
    if (path.isEmpty()) {
        return false;
    }
    
    juce::File directory(path);
    
    // Check if directory exists and is actually a directory
    if (!directory.exists() || !directory.isDirectory()) {
        return false;
    }
    
    // Check if directory is readable
    if (!isDirectoryAccessible(path)) {
        return false;
    }
    
    // Check for sample rate subdirectories (optional but recommended)
    juce::File sr44100 = directory.getChildFile("44100");
    juce::File sr48000 = directory.getChildFile("48000");
    
    if (sr44100.exists() || sr48000.exists()) {
        logSafe("SamplerManager/validateSampleDirectory", "info", 
               "Found sample rate subdirectories in: " + path.toStdString());
        return true;
    }
    
    // Check for any WAV files in root directory
    auto wavFiles = directory.findChildFiles(juce::File::findFiles, false, "*.wav");
    if (wavFiles.size() > 0) {
        logSafe("SamplerManager/validateSampleDirectory", "info", 
               "Found " + std::to_string(wavFiles.size()) + " WAV files in: " + path.toStdString());
        return true;
    }
    
    logSafe("SamplerManager/validateSampleDirectory", "warn", 
           "No samples found in directory: " + path.toStdString());
    return false; // No samples found
}

//==============================================================================
// Audio Processing Interface (RT-Safe Delegation)

void SamplerManager::processMidiNote(uint8_t midiNote, bool isNoteOn, uint8_t velocity) noexcept
{
    // RT-SAFE: No logging in this method
    if (!isInitialized_.load() || !voiceManager_) return;
    
    totalMidiEventsProcessed_.fetch_add(1);
    voiceManager_->setNoteStateMIDI(midiNote, isNoteOn, velocity);
}

void SamplerManager::processMidiNote(uint8_t midiNote, bool isNoteOn) noexcept
{
    // RT-SAFE: No logging in this method
    if (!isInitialized_.load() || !voiceManager_) return;
    
    totalMidiEventsProcessed_.fetch_add(1);
    voiceManager_->setNoteStateMIDI(midiNote, isNoteOn);
}

//==============================================================================
// Parameter Control (RT-Safe Operations)

void SamplerManager::setMasterGain(uint8_t gainMIDI)
{
    // RT-SAFE in most contexts, but can log during non-RT parameter updates
    if (!isInitialized_.load() || !voiceManager_) return;
    
    voiceManager_->setAllVoicesMasterGainMIDI(gainMIDI, *logger_);
}

void SamplerManager::setMasterPan(uint8_t panMIDI)
{
    // RT-SAFE: No logging version for audio thread
    if (!isInitialized_.load() || !voiceManager_) return;
    
    voiceManager_->setAllVoicesPanMIDI(panMIDI);
}

void SamplerManager::stopAllVoices() noexcept
{
    // RT-SAFE: No logging in this method
    if (!isInitialized_.load() || !voiceManager_) return;
    
    voiceManager_->stopAllVoices();
}

//==============================================================================
// Monitoring and Statistics

SamplerManager::SamplerStats SamplerManager::getStats() const
{
    SamplerStats stats;
    
    // Basic state
    stats.isInitialized = isInitialized_.load();
    stats.currentSampleDirectory = currentSampleDirectory_.toStdString();
    stats.lastError = lastError_;
    stats.envelopeDataInitialized = EnvelopeStaticData::isInitialized();
    
    // VoiceManager statistics
    if (voiceManager_) {
        stats.activeVoices = voiceManager_->getActiveVoicesCount();
        stats.sustainingVoices = voiceManager_->getSustainingVoicesCount();
        stats.releasingVoices = voiceManager_->getReleasingVoicesCount();
        stats.currentSampleRate = voiceManager_->getCurrentSampleRate();
        // Note: totalLoadedSamples would need InstrumentLoader exposure
    }
    
    return stats;
}

void SamplerManager::logSystemStatistics() const
{
    if (!logger_) return;
    
    logSafe("SamplerManager/statistics", "info", 
           "=== SAMPLER SYSTEM STATISTICS ===");
    
    auto stats = getStats();
    logSafe("SamplerManager/statistics", "info", 
           "Initialization Status: " + std::string(stats.isInitialized ? "Ready" : "Not Initialized"));
    logSafe("SamplerManager/statistics", "info", 
           "Sample Directory: " + stats.currentSampleDirectory);
    logSafe("SamplerManager/statistics", "info", 
           "Current Sample Rate: " + std::to_string(stats.currentSampleRate) + " Hz");
    logSafe("SamplerManager/statistics", "info", 
           "Active Voices: " + std::to_string(stats.activeVoices) + "/" + std::to_string(stats.maxVoices));
    logSafe("SamplerManager/statistics", "info", 
           "Sustaining Voices: " + std::to_string(stats.sustainingVoices));
    logSafe("SamplerManager/statistics", "info", 
           "Releasing Voices: " + std::to_string(stats.releasingVoices));
    logSafe("SamplerManager/statistics", "info", 
           "Total MIDI Events Processed: " + std::to_string(totalMidiEventsProcessed_.load()));
    logSafe("SamplerManager/statistics", "info", 
           "Initialization Attempts: " + std::to_string(initializationAttempts_.load()));
    logSafe("SamplerManager/statistics", "info", 
           "Envelope Data Initialized: " + std::string(stats.envelopeDataInitialized ? "Yes" : "No"));
    
    if (!stats.lastError.empty()) {
        logSafe("SamplerManager/statistics", "warn", 
               "Last Error: " + stats.lastError);
    }
    
    // Delegate to VoiceManager for detailed statistics
    if (voiceManager_) {
        voiceManager_->logSystemStatistics(*logger_);
    }
    
    logSafe("SamplerManager/statistics", "info", 
           "================================");
}

std::string SamplerManager::getPerformanceSummary() const
{
    auto stats = getStats();
    return "Voices: " + std::to_string(stats.activeVoices) + "/" + std::to_string(stats.maxVoices) +
           " | MIDI Events: " + std::to_string(totalMidiEventsProcessed_.load()) +
           " | Sample Rate: " + std::to_string(stats.currentSampleRate) + " Hz" +
           " | Status: " + (stats.isInitialized ? "Ready" : "Not Ready");
}

//==============================================================================
// Advanced Operations

bool SamplerManager::changeSampleRate(double newSampleRate)
{
    logSafe("SamplerManager/changeSampleRate", "info", 
           "Changing sample rate from " + std::to_string(currentSampleRate_) + 
           " Hz to " + std::to_string(newSampleRate) + " Hz");
    
    if (!isSampleRateSupported(newSampleRate)) {
        setLastError("Unsupported sample rate: " + std::to_string(newSampleRate));
        logSafe("SamplerManager/changeSampleRate", "error", 
               "Sample rate not supported: " + std::to_string(newSampleRate));
        return false;
    }
    
    if (currentSampleRate_ == newSampleRate) {
        logSafe("SamplerManager/changeSampleRate", "info", 
               "Sample rate unchanged - no action needed");
        return true;
    }
    
    // Store old sample rate for rollback
    double oldSampleRate = currentSampleRate_;
    currentSampleRate_ = newSampleRate;
    
    // Reinitialize if system is active
    if (isInitialized_.load()) {
        bool success = initialize(newSampleRate, currentBlockSize_);
        if (!success) {
            // Rollback on failure
            currentSampleRate_ = oldSampleRate;
            logSafe("SamplerManager/changeSampleRate", "error", 
                   "Sample rate change failed - rolling back");
            initialize(oldSampleRate, currentBlockSize_);
        }
        return success;
    }
    
    logSafe("SamplerManager/changeSampleRate", "info", 
           "Sample rate updated (will take effect on next initialization)");
    return true;
}

std::vector<int> SamplerManager::getSupportedSampleRates() const
{
    return {ITHACA_DEFAULT_SAMPLE_RATE, ITHACA_ALTERNATIVE_SAMPLE_RATE};
}

bool SamplerManager::isSampleRateSupported(double sampleRate) const
{
    auto supported = getSupportedSampleRates();
    int intSampleRate = static_cast<int>(sampleRate);
    return std::find(supported.begin(), supported.end(), intSampleRate) != supported.end();
}

//==============================================================================
// Private Methods

bool SamplerManager::initializeEnvelopeData()
{
    if (EnvelopeStaticData::isInitialized()) {
        logSafe("SamplerManager/initializeEnvelopeData", "info", 
               "EnvelopeStaticData already initialized globally - skipping");
        return true;
    }
    
    logSafe("SamplerManager/initializeEnvelopeData", "info", 
           "Initializing global envelope static data...");
    
    try {
        if (!EnvelopeStaticData::initialize(*logger_)) {
            setLastError("EnvelopeStaticData::initialize() returned false");
            logSafe("SamplerManager/initializeEnvelopeData", "error", 
                   "EnvelopeStaticData initialization failed");
            return false;
        }
        
        logSafe("SamplerManager/initializeEnvelopeData", "info", 
               "Global envelope static data initialized successfully");
        return true;
        
    } catch (const std::exception& e) {
        setLastError("Exception in EnvelopeStaticData::initialize(): " + std::string(e.what()));
        logSafe("SamplerManager/initializeEnvelopeData", "error", 
               "Exception during envelope data initialization: " + std::string(e.what()));
        return false;
    } catch (...) {
        setLastError("Unknown exception in EnvelopeStaticData::initialize()");
        logSafe("SamplerManager/initializeEnvelopeData", "error", 
               "Unknown exception during envelope data initialization");
        return false;
    }
}

bool SamplerManager::initializeVoiceManager()
{
    try {
        logSafe("SamplerManager/initializeVoiceManager", "info", 
               "Creating VoiceManager with directory: " + currentSampleDirectory_.toStdString());
        
        // Create VoiceManager instance
        voiceManager_ = std::make_unique<VoiceManager>(currentSampleDirectory_.toStdString(), *logger_);
        
        logSafe("SamplerManager/initializeVoiceManager", "info", 
               "VoiceManager instance created, starting initialization pipeline...");
        
        // Phase 1: System initialization (directory scanning)
        logSafe("SamplerManager/initializeVoiceManager", "info", 
               "VoiceManager Phase 1: System initialization...");
        voiceManager_->initializeSystem(*logger_);
        
        // Phase 2: Sample rate specific loading
        logSafe("SamplerManager/initializeVoiceManager", "info", 
               "VoiceManager Phase 2: Loading for sample rate " + 
               std::to_string(static_cast<int>(currentSampleRate_)) + " Hz...");
        voiceManager_->loadForSampleRate(static_cast<int>(currentSampleRate_), *logger_);
        
        logSafe("SamplerManager/initializeVoiceManager", "info", 
               "VoiceManager initialization pipeline completed successfully");
        return true;
        
    } catch (const std::exception& e) {
        setLastError("Exception in VoiceManager initialization: " + std::string(e.what()));
        logSafe("SamplerManager/initializeVoiceManager", "error", 
               "Exception during VoiceManager initialization: " + std::string(e.what()));
        voiceManager_.reset();
        return false;
        
    } catch (...) {
        setLastError("Unknown exception in VoiceManager initialization");
        logSafe("SamplerManager/initializeVoiceManager", "error", 
               "Unknown exception during VoiceManager initialization");
        voiceManager_.reset();
        return false;
    }
}

bool SamplerManager::validateConfiguration() const
{
    // Check sample rate
    if (!isSampleRateSupported(currentSampleRate_)) {
        return false;
    }
    
    // Check block size
    if (currentBlockSize_ < ITHACA_MIN_JUCE_BLOCK_SIZE || 
        currentBlockSize_ > ITHACA_MAX_JUCE_BLOCK_SIZE) {
        return false;
    }
    
    // Check sample directory
    if (currentSampleDirectory_.isEmpty()) {
        return false;
    }
    
    return true;
}

juce::String SamplerManager::determineSampleDirectory() const
{
    logSafe("SamplerManager/determineSampleDirectory", "info", 
           "Determining best sample directory using fallback chain...");
    
    // Try default directory first
    juce::String defaultDir = ITHACA_DEFAULT_SAMPLE_DIR;
    logSafe("SamplerManager/determineSampleDirectory", "info", 
           "Checking default directory: " + defaultDir.toStdString());
    if (juce::File(defaultDir).exists()) {
        logSafe("SamplerManager/determineSampleDirectory", "info", 
               "Default directory found and accessible");
        return defaultDir;
    }
    
    // Try variant directory (for different user accounts)
    #ifdef ITHACA_DEFAULT_SAMPLE_DIR_VARIANT
    juce::String variantDir = ITHACA_DEFAULT_SAMPLE_DIR_VARIANT;
    logSafe("SamplerManager/determineSampleDirectory", "info", 
           "Checking variant directory: " + variantDir.toStdString());
    if (juce::File(variantDir).exists()) {
        logSafe("SamplerManager/determineSampleDirectory", "info", 
               "Variant directory found and accessible");
        return variantDir;
    }
    #endif
    
    // Fallback to system directory
    juce::String fallbackDir = ITHACA_FALLBACK_SAMPLE_DIR;
    logSafe("SamplerManager/determineSampleDirectory", "info", 
           "Checking fallback directory: " + fallbackDir.toStdString());
    if (juce::File(fallbackDir).exists()) {
        logSafe("SamplerManager/determineSampleDirectory", "info", 
               "Fallback directory found and accessible");
        return fallbackDir;
    }
    
    // Last resort - return default even if it doesn't exist
    logSafe("SamplerManager/determineSampleDirectory", "warn", 
           "No accessible directories found - using default path anyway");
    logSafe("SamplerManager/determineSampleDirectory", "warn", 
           "This may cause initialization failures if samples are not present");
    return defaultDir;
}

bool SamplerManager::isDirectoryAccessible(const juce::String& path) const
{
    try {
        juce::File directory(path);
        return directory.exists() && directory.isDirectory();
    } catch (...) {
        return false;
    }
}

void SamplerManager::logSafe(const std::string& component, const std::string& severity, 
                            const std::string& message) const
{
    if (logger_) {
        logger_->log(component, severity, message);
    }
}

void SamplerManager::setLastError(const std::string& errorMessage)
{
    lastError_ = errorMessage;
}

void SamplerManager::clearLastError()
{
    lastError_.clear();
}