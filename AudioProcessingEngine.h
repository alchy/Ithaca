#pragma once

#include "SamplerManager.h"
#include "ParameterManager.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>
#include <chrono>

/**
 * @class AudioProcessingEngine
 * @brief Specialized class for real-time audio processing and MIDI handling
 * 
 * RESPONSIBILITIES:
 * - RT-safe audio block processing with zero allocation guarantee
 * - MIDI event parsing and routing to sampler system
 * - Real-time parameter updates during audio processing
 * - Performance monitoring and CPU usage tracking
 * - Audio thread safety management and validation
 * - Buffer management and audio format handling
 * 
 * DESIGN NOTES:
 * - Completely RT-safe processBlock implementation (no logging, no allocation)
 * - Atomic operations for all shared data to ensure thread safety
 * - Efficient MIDI event parsing with minimal overhead
 * - Performance monitoring without impacting audio thread
 * - Clean separation between RT and non-RT operations
 * - Robust error handling that doesn't compromise audio stability
 * 
 * RT-SAFETY GUARANTEES:
 * - No memory allocation in processBlock()
 * - No logging or I/O operations in audio thread
 * - No mutex/lock operations in critical path
 * - All shared data access via atomic operations
 * - No exceptions propagated from audio thread
 * - Predictable execution time for all RT operations
 * 
 * THREAD MODEL:
 * - Audio thread: processBlock() and related RT methods
 * - GUI/Message thread: statistics, configuration, logging
 * - Initialization thread: prepareToPlay(), releaseResources()
 * - All cross-thread communication via atomic variables
 */
class AudioProcessingEngine
{
public:
    //==============================================================================
    // Performance Statistics Structure
    struct ProcessingStats {
        int totalBlocksProcessed = 0;       ///< Total audio blocks processed
        int totalMidiEventsProcessed = 0;   ///< Total MIDI events handled
        double averageBlockSize = 0.0;      ///< Average block size processed
        double currentCpuUsage = 0.0;       ///< Current CPU usage estimate (0.0-1.0)
        int lastBlockSize = 0;              ///< Size of last processed block
        double lastSampleRate = 0.0;        ///< Last configured sample rate
        bool isProcessingEnabled = false;   ///< Current processing state
        
        // Timing statistics
        double minProcessingTimeMs = 0.0;   ///< Minimum processing time per block
        double maxProcessingTimeMs = 0.0;   ///< Maximum processing time per block  
        double avgProcessingTimeMs = 0.0;   ///< Average processing time per block
        
        // Error statistics
        int bufferUnderruns = 0;            ///< Count of buffer underrun conditions
        int parameterUpdateErrors = 0;      ///< Parameter update failure count
        int midiProcessingErrors = 0;       ///< MIDI processing error count
    };

    //==============================================================================
    // Constructor & Destructor
    
    /**
     * @brief Constructor - initialize with manager references
     * @param samplerManager Reference to sampler management system
     * @param parameterManager Reference to parameter management system
     * 
     * Note: Managers must outlive this AudioProcessingEngine instance
     */
    explicit AudioProcessingEngine(SamplerManager& samplerManager, 
                                  ParameterManager& parameterManager);
    
    /**
     * @brief Destructor - ensures clean shutdown
     */
    ~AudioProcessingEngine() = default;

    //==============================================================================
    // Audio Processing Lifecycle
    
    /**
     * @brief Prepare audio engine for processing
     * @param sampleRate Sample rate from DAW/host
     * @param maxBlockSize Maximum block size from DAW/host
     * 
     * This method handles:
     * - Sampler system initialization coordination
     * - Performance monitoring setup
     * - Buffer preparation and validation
     * - RT mode activation
     */
    void prepareToPlay(double sampleRate, int maxBlockSize);
    
    /**
     * @brief Release audio processing resources
     * 
     * This method handles:
     * - RT mode deactivation
     * - Voice stopping and cleanup
     * - Performance statistics finalization
     * - Resource cleanup coordination
     */
    void releaseResources();
    
