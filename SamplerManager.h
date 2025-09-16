#pragma once

#include <memory>
#include <atomic>
#include <string>
#include <juce_core/juce_core.h>

#include "IthacaConfig.h"
#include "sampler/voice_manager.h"
#include "sampler/core_logger.h"


/**
 * @class SamplerManager  
 * @brief Specialized class for IthacaCore sampler operations and lifecycle management
 * 
 * RESPONSIBILITIES:
 * - VoiceManager lifecycle management and initialization coordination
 * - EnvelopeStaticData global initialization (one-time setup)
 * - Sample directory management with automatic path resolution
 * - Sampler statistics aggregation and monitoring
 * - Error handling and graceful degradation
 * - Thread-safe operations for multi-instance scenarios
 * 
 * DESIGN NOTES:
 * - Encapsulates IthacaCore initialization complexity behind clean interface
 * - Provides robust error handling with detailed logging
 * - Handles platform-specific sample directory resolution
 * - Thread-safe operations where needed for DAW multi-instance support
 * - Clean separation between sampler logic and JUCE integration
 * 
 * INITIALIZATION PIPELINE:
 * 1. Constructor: Basic setup and directory resolution
 * 2. initialize(): EnvelopeStaticData + VoiceManager creation
 * 3. Audio processing: RT-safe operations through VoiceManager
 * 4. shutdown(): Clean resource cleanup in reverse order
 * 
 * ERROR HANDLING STRATEGY:
 * - Graceful degradation: failed init doesn't crash, just disables features
 * - Detailed logging for debugging initialization issues
 * - Exception safety: all exceptions caught and logged appropriately
 * - Safe defaults: return sensible values even when not initialized
 */
class SamplerManager
{
public:
    //==============================================================================
    // Statistics Structure for Monitoring
    struct SamplerStats {
        int activeVoices = 0;           ///< Currently playing voices
        int sustainingVoices = 0;       ///< Voices in sustain phase
        int releasingVoices = 0;        ///< Voices in release phase
        int totalLoadedSamples = 0;     ///< Total samples loaded in memory
        int currentSampleRate = 0;      ///< Active sample rate (0 = not initialized)
        bool isInitialized = false;     ///< Overall initialization status
        std::string currentSampleDirectory; ///< Active sample directory path
        
        // Additional diagnostics
        int maxVoices = 128;            ///< Maximum supported voices (constant)
        bool envelopeDataInitialized = false; ///< EnvelopeStaticData status
        std::string lastError;          ///< Last error message (if any)
    };

    //==============================================================================
    // Constructor & Destructor
    
    /**
     * @brief Constructor - basic setup and directory resolution
     * Initializes logging, determines sample directory, but doesn't load samples yet
     */
    SamplerManager();
    
    /**
     * @brief Destructor - ensures clean shutdown
     * Calls shutdown() to properly cleanup all resources
     */
    ~SamplerManager();

    //==============================================================================
    // Lifecycle Management
    
    /**
     * @brief Initialize complete sampler system for audio processing
     * @param sampleRate Target sample rate (44100 or 48000 Hz)
     * @param maxBlockSize Maximum audio block size from DAW
     * @return true if initialization successful, false otherwise
     * 
     * INITIALIZATION PHASES:
     * 1. EnvelopeStaticData global initialization (if not already done)
     * 2. VoiceManager creation with exception handling
     * 3. Sample loading for specified sample rate
     * 4. Audio processing preparation
     */
    bool initialize(double sampleRate, int maxBlockSize);
    
    /**
     * @brief Release all sampler resources and cleanup
     * Safe to call multiple times, handles cleanup in proper order
     */
    void shutdown();
    
    /**
     * @brief Check if sampler is properly initialized and ready
     * @return true if ready for audio processing operations
     */
    bool isInitialized() const noexcept { return isInitialized_.load(); }

    //==============================================================================
    // Sample Directory Management
    
    /**
     * @brief Change sample directory and reload samples
     * @param newPath New sample directory path (absolute or relative)
     * @return true if successful, false if path invalid or loading failed
     * 
     * NOTE: This triggers reinitialization which may cause brief audio dropout
     */
    bool changeSampleDirectory(const juce::String& newPath);
    
    /**
     * @brief Get current sample directory path
     * @return Current sample directory as JUCE String
     */
    juce::String getCurrentSampleDirectory() const;
    
    /**
     * @brief Validate sample directory exists and is accessible
     * @param path Directory path to validate
     * @return true if directory exists and contains samples
     */
    bool validateSampleDirectory(const juce::String& path) const;

    //==============================================================================
    // Audio Processing Interface (RT-Safe Delegation)
    
    /**
     * @brief Get VoiceManager for direct audio processing
     * @return Pointer to VoiceManager (nullptr if not initialized)
     * @note Use this for processBlock operations in audio thread
     */
    VoiceManager* getVoiceManager() const noexcept { return voiceManager_.get(); }
    
    /**
     * @brief RT-SAFE: Process MIDI note events with velocity
     * @param midiNote MIDI note number (0-127)
     * @param isNoteOn true for note-on, false for note-off
     * @param velocity MIDI velocity (0-127)
     * 
     * @note Safe to call from audio thread - no logging or allocation
     */
    void processMidiNote(uint8_t midiNote, bool isNoteOn, uint8_t velocity) noexcept;
    
    /**
     * @brief RT-SAFE: Process MIDI note events without velocity
     * @param midiNote MIDI note number (0-127)
     * @param isNoteOn true for note-on, false for note-off
     * 
     * @note Safe to call from audio thread - uses default velocity
     */
    void processMidiNote(uint8_t midiNote, bool isNoteOn) noexcept;

