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
      targetSampleRate_(0)
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

        logger->log("AsyncSampleLoader", "info",
                   "=== ASYNC LOADING STARTED ===");
        logger->log("AsyncSampleLoader", "info",
                   "Target sample rate: " + std::to_string(targetSampleRate) + " Hz");
        logger->log("AsyncSampleLoader", "info",
                   "Sample directory: " + sampleDirectory);

        // Step 1a: Load instrument metadata from JSON
        logger->log("AsyncSampleLoader", "info",
                   "Loading instrument metadata...");

        juce::File sampleDir(sampleDirectory);
        auto metadata = InstrumentMetadataLoader::loadFromDirectory(sampleDir);

        {
            std::lock_guard<std::mutex> lock(stateMutex_);
            instrumentName_ = metadata.instrumentName.toStdString();
        }

        logger->log("AsyncSampleLoader", "info",
                   "Instrument: " + metadata.instrumentName.toStdString() +
                   " (v" + metadata.instrumentVersion.toStdString() + ")");
        
        // Step 2: Initialize EnvelopeStaticData if needed
        if (!EnvelopeStaticData::isInitialized()) {
            logger->log("AsyncSampleLoader", "info", 
                       "Initializing envelope static data...");
            
            if (!EnvelopeStaticData::initialize(*logger)) {
                throw std::runtime_error("Envelope static data initialization failed");
            }
            
            logger->log("AsyncSampleLoader", "info", 
                       "Envelope static data initialized successfully");
        }
        
        // Step 3: Check for interruption
        if (shouldStop_.load()) {
            logger->log("AsyncSampleLoader", "info", 
                       "Loading interrupted after envelope init");
            state_.store(LoadingState::Idle);
            return;
        }
        
        // Step 4: Create VoiceManager
        logger->log("AsyncSampleLoader", "info", 
                   "Creating VoiceManager...");
        
        auto vm = std::make_unique<VoiceManager>(sampleDirectory, *logger);
        
        logger->log("AsyncSampleLoader", "info", 
                   "VoiceManager created successfully");
        
        // Step 5: Check for interruption
        if (shouldStop_.load()) {
            logger->log("AsyncSampleLoader", "info", 
                       "Loading interrupted after VoiceManager creation");
            state_.store(LoadingState::Idle);
            return;
        }
        
        // Step 6: Initialize system (scan directory)
        logger->log("AsyncSampleLoader", "info", 
                   "Initializing sampler system (scanning directory)...");
        
        vm->initializeSystem(*logger);
        
        logger->log("AsyncSampleLoader", "info", 
                   "System initialization completed");
        
        // Step 7: Check for interruption
        if (shouldStop_.load()) {
            logger->log("AsyncSampleLoader", "info", 
                       "Loading interrupted after system init");
            state_.store(LoadingState::Idle);
            return;
        }
        
        // Step 8: Load samples for target sample rate
        logger->log("AsyncSampleLoader", "info", 
                   "Loading samples for " + std::to_string(targetSampleRate) + " Hz...");
        logger->log("AsyncSampleLoader", "info", 
                   "This may take a few seconds...");
        
        vm->loadForSampleRate(targetSampleRate, *logger);
        
        logger->log("AsyncSampleLoader", "info", 
                   "Samples loaded successfully");
        
        // Step 9: Check for interruption
        if (shouldStop_.load()) {
            logger->log("AsyncSampleLoader", "info", 
                       "Loading interrupted after sample loading");
            state_.store(LoadingState::Idle);
            return;
        }
        
        // Step 10: Prepare for audio processing
        logger->log("AsyncSampleLoader", "info", 
                   "Preparing VoiceManager for audio processing...");
        
        vm->prepareToPlay(blockSize);
        vm->setRealTimeMode(true);
        
        logger->log("AsyncSampleLoader", "info", 
                   "VoiceManager prepared for real-time mode");
        
        // Step 11: Store result and mark as completed
        {
            std::lock_guard<std::mutex> lock(stateMutex_);
            voiceManager_ = std::move(vm);
            state_.store(LoadingState::Completed);
        }
        
        logger->log("AsyncSampleLoader", "info", 
                   "=== ASYNC LOADING COMPLETED SUCCESSFULLY ===");
        
    } catch (const std::exception& e) {
        // Error occurred - store error message and set state
        std::lock_guard<std::mutex> lock(stateMutex_);
        errorMessage_ = std::string(e.what());
        state_.store(LoadingState::Error);
        
        if (logger) {
            logger->log("AsyncSampleLoader", "error", 
                       "=== ASYNC LOADING FAILED ===");
            logger->log("AsyncSampleLoader", "error", 
                       "Error: " + errorMessage_);
        }
    } catch (...) {
        // Unknown error
        std::lock_guard<std::mutex> lock(stateMutex_);
        errorMessage_ = "Unknown error during sample loading";
        state_.store(LoadingState::Error);
        
        if (logger) {
            logger->log("AsyncSampleLoader", "error", 
                       "=== ASYNC LOADING FAILED ===");
            logger->log("AsyncSampleLoader", "error", 
                       "Unknown exception caught");
        }
    }
}