    /**
     * @brief RT-SAFE: Process audio block with MIDI events
     * @param buffer Audio buffer (expects stereo output)
     * @param midiMessages MIDI events for this block
     * 
     * RT-SAFETY GUARANTEES:
     * - No memory allocation
     * - No logging or I/O operations
     * - No exception propagation
     * - Predictable execution time
     * - Thread-safe parameter access
     * 
     * PROCESSING ORDER:
     * 1. Validate inputs and early exit if needed
     * 2. Clear output buffer
     * 3. Process MIDI events
     * 4. Update parameters from parameter manager
     * 5. Delegate audio rendering to sampler
     * 6. Update performance statistics
     */
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) noexcept;

    //==============================================================================
    // Configuration and State Management
    
    /**
     * @brief Check if audio processing is currently enabled
     * @return true if ready to process audio
     */
    bool isProcessingEnabled() const noexcept { return isProcessingEnabled_.load(); }
    
    /**
     * @brief Get current sample rate
     * @return Current sample rate in Hz (0.0 if not initialized)
     */
    double getCurrentSampleRate() const noexcept { return currentSampleRate_; }
    
    /**
     * @brief Get current maximum block size
     * @return Current max block size in samples (0 if not initialized)
     */
    int getCurrentMaxBlockSize() const noexcept { return currentMaxBlockSize_; }

    //==============================================================================
    // Performance Monitoring and Statistics
    
    /**
     * @brief Get comprehensive processing statistics
     * @return ProcessingStats structure with current performance data
     * @note Safe to call from any thread - uses atomic operations
     */
    ProcessingStats getStats() const noexcept;
    
    /**
     * @brief Reset all performance counters and statistics
     * @note Non-RT operation - call from GUI thread only
     */
    void resetStats() noexcept;
    
    /**
     * @brief Get current CPU usage estimate
     * @return CPU usage as percentage (0.0-1.0)
     * @note Based on processing time vs available time per block
     */
    double getCurrentCpuUsage() const noexcept { return estimatedCpuUsage_.load(); }
    
    /**
     * @brief Log detailed performance statistics
     * @note Non-RT operation with comprehensive system diagnostics
     */
    void logPerformanceStatistics() const;

    //==============================================================================
    // Advanced Performance Analysis
    
    /**
     * @brief Check if system is experiencing performance issues
     * @return true if CPU usage or timing indicates problems
     */
    bool isPerformanceStressed() const noexcept;
    
    /**
     * @brief Get performance health score (0.0 = poor, 1.0 = excellent)
     * @return Performance health indicator based on multiple metrics
     */
    double getPerformanceHealthScore() const noexcept;
    
    /**
     * @brief Estimate maximum safe polyphony for current performance
     * @return Recommended maximum number of simultaneous voices
     */
    int getRecommendedMaxPolyphony() const noexcept;

