/**
 * @file AsyncSampleLoader.h
 * @brief Asynchronous sample loading wrapper for IthacaCore
 * 
 * This class encapsulates all async loading logic, providing a clean interface
 * for loading samples in a background thread without blocking the main/audio thread.
 */

#pragma once

#include <atomic>
#include <thread>
#include <memory>
#include <string>
#include <mutex>

// Forward declarations - avoid including heavy headers
class VoiceManager;
class Logger;

/**
 * @class AsyncSampleLoader
 * @brief Thread-safe asynchronous sample loader
 * 
 * Manages background loading of sample data with these features:
 * - Non-blocking sample loading
 * - Graceful interruption support
 * - State tracking (Idle, InProgress, Completed, Error)
 * - Ownership transfer of VoiceManager after successful load
 * - Prevention of duplicate loads for same sample rate
 */
class AsyncSampleLoader {
public:
    /**
     * @enum LoadingState
     * @brief Represents the current state of the loading process
     */
    enum class LoadingState {
        Idle,           ///< No loading in progress
        InProgress,     ///< Loading is currently running
        Completed,      ///< Loading finished successfully
        Error          ///< Loading failed with error
    };

    /**
     * @brief Constructor - initializes loader in Idle state
     */
    AsyncSampleLoader();
    
    /**
     * @brief Destructor - ensures thread is stopped gracefully
     */
    ~AsyncSampleLoader();
    
    // Disable copy/move (thread management makes this complex)
    AsyncSampleLoader(const AsyncSampleLoader&) = delete;
    AsyncSampleLoader& operator=(const AsyncSampleLoader&) = delete;
    AsyncSampleLoader(AsyncSampleLoader&&) = delete;
    AsyncSampleLoader& operator=(AsyncSampleLoader&&) = delete;
    
    //==========================================================================
    // Main Interface
    
    /**
     * @brief Start asynchronous sample loading
     * @param sampleDirectory Path to sample directory
     * @param targetSampleRate Target sample rate (44100 or 48000 Hz)
     * @param blockSize Audio block size for prepareToPlay
     * @param logger Reference to Logger for error reporting
     * 
     * If loading is already in progress, it will be stopped first.
     * This method returns immediately - loading happens in background thread.
     */
    void startLoading(const std::string& sampleDirectory, 
                     int targetSampleRate,
                     int blockSize,
                     Logger& logger);
    
    /**
     * @brief Stop loading gracefully
     * 
     * Sets stop flag and waits for thread to finish (blocking call).
     * Safe to call even if no loading is in progress.
     */
    void stopLoading();
    
    //==========================================================================
    // State Queries (thread-safe, RT-safe)
    
    /**
     * @brief Get current loading state
     * @return Current LoadingState
     */
    LoadingState getState() const;
    
    /**
     * @brief Check if loading is currently in progress
     * @return true if loading is running
     */
    bool isInProgress() const;
    
    /**
     * @brief Check if an error occurred during loading
     * @return true if loading failed
     */
    bool hasError() const;
    
    /**
     * @brief Get error message (if hasError() returns true)
     * @return Error message string (empty if no error)
     */
    std::string getErrorMessage() const;
    
    /**
     * @brief Get target sample rate for current/last loading operation
     * @return Sample rate in Hz (0 if never started)
     */
    int getTargetSampleRate() const;
    
    //==========================================================================
    // Result Transfer
    
    /**
     * @brief Transfer ownership of loaded VoiceManager
     * @return Unique pointer to VoiceManager (nullptr if not completed)
     *
     * This method should be called once after getState() returns Completed.
     * After calling, the loader returns to Idle state and VoiceManager
     * ownership is transferred to caller.
     */
    std::unique_ptr<VoiceManager> takeVoiceManager();

    /**
     * @brief Get loaded instrument name from metadata
     * @return Instrument name (empty string if not loaded or no metadata)
     */
    std::string getInstrumentName() const;

private:
    //==========================================================================
    // Thread Management
    
    std::unique_ptr<std::thread> loadingThread_;  ///< Worker thread
    std::atomic<bool> shouldStop_;                ///< Interrupt flag
    
    //==========================================================================
    // State Management
    
    std::atomic<LoadingState> state_;             ///< Current loading state
    int targetSampleRate_;                        ///< Target sample rate
    std::string errorMessage_;                    ///< Error details
    mutable std::mutex stateMutex_;               ///< Protects errorMessage_
    
    //==========================================================================
    // Result Storage

    std::unique_ptr<VoiceManager> voiceManager_;  ///< Loaded VoiceManager
    std::string instrumentName_;                   ///< Loaded instrument name from JSON
    
    //==========================================================================
    // Worker Function
    
    /**
     * @brief Worker function running in background thread
     * @param sampleDirectory Sample directory path
     * @param targetSampleRate Target sample rate
     * @param blockSize Block size for audio processing
     * @param logger Logger pointer (guaranteed valid during execution)
     * 
     * This function:
     * 1. Initializes EnvelopeStaticData
     * 2. Creates VoiceManager
     * 3. Calls initializeSystem()
     * 4. Calls loadForSampleRate()
     * 5. Calls prepareToPlay() and setRealTimeMode()
     * 
     * Checks shouldStop_ flag between each step for graceful interruption.
     */
    void workerFunction(const std::string& sampleDirectory,
                       int targetSampleRate,
                       int blockSize,
                       Logger* logger);
};