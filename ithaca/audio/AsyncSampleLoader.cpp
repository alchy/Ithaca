/**
 * @file AsyncSampleLoader.cpp
 * @brief Implementation of asynchronous sample loading
 */

#include "ithaca/audio/AsyncSampleLoader.h"
#include "ithaca/audio/InstrumentMetadata.h"
#include "ithaca-core/sampler/voice_manager.h"
#include "ithaca-core/sampler/envelopes/envelope_static_data.h"
#include "ithaca-core/sampler/core_logger.h"
#include <juce_core/juce_core.h>

//==============================================================================
// Constructor / Destructor

AsyncSampleLoader::AsyncSampleLoader()
    : shouldStop_(false),
      state_(LoadingState::Idle),
      targetSampleRate_(0),
      velocityLayerCount_(0)
{
}

AsyncSampleLoader::~AsyncSampleLoader()
{
    stopLoading();
}

//==============================================================================
// Main Interface

void AsyncSampleLoader::startLoading(const std::string& sampleDirectory,
                                     int targetSampleRate,
                                     int blockSize,
                                     Logger& logger)
{
    // Stop any previous loading operation
    stopLoading();
    
    // Reset state for new loading operation
    {
        std::lock_guard<std::mutex> lock(stateMutex_);
        state_.store(LoadingState::InProgress);
        targetSampleRate_ = targetSampleRate;
        errorMessage_.clear();
        shouldStop_.store(false);
        voiceManager_.reset();
    }
    
    // Start worker thread
    loadingThread_ = std::make_unique<std::thread>(
        &AsyncSampleLoader::workerFunction,
        this,
        sampleDirectory,
        targetSampleRate,
        blockSize,
        &logger
    );
}

void AsyncSampleLoader::initializeWithSineWaves(int targetSampleRate,
                                                 int blockSize,
                                                 Logger& logger,
                                                 int velocityLayerCount)
{
    logger.log("AsyncSampleLoader/initializeWithSineWaves", LogSeverity::Info,
              "=== INITIALIZING WITH SINE WAVES ===");
    logger.log("AsyncSampleLoader/initializeWithSineWaves", LogSeverity::Info,
              "Target sample rate: " + std::to_string(targetSampleRate) + " Hz");
    logger.log("AsyncSampleLoader/initializeWithSineWaves", LogSeverity::Info,
              "Velocity layers: " + std::to_string(velocityLayerCount));

    // Stop any previous loading operation
    stopLoading();

    try {
        // Initialize EnvelopeStaticData if not already done
        if (!EnvelopeStaticData::isInitialized()) {
            logger.log("AsyncSampleLoader/initializeWithSineWaves", LogSeverity::Info,
                      "Initializing envelope static data...");
            EnvelopeStaticData::initialize(logger);
        }

        // Create VoiceManager with sine waves (synchronous, fast)
        logger.log("AsyncSampleLoader/initializeWithSineWaves", LogSeverity::Info,
                  "Creating VoiceManager with sine waves...");
        auto vm = std::make_unique<VoiceManager>(logger, velocityLayerCount, targetSampleRate);

        // Prepare to play
        vm->prepareToPlay(blockSize);

        // Store VoiceManager
        {
            std::lock_guard<std::mutex> lock(stateMutex_);
            voiceManager_ = std::move(vm);
            targetSampleRate_ = targetSampleRate;
            velocityLayerCount_ = velocityLayerCount;
            instrumentName_ = "Sine Wave Test Tone";
            state_.store(LoadingState::Completed);
        }

        logger.log("AsyncSampleLoader/initializeWithSineWaves", LogSeverity::Info,
                  "=== SINE WAVE INITIALIZATION COMPLETE ===");

    } catch (const std::exception& e) {
        logger.log("AsyncSampleLoader/initializeWithSineWaves", LogSeverity::Error,
                  "Failed to initialize with sine waves: " + std::string(e.what()));
        std::lock_guard<std::mutex> lock(stateMutex_);
        errorMessage_ = e.what();
        state_.store(LoadingState::Error);
    }
}