private:
    //==============================================================================
    // Component References (must outlive this instance)
    SamplerManager& samplerManager_;     ///< Reference to sampler management
    ParameterManager& parameterManager_; ///< Reference to parameter management
    
    //==============================================================================
    // Processing State (Thread-Safe)
    
    /**
     * @brief Atomic flag for processing enabled state
     * Modified by prepareToPlay/releaseResources, read by processBlock
     */
    std::atomic<bool> isProcessingEnabled_{false};
    
    /**
     * @brief Current audio configuration
     * Set during prepareToPlay, read during processing
     */
    double currentSampleRate_;           ///< Current sample rate
    int currentMaxBlockSize_;            ///< Current maximum block size

    //==============================================================================
    // Performance Monitoring (All Atomic for RT-Safety)
    
    // Basic counters
    mutable std::atomic<int> totalBlocksProcessed_{0};        ///< Total blocks counter
    mutable std::atomic<int> totalMidiEventsProcessed_{0};    ///< Total MIDI events counter
    mutable std::atomic<int> lastBlockSize_{0};               ///< Last block size
    
    // Timing statistics (in microseconds for precision)
    mutable std::atomic<long long> totalProcessingTimeMicros_{0}; ///< Cumulative processing time
    mutable std::atomic<long long> minProcessingTimeMicros_{999999}; ///< Minimum processing time
    mutable std::atomic<long long> maxProcessingTimeMicros_{0};   ///< Maximum processing time
    
    // CPU usage estimation
    mutable std::atomic<double> estimatedCpuUsage_{0.0};      ///< Current CPU usage estimate
    
    // Error counters
    mutable std::atomic<int> bufferUnderruns_{0};             ///< Buffer underrun counter
    mutable std::atomic<int> parameterUpdateErrors_{0};       ///< Parameter error counter
    mutable std::atomic<int> midiProcessingErrors_{0};        ///< MIDI error counter

    //==============================================================================
    // RT-Safe Private Methods (No Logging, No Allocation)
    
    /**
     * @brief RT-SAFE: Process MIDI events from buffer
     * @param midiMessages MIDI buffer to process
     * 
     * Handles note-on, note-off, and potentially other MIDI events.
     * Updates MIDI event counter and delegates to sampler manager.
     */
    void processMidiEvents(const juce::MidiBuffer& midiMessages) noexcept;
    
    /**
     * @brief RT-SAFE: Update sampler parameters from current parameter values
     * 
     * Reads current parameter values and applies them to the sampler system.
     * Uses atomic operations for thread-safe parameter access.
     */
    void updateSamplerParameters() noexcept;
    
    /**
     * @brief RT-SAFE: Clear audio buffer safely
     * @param buffer Buffer to clear
     * 
     * Ensures buffer is properly cleared before audio processing.
     * Handles invalid buffer conditions gracefully.
     */
    void clearAudioBuffer(juce::AudioBuffer<float>& buffer) noexcept;
    
    /**
     * @brief RT-SAFE: Update performance timing statistics
     * @param processingTimeMicros Processing time for current block in microseconds
     * @param blockSize Size of processed block
     * 
     * Updates all timing-related performance metrics atomically.
     */
    void updatePerformanceMetrics(long long processingTimeMicros, int blockSize) noexcept;
    
    /**
     * @brief RT-SAFE: Calculate CPU usage estimate
     * @param processingTimeMicros Time taken to process block
     * @param blockSize Number of samples in block
     * @return CPU usage estimate (0.0-1.0)
     */
    double calculateCpuUsage(long long processingTimeMicros, int blockSize) const noexcept;

    //==============================================================================
    // Non-RT Private Methods (Can Log, Can Allocate)
    
    /**
     * @brief Initialize performance monitoring system
     * Called during prepareToPlay to reset all counters
     */
    void initializePerformanceMonitoring();
    
    /**
     * @brief Validate audio configuration parameters
     * @param sampleRate Sample rate to validate
     * @param maxBlockSize Block size to validate
     * @return true if configuration is valid
     */
    bool validateAudioConfiguration(double sampleRate, int maxBlockSize) const;
    
    /**
     * @brief Log safe wrapper for non-RT operations
     * @param component Component name for logging
     * @param severity Log severity level
     * @param message Log message content
     */
    void logSafe(const std::string& component, const std::string& severity, 
                const std::string& message) const;

    //==============================================================================
    // Performance Analysis Helpers
    
    /**
     * @brief Calculate average processing time in milliseconds
     * @return Average processing time per block
     */
    double calculateAverageProcessingTimeMs() const noexcept;
    
    /**
     * @brief Calculate average block size
     * @return Average block size across all processed blocks
     */
    double calculateAverageBlockSize() const noexcept;
    
    /**
     * @brief Determine if timing indicates performance stress
     * @return true if processing time is approaching limits
     */
    bool isTimingStressed() const noexcept;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioProcessingEngine)
};