    //==============================================================================
    // Parameter Control (RT-Safe Operations)
    
    /**
     * @brief RT-SAFE: Set master gain for all voices
     * @param gainMIDI Master gain as MIDI value (0-127)
     * @note Safe to call from audio thread during parameter updates
     */
    void setMasterGain(uint8_t gainMIDI);
    
    /**
     * @brief RT-SAFE: Set master pan for all voices  
     * @param panMIDI Master pan as MIDI value (0-127, center=64)
     * @note Safe to call from audio thread during parameter updates
     */
    void setMasterPan(uint8_t panMIDI);
    
    /**
     * @brief Stop all currently playing voices (emergency stop)
     * @note RT-safe operation for immediate voice stopping
     */
    void stopAllVoices() noexcept;

    //==============================================================================
    // Monitoring and Statistics
    
    /**
     * @brief Get comprehensive sampler statistics
     * @return SamplerStats structure with current system state
     * @note Safe to call from any thread for monitoring purposes
     */
    SamplerStats getStats() const;
    
    /**
     * @brief Log detailed system statistics to logger
     * @note Non-RT operation - use from GUI or initialization code
     */
    void logSystemStatistics() const;
    
    /**
     * @brief Get performance metrics summary
     * @return String summary of key performance indicators
     */
    std::string getPerformanceSummary() const;

    //==============================================================================
    // Advanced Operations
    
    /**
     * @brief Reinitialize sampler with new sample rate
     * @param newSampleRate New target sample rate
     * @return true if successful
     * @note Triggers full reinitialization - may cause audio dropout
     */
    bool changeSampleRate(double newSampleRate);
    
    /**
     * @brief Get supported sample rates
     * @return Vector of supported sample rate values
     */
    std::vector<int> getSupportedSampleRates() const;
    
    /**
     * @brief Check if specific sample rate is supported
     * @param sampleRate Sample rate to check
     * @return true if sample rate is supported
     */
    bool isSampleRateSupported(double sampleRate) const;

private:
    //==============================================================================
    // Core Components
    
    /**
     * @brief Logger instance for all sampler operations
     * Used throughout initialization and non-RT operations
     */
    std::unique_ptr<Logger> logger_;
    
    /**
     * @brief IthacaCore VoiceManager instance
     * The main audio processing engine - created during initialize()
     */
    std::unique_ptr<VoiceManager> voiceManager_;
    
    //==============================================================================
    // State Management (Thread-Safe)
    
    /**
     * @brief Atomic flag for initialization status
     * Thread-safe access from multiple threads (GUI, audio, etc.)
     */
    std::atomic<bool> isInitialized_{false};
    
    /**
     * @brief Current sample directory path
     * Protected by being set only during non-RT operations
     */
    juce::String currentSampleDirectory_;
    
    /**
     * @brief Current audio configuration
     */
    double currentSampleRate_;          ///< Active sample rate
    int currentBlockSize_;              ///< Current audio block size
    
    //==============================================================================
    // Performance Tracking (Thread-Safe)
    
    /**
     * @brief Total MIDI events processed counter
     * Atomic for thread-safe access from audio and GUI threads
     */
    mutable std::atomic<int> totalMidiEventsProcessed_{0};
    
    /**
     * @brief Initialization attempt counter
     * For debugging repeated initialization failures
     */
    mutable std::atomic<int> initializationAttempts_{0};

    //==============================================================================
    // Private Initialization Methods
    
    /**
     * @brief Initialize EnvelopeStaticData (global, one-time operation)
     * @return true if successful or already initialized
     * 
     * This is the first phase of sampler initialization and must succeed
     * before any VoiceManager instances can be created.
     */
    bool initializeEnvelopeData();
    
    /**
     * @brief Initialize VoiceManager with current settings
     * @return true if successful
     * 
     * Creates VoiceManager instance and runs the IthacaCore initialization
     * pipeline with comprehensive error handling.
     */
    bool initializeVoiceManager();
    
    /**
     * @brief Validate current configuration before initialization
     * @return true if configuration is valid for initialization
     */
    bool validateConfiguration() const;

    //==============================================================================
    // Private Directory Management
    
    /**
     * @brief Determine appropriate sample directory using fallback chain
     * @return Best available sample directory path
     * 
     * RESOLUTION ORDER:
     * 1. ITHACA_DEFAULT_SAMPLE_DIR (if exists)
     * 2. ITHACA_DEFAULT_SAMPLE_DIR_VARIANT (if exists and defined)
     * 3. ITHACA_FALLBACK_SAMPLE_DIR (if exists)
     * 4. Default path (even if doesn't exist)
     */
    juce::String determineSampleDirectory() const;
    
    /**
     * @brief Check if directory exists and is readable
     * @param path Directory path to check
     * @return true if directory is accessible
     */
    bool isDirectoryAccessible(const juce::String& path) const;

    //==============================================================================
    // Private Utility Methods
    
    /**
     * @brief Safe logging wrapper for non-RT operations
     * @param component Component name for log categorization
     * @param severity Log severity level (info, warn, error)
     * @param message Log message content
     * 
     * Only logs if logger is available, safe to call during cleanup
     */
    void logSafe(const std::string& component, const std::string& severity, 
                const std::string& message) const;
    
    /**
     * @brief Update internal error state
     * @param errorMessage Error description for diagnostics
     */
    void setLastError(const std::string& errorMessage);
    
    /**
     * @brief Clear internal error state
     */
    void clearLastError();

    //==============================================================================
    // Error State Tracking
    mutable std::string lastError_;     ///< Last error for diagnostics

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SamplerManager)
};