void AsyncSampleLoader::loadSampleBankAsync(const std::string& sampleDirectory, Logger& logger)
{
    logger.log("AsyncSampleLoader/loadSampleBankAsync", LogSeverity::Info,
              "=== LOADING SAMPLE BANK (ASYNC) ===");
    logger.log("AsyncSampleLoader/loadSampleBankAsync", LogSeverity::Info,
              "Sample directory: " + sampleDirectory);

    // Verify sample rate is set (from sine wave initialization)
    if (targetSampleRate_ <= 0) {
        logger.log("AsyncSampleLoader/loadSampleBankAsync", LogSeverity::Error,
                  "Cannot load sample bank: Sample rate not set. Call initializeWithSineWaves() first.");
        return;
    }

    // Stop any previous loading operation
    stopLoading();

    // Use stored sample rate (set during sine wave initialization)
    int currentSampleRate = targetSampleRate_;

    logger.log("AsyncSampleLoader/loadSampleBankAsync", LogSeverity::Info,
              "Using sample rate: " + std::to_string(currentSampleRate) + " Hz");

    // Reset state for new loading operation
    {
        std::lock_guard<std::mutex> lock(stateMutex_);
        state_.store(LoadingState::InProgress);
        errorMessage_.clear();
        shouldStop_.store(false);
    }

    // Start worker thread for sample bank loading
    loadingThread_ = std::make_unique<std::thread>(
        &AsyncSampleLoader::sampleBankWorkerFunction,
        this,
        sampleDirectory,
        currentSampleRate,
        &logger
    );
}

void AsyncSampleLoader::stopLoading()
{
    if (!loadingThread_) {
        return;
    }
    
    // Signal thread to stop
    shouldStop_.store(true);
    
    // Wait for thread to finish
    if (loadingThread_->joinable()) {
        loadingThread_->join();
    }
    
    loadingThread_.reset();
}

//==============================================================================
// State Queries

AsyncSampleLoader::LoadingState AsyncSampleLoader::getState() const
{
    return state_.load();
}

bool AsyncSampleLoader::isInProgress() const
{
    return state_.load() == LoadingState::InProgress;
}

bool AsyncSampleLoader::hasError() const
{
    return state_.load() == LoadingState::Error;
}

std::string AsyncSampleLoader::getErrorMessage() const
{
    std::lock_guard<std::mutex> lock(stateMutex_);
    return errorMessage_;
}

int AsyncSampleLoader::getTargetSampleRate() const
{
    return targetSampleRate_;
}

//==============================================================================
// Result Transfer

bool AsyncSampleLoader::hasVoiceManager() const
{
    std::lock_guard<std::mutex> lock(stateMutex_);
    return voiceManager_ != nullptr;
}

std::unique_ptr<VoiceManager> AsyncSampleLoader::takeVoiceManager()
{
    std::lock_guard<std::mutex> lock(stateMutex_);

    // Transfer ownership and reset state
    auto result = std::move(voiceManager_);

    if (result) {
        state_.store(LoadingState::Idle);
    }

    return result;
}

std::string AsyncSampleLoader::getInstrumentName() const
{
    std::lock_guard<std::mutex> lock(stateMutex_);
    return instrumentName_;
}

int AsyncSampleLoader::getVelocityLayerCount() const
{
    std::lock_guard<std::mutex> lock(stateMutex_);
    return velocityLayerCount_;
}

//==============================================================================
// Worker Function

void AsyncSampleLoader::workerFunction(const std::string& sampleDirectory,
                                       int targetSampleRate,
                                       int blockSize,
                                       Logger* logger)
{
    try {
        // Step 1: Check for interruption
        if (shouldStop_.load()) {
            state_.store(LoadingState::Idle);
            return;
        }

        logger->log("AsyncSampleLoader", LogSeverity::Info,
                   "=== ASYNC LOADING STARTED ===");
        logger->log("AsyncSampleLoader", LogSeverity::Info,
                   "Target sample rate: " + std::to_string(targetSampleRate) + " Hz");
        logger->log("AsyncSampleLoader", LogSeverity::Info,
                   "Sample directory: " + sampleDirectory);

        // Step 1a: Load instrument metadata from JSON
        logger->log("AsyncSampleLoader", LogSeverity::Info,
                   "Loading instrument metadata...");

        juce::File sampleDir(sampleDirectory);
        auto metadata = InstrumentMetadataLoader::loadFromDirectory(sampleDir, logger);

        {
            std::lock_guard<std::mutex> lock(stateMutex_);
            instrumentName_ = metadata.instrumentName.toStdString();
            velocityLayerCount_ = metadata.velocityMaps;
        }

        // Log all metadata fields (Q1B: only populated fields)
        logger->log("AsyncSampleLoader/Metadata", LogSeverity::Info,
                   "=== INSTRUMENT METADATA ===");
        logger->log("AsyncSampleLoader/Metadata", LogSeverity::Info,
                   "Instrument Name: " + metadata.instrumentName.toStdString());

        // Q2A: Indicate if velocityMaps is default (není v JSON)
        // Poznámka: Parser už nastavil default 8, nemůžeme rozlišit mezi "8" v JSON a chybějícím polem
        // Ale to je OK, protože v obou případech chceme použít 8
        logger->log("AsyncSampleLoader/Metadata", LogSeverity::Info,
                   "Velocity Maps: " + std::to_string(metadata.velocityMaps));

        if (!metadata.instrumentVersion.isEmpty()) {
            logger->log("AsyncSampleLoader/Metadata", LogSeverity::Info,
                       "Version: " + metadata.instrumentVersion.toStdString());
        }
        if (!metadata.author.isEmpty()) {
            logger->log("AsyncSampleLoader/Metadata", LogSeverity::Info,
                       "Author: " + metadata.author.toStdString());
        }
        if (!metadata.description.isEmpty()) {
            logger->log("AsyncSampleLoader/Metadata", LogSeverity::Info,
                       "Description: " + metadata.description.toStdString());
        }
        if (!metadata.category.isEmpty()) {
            logger->log("AsyncSampleLoader/Metadata", LogSeverity::Info,
                       "Category: " + metadata.category.toStdString());
        }
        if (metadata.sampleCount > 0) {
            logger->log("AsyncSampleLoader/Metadata", LogSeverity::Info,
                       "Sample Count: " + std::to_string(metadata.sampleCount));
        }
        logger->log("AsyncSampleLoader/Metadata", LogSeverity::Info,
                   "===========================");
        
        // Step 2: Initialize EnvelopeStaticData if needed
        if (!EnvelopeStaticData::isInitialized()) {
            logger->log("AsyncSampleLoader", LogSeverity::Info, 
                       "Initializing envelope static data...");
            
            if (!EnvelopeStaticData::initialize(*logger)) {
                throw std::runtime_error("Envelope static data initialization failed");
            }
            
            logger->log("AsyncSampleLoader", LogSeverity::Info, 
                       "Envelope static data initialized successfully");
        }
        
        // Step 3: Check for interruption
        if (shouldStop_.load()) {
            logger->log("AsyncSampleLoader", LogSeverity::Info, 
                       "Loading interrupted after envelope init");
            state_.store(LoadingState::Idle);
            return;
        }
        
        // Step 4: Create VoiceManager with velocity layer count
        int velocityLayers = metadata.velocityMaps;
        logger->log("AsyncSampleLoader", LogSeverity::Info,
                   "Creating VoiceManager with " + std::to_string(velocityLayers) + " velocity layers...");

        auto vm = std::make_unique<VoiceManager>(sampleDirectory, *logger, velocityLayers);

        logger->log("AsyncSampleLoader", LogSeverity::Info,
                   "VoiceManager created successfully");
        
        // Step 5: Check for interruption
        if (shouldStop_.load()) {
            logger->log("AsyncSampleLoader", LogSeverity::Info, 
                       "Loading interrupted after VoiceManager creation");
            state_.store(LoadingState::Idle);
            return;
        }
        
        // Step 6: Initialize system (scan directory)
        logger->log("AsyncSampleLoader", LogSeverity::Info, 
                   "Initializing sampler system (scanning directory)...");
        
        vm->initializeSystem(*logger);
        
        logger->log("AsyncSampleLoader", LogSeverity::Info, 
                   "System initialization completed");
        
        // Step 7: Check for interruption
        if (shouldStop_.load()) {
            logger->log("AsyncSampleLoader", LogSeverity::Info, 
                       "Loading interrupted after system init");
            state_.store(LoadingState::Idle);
            return;
        }
        
        // Step 8: Load samples for target sample rate
        logger->log("AsyncSampleLoader", LogSeverity::Info, 
                   "Loading samples for " + std::to_string(targetSampleRate) + " Hz...");
        logger->log("AsyncSampleLoader", LogSeverity::Info, 
                   "This may take a few seconds...");
        
        vm->loadForSampleRate(targetSampleRate, *logger);
        
        logger->log("AsyncSampleLoader", LogSeverity::Info, 
                   "Samples loaded successfully");
        
        // Step 9: Check for interruption
        if (shouldStop_.load()) {
            logger->log("AsyncSampleLoader", LogSeverity::Info, 
                       "Loading interrupted after sample loading");
            state_.store(LoadingState::Idle);
            return;
        }
        
        // Step 10: Prepare for audio processing
        logger->log("AsyncSampleLoader", LogSeverity::Info, 
                   "Preparing VoiceManager for audio processing...");
        
        vm->prepareToPlay(blockSize);
        vm->setRealTimeMode(true);
        
        logger->log("AsyncSampleLoader", LogSeverity::Info, 
                   "VoiceManager prepared for real-time mode");
        
        // Step 11: Store result and mark as completed
        {
            std::lock_guard<std::mutex> lock(stateMutex_);
            voiceManager_ = std::move(vm);
            state_.store(LoadingState::Completed);
        }
        
        logger->log("AsyncSampleLoader", LogSeverity::Info, 
                   "=== ASYNC LOADING COMPLETED SUCCESSFULLY ===");
        
    } catch (const std::exception& e) {
        // Error occurred - store error message and set state
        std::lock_guard<std::mutex> lock(stateMutex_);
        errorMessage_ = std::string(e.what());
        state_.store(LoadingState::Error);
        
        if (logger) {
            logger->log("AsyncSampleLoader", LogSeverity::Error, 
                       "=== ASYNC LOADING FAILED ===");
            logger->log("AsyncSampleLoader", LogSeverity::Error, 
                       "Error: " + errorMessage_);
        }
    } catch (...) {
        // Unknown error
        std::lock_guard<std::mutex> lock(stateMutex_);
        errorMessage_ = "Unknown error during sample loading";
        state_.store(LoadingState::Error);
        
        if (logger) {
            logger->log("AsyncSampleLoader", LogSeverity::Error,
                       "=== ASYNC LOADING FAILED ===");
            logger->log("AsyncSampleLoader", LogSeverity::Error,
                       "Unknown exception caught");
        }
    }
}

//==============================================================================
// Sample Bank Worker Function

void AsyncSampleLoader::sampleBankWorkerFunction(const std::string& sampleDirectory,
                                                  int targetSampleRate,
                                                  Logger* logger)
{
    try {
        if (logger) {
            logger->log("AsyncSampleLoader/sampleBankWorker", LogSeverity::Info,
                       "=== SAMPLE BANK LOADING STARTED (ASYNC) ===");
        }

        // Check for stop signal
        if (shouldStop_.load()) {
            if (logger) {
                logger->log("AsyncSampleLoader/sampleBankWorker", LogSeverity::Warning,
                           "Loading interrupted before starting");
            }
            std::lock_guard<std::mutex> lock(stateMutex_);
            state_.store(LoadingState::Idle);
            return;
        }

        // Load instrument metadata
        if (logger) {
            logger->log("AsyncSampleLoader/sampleBankWorker", LogSeverity::Info,
                       "Loading instrument metadata...");
        }

        InstrumentMetadata metadata = InstrumentMetadataLoader::loadFromDirectory(
            juce::File(sampleDirectory), logger);

        // Check for stop signal
        if (shouldStop_.load()) {
            if (logger) {
                logger->log("AsyncSampleLoader/sampleBankWorker", LogSeverity::Warning,
                           "Loading interrupted after metadata");
            }
            std::lock_guard<std::mutex> lock(stateMutex_);
            state_.store(LoadingState::Idle);
            return;
        }

        // Create new VoiceManager with sample bank (old one was moved to processor)
        if (logger) {
            logger->log("AsyncSampleLoader/sampleBankWorker", LogSeverity::Info,
                       "Creating VoiceManager with " + std::to_string(metadata.velocityMaps) + " velocity layers...");
        }

        auto newVoiceManager = std::make_unique<VoiceManager>(sampleDirectory, *logger, metadata.velocityMaps);

        if (logger) {
            logger->log("AsyncSampleLoader/sampleBankWorker", LogSeverity::Info,
                       "VoiceManager created successfully");
        }

        // Check for stop signal
        if (shouldStop_.load()) {
            if (logger) {
                logger->log("AsyncSampleLoader/sampleBankWorker", LogSeverity::Warning,
                           "Loading interrupted after VoiceManager creation");
            }
            std::lock_guard<std::mutex> lock(stateMutex_);
            state_.store(LoadingState::Idle);
            return;
        }

        // Initialize system (scan directory)
        if (logger) {
            logger->log("AsyncSampleLoader/sampleBankWorker", LogSeverity::Info,
                       "Initializing sampler system (scanning directory)...");
        }

        newVoiceManager->initializeSystem(*logger);

        if (logger) {
            logger->log("AsyncSampleLoader/sampleBankWorker", LogSeverity::Info,
                       "System initialized successfully");
        }

        // Check for stop signal
        if (shouldStop_.load()) {
            if (logger) {
                logger->log("AsyncSampleLoader/sampleBankWorker", LogSeverity::Warning,
                           "Loading interrupted after system initialization");
            }
            std::lock_guard<std::mutex> lock(stateMutex_);
            state_.store(LoadingState::Idle);
            return;
        }

        // Load sample bank with target sample rate
        if (logger) {
            logger->log("AsyncSampleLoader/sampleBankWorker", LogSeverity::Info,
                       "Loading sample bank with target sample rate: " + std::to_string(targetSampleRate) + " Hz...");
        }

        newVoiceManager->loadSampleBank(sampleDirectory, targetSampleRate, *logger);

        if (logger) {
            logger->log("AsyncSampleLoader/sampleBankWorker", LogSeverity::Info,
                       "Sample bank loaded successfully");
        }

        // Check for stop signal
        if (shouldStop_.load()) {
            if (logger) {
                logger->log("AsyncSampleLoader/sampleBankWorker", LogSeverity::Warning,
                           "Loading interrupted after sample bank loading");
            }
            std::lock_guard<std::mutex> lock(stateMutex_);
            state_.store(LoadingState::Idle);
            return;
        }

        // Prepare to play
        int blockSize = 256;  // Default block size
        if (logger) {
            logger->log("AsyncSampleLoader/sampleBankWorker", LogSeverity::Info,
                       "Preparing to play (block size: " + std::to_string(blockSize) + ")...");
        }

        newVoiceManager->prepareToPlay(blockSize);

        if (logger) {
            logger->log("AsyncSampleLoader/sampleBankWorker", LogSeverity::Info,
                       "Prepared to play successfully");
        }

        // Store new VoiceManager (thread-safe)
        {
            std::lock_guard<std::mutex> lock(stateMutex_);
            voiceManager_ = std::move(newVoiceManager);
        }

        // Check for stop signal
        if (shouldStop_.load()) {
            if (logger) {
                logger->log("AsyncSampleLoader/sampleBankWorker", LogSeverity::Warning,
                           "Loading interrupted after sample loading");
            }
            std::lock_guard<std::mutex> lock(stateMutex_);
            state_.store(LoadingState::Idle);
            return;
        }

        // Update metadata
        {
            std::lock_guard<std::mutex> lock(stateMutex_);
            instrumentName_ = metadata.instrumentName.toStdString();
            velocityLayerCount_ = metadata.velocityMaps;
            state_.store(LoadingState::Completed);
        }

        if (logger) {
            logger->log("AsyncSampleLoader/sampleBankWorker", LogSeverity::Info,
                       "=== SAMPLE BANK LOADING COMPLETED ===");
            logger->log("AsyncSampleLoader/sampleBankWorker", LogSeverity::Info,
                       "Instrument: " + metadata.instrumentName.toStdString());
            logger->log("AsyncSampleLoader/sampleBankWorker", LogSeverity::Info,
                       "Velocity layers: " + std::to_string(metadata.velocityMaps));
        }

    } catch (const std::exception& e) {
        std::lock_guard<std::mutex> lock(stateMutex_);
        errorMessage_ = e.what();
        state_.store(LoadingState::Error);

        if (logger) {
            logger->log("AsyncSampleLoader/sampleBankWorker", LogSeverity::Error,
                       "=== SAMPLE BANK LOADING FAILED ===");
            logger->log("AsyncSampleLoader/sampleBankWorker", LogSeverity::Error,
                       "Exception: " + std::string(e.what()));
        }

    } catch (...) {
        std::lock_guard<std::mutex> lock(stateMutex_);
        errorMessage_ = "Unknown error during sample bank loading";
        state_.store(LoadingState::Error);

        if (logger) {
            logger->log("AsyncSampleLoader/sampleBankWorker", LogSeverity::Error,
                       "=== SAMPLE BANK LOADING FAILED ===");
            logger->log("AsyncSampleLoader/sampleBankWorker", LogSeverity::Error,
                       "Unknown exception caught");
        }
    }
}