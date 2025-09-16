#include <algorithm>
#include <chrono>

#include "sampler/core_logger.h"
#include "AudioProcessingEngine.h"

//==============================================================================
AudioProcessingEngine::AudioProcessingEngine(SamplerManager& samplerManager, 
                                           ParameterManager& parameterManager)
    : samplerManager_(samplerManager),
      parameterManager_(parameterManager),
      currentSampleRate_(0.0),
      currentMaxBlockSize_(0)
{
    // Create temporary logger for initialization
    auto logger = std::make_unique<Logger>(".");
    logSafe("AudioProcessingEngine/constructor", "info", 
           "=== AUDIO PROCESSING ENGINE INITIALIZING ===");
    
    logSafe("AudioProcessingEngine/constructor", "info", 
           "Connecting to SamplerManager and ParameterManager...");
    
    // Validate manager references
    if (&samplerManager_ && &parameterManager_) {
        logSafe("AudioProcessingEngine/constructor", "info", 
               "Manager references established successfully");
    } else {
        logSafe("AudioProcessingEngine/constructor", "error", 
               "Invalid manager references - engine may not function properly");
    }
    
    // Initialize performance monitoring
    initializePerformanceMonitoring();
    
    logSafe("AudioProcessingEngine/constructor", "info", 
           "Performance monitoring initialized");
    logSafe("AudioProcessingEngine/constructor", "info", 
           "=== AUDIO PROCESSING ENGINE READY ===");
}

//==============================================================================
// Audio Processing Lifecycle

void AudioProcessingEngine::prepareToPlay(double sampleRate, int maxBlockSize)
{
    auto logger = std::make_unique<Logger>(".");
    logSafe("AudioProcessingEngine/prepareToPlay", "info", 
           "=== PREPARING AUDIO PROCESSING ENGINE ===");
    logSafe("AudioProcessingEngine/prepareToPlay", "info", 
           "Configuration: " + std::to_string(sampleRate) + " Hz, " + 
           std::to_string(maxBlockSize) + " samples");
    
    // Validate configuration
    if (!validateAudioConfiguration(sampleRate, maxBlockSize)) {
        logSafe("AudioProcessingEngine/prepareToPlay", "error", 
               "Invalid audio configuration - cannot prepare for processing");
        return;
    }
    
    // Store configuration
    currentSampleRate_ = sampleRate;
    currentMaxBlockSize_ = maxBlockSize;
    
    logSafe("AudioProcessingEngine/prepareToPlay", "info", 
           "Audio configuration validated and stored");
    
    // Initialize sampler system if not already done
    if (!samplerManager_.isInitialized()) {
        logSafe("AudioProcessingEngine/prepareToPlay", "info", 
               "Sampler not initialized - triggering initialization...");
        
        bool samplerInitialized = samplerManager_.initialize(sampleRate, maxBlockSize);
        if (samplerInitialized) {
            logSafe("AudioProcessingEngine/prepareToPlay", "info", 
                   "Sampler system initialized successfully");
        } else {
            logSafe("AudioProcessingEngine/prepareToPlay", "error", 
                   "Sampler initialization failed - audio processing will be disabled");
            return;
        }
    } else {
        logSafe("AudioProcessingEngine/prepareToPlay", "info", 
               "Sampler already initialized - checking configuration compatibility...");
        
        auto stats = samplerManager_.getStats();
        if (stats.currentSampleRate != static_cast<int>(sampleRate)) {
            logSafe("AudioProcessingEngine/prepareToPlay", "info", 
                   "Sample rate mismatch - reinitializing sampler...");
            samplerManager_.changeSampleRate(sampleRate);
        }
    }
    
    // Reset performance monitoring for new session
    logSafe("AudioProcessingEngine/prepareToPlay", "info", 
           "Resetting performance monitoring for new audio session...");
    resetStats();
    
    // Enable processing
    isProcessingEnabled_.store(true);
    
    logSafe("AudioProcessingEngine/prepareToPlay", "info", 
           "=== AUDIO PROCESSING ENGINE READY FOR REAL-TIME OPERATION ===");
    logSafe("AudioProcessingEngine/prepareToPlay", "info", 
           "RT-mode enabled - no more logging in processBlock()");
}

void AudioProcessingEngine::releaseResources()
{
    auto logger = std::make_unique<Logger>(".");
    logSafe("AudioProcessingEngine/releaseResources", "info", 
           "=== RELEASING AUDIO PROCESSING RESOURCES ===");
    
    // Disable processing first
    isProcessingEnabled_.store(false);
    logSafe("AudioProcessingEngine/releaseResources", "info", 
           "Processing disabled - safe to release resources");
    
    // Stop all voices safely
    logSafe("AudioProcessingEngine/releaseResources", "info", 
           "Stopping all active voices...");
    samplerManager_.stopAllVoices();
    
    // Get final performance statistics before cleanup
    auto finalStats = getStats();
    logSafe("AudioProcessingEngine/releaseResources", "info", 
           "Final session statistics:");
    logSafe("AudioProcessingEngine/releaseResources", "info", 
           "Total blocks processed: " + std::to_string(finalStats.totalBlocksProcessed));
    logSafe("AudioProcessingEngine/releaseResources", "info", 
           "Total MIDI events: " + std::to_string(finalStats.totalMidiEventsProcessed));
    logSafe("AudioProcessingEngine/releaseResources", "info", 
           "Average CPU usage: " + std::to_string(finalStats.currentCpuUsage * 100.0) + "%");
    
    if (finalStats.bufferUnderruns > 0 || finalStats.parameterUpdateErrors > 0 || finalStats.midiProcessingErrors > 0) {
        logSafe("AudioProcessingEngine/releaseResources", "warn", 
               "Session had errors - Underruns: " + std::to_string(finalStats.bufferUnderruns) + 
               ", Parameter errors: " + std::to_string(finalStats.parameterUpdateErrors) + 
               ", MIDI errors: " + std::to_string(finalStats.midiProcessingErrors));
    }
    
    logSafe("AudioProcessingEngine/releaseResources", "info", 
           "=== AUDIO PROCESSING RESOURCES RELEASED ===");
}

void AudioProcessingEngine::processBlock(juce::AudioBuffer<float>& buffer, 
                                       juce::MidiBuffer& midiMessages) noexcept
{
    // RT-SAFE: No logging, no allocation, no exceptions beyond this point
    
    // Start timing for performance monitoring
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Early exit if not enabled
    if (!isProcessingEnabled_.load()) {
        clearAudioBuffer(buffer);
        return;
    }
    
    // Get VoiceManager (RT-safe operation)
    auto* voiceManager = samplerManager_.getVoiceManager();
    if (!voiceManager) {
        clearAudioBuffer(buffer);
        bufferUnderruns_.fetch_add(1);
        return;
    }
    
    // Update performance statistics
    const int numSamples = buffer.getNumSamples();
    totalBlocksProcessed_.fetch_add(1);
    lastBlockSize_.store(numSamples);
    
    // Clear output buffer first
    clearAudioBuffer(buffer);
    
    // Process MIDI events
    try {
        processMidiEvents(midiMessages);
    } catch (...) {
        midiProcessingErrors_.fetch_add(1);
        // Continue processing even if MIDI fails
    }
    
    // Update sampler parameters from JUCE parameters
    try {
        updateSamplerParameters();
    } catch (...) {
        parameterUpdateErrors_.fetch_add(1);
        // Continue processing even if parameter update fails
    }
    
    // Process audio through VoiceManager
    const int numChannels = buffer.getNumChannels();
    if (numChannels >= 2 && numSamples > 0) {
        float* leftChannel = buffer.getWritePointer(0);
        float* rightChannel = buffer.getWritePointer(1);
        
        if (leftChannel && rightChannel) {
            // Process audio block through IthacaCore VoiceManager
            voiceManager->processBlockUninterleaved(leftChannel, rightChannel, numSamples);
        } else {
            bufferUnderruns_.fetch_add(1);
        }
    } else {
        bufferUnderruns_.fetch_add(1);
    }
    
    // End timing and update performance metrics
    auto endTime = std::chrono::high_resolution_clock::now();
    auto processingTime = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    updatePerformanceMetrics(processingTime.count(), numSamples);
}

//==============================================================================
// Performance Monitoring and Statistics

AudioProcessingEngine::ProcessingStats AudioProcessingEngine::getStats() const noexcept
{
    ProcessingStats stats;
    
    // Basic counters
    stats.totalBlocksProcessed = totalBlocksProcessed_.load();
    stats.totalMidiEventsProcessed = totalMidiEventsProcessed_.load();
    stats.lastBlockSize = lastBlockSize_.load();
    stats.lastSampleRate = currentSampleRate_;
    stats.isProcessingEnabled = isProcessingEnabled_.load();
    
    // Timing statistics
    long long totalTimeMicros = totalProcessingTimeMicros_.load();
    long long minTimeMicros = minProcessingTimeMicros_.load();
    long long maxTimeMicros = maxProcessingTimeMicros_.load();
    
    if (stats.totalBlocksProcessed > 0) {
        stats.avgProcessingTimeMs = (totalTimeMicros / static_cast<double>(stats.totalBlocksProcessed)) / 1000.0;
        stats.averageBlockSize = calculateAverageBlockSize();
    }
    
    stats.minProcessingTimeMs = (minTimeMicros == 999999) ? 0.0 : minTimeMicros / 1000.0;
    stats.maxProcessingTimeMs = maxTimeMicros / 1000.0;
    
    // CPU usage and error statistics
    stats.currentCpuUsage = estimatedCpuUsage_.load();
    stats.bufferUnderruns = bufferUnderruns_.load();
    stats.parameterUpdateErrors = parameterUpdateErrors_.load();
    stats.midiProcessingErrors = midiProcessingErrors_.load();
    
    return stats;
}

void AudioProcessingEngine::resetStats() noexcept
{
    auto logger = std::make_unique<Logger>(".");
    logSafe("AudioProcessingEngine/resetStats", "info", 
           "Resetting all performance statistics...");
    
    totalBlocksProcessed_.store(0);
    totalMidiEventsProcessed_.store(0);
    lastBlockSize_.store(0);
    totalProcessingTimeMicros_.store(0);
    minProcessingTimeMicros_.store(999999);
    maxProcessingTimeMicros_.store(0);
    estimatedCpuUsage_.store(0.0);
    bufferUnderruns_.store(0);
    parameterUpdateErrors_.store(0);
    midiProcessingErrors_.store(0);
    
    logSafe("AudioProcessingEngine/resetStats", "info", 
           "Performance statistics reset completed");
}

void AudioProcessingEngine::logPerformanceStatistics() const
{
    auto logger = std::make_unique<Logger>(".");
    auto stats = getStats();
    
    logSafe("AudioProcessingEngine/statistics", "info", 
           "=== AUDIO PROCESSING ENGINE STATISTICS ===");
    logSafe("AudioProcessingEngine/statistics", "info", 
           "Processing Status: " + std::string(stats.isProcessingEnabled ? "Enabled" : "Disabled"));
    logSafe("AudioProcessingEngine/statistics", "info", 
           "Sample Rate: " + std::to_string(stats.lastSampleRate) + " Hz");
    logSafe("AudioProcessingEngine/statistics", "info", 
           "Last Block Size: " + std::to_string(stats.lastBlockSize) + " samples");
    logSafe("AudioProcessingEngine/statistics", "info", 
           "Average Block Size: " + std::to_string(stats.averageBlockSize) + " samples");
    
    logSafe("AudioProcessingEngine/statistics", "info", 
           "Performance Metrics:");
    logSafe("AudioProcessingEngine/statistics", "info", 
           "  Total Blocks Processed: " + std::to_string(stats.totalBlocksProcessed));
    logSafe("AudioProcessingEngine/statistics", "info", 
           "  Total MIDI Events: " + std::to_string(stats.totalMidiEventsProcessed));
    logSafe("AudioProcessingEngine/statistics", "info", 
           "  Current CPU Usage: " + std::to_string(stats.currentCpuUsage * 100.0) + "%");
    
    logSafe("AudioProcessingEngine/statistics", "info", 
           "Processing Time Statistics:");
    logSafe("AudioProcessingEngine/statistics", "info", 
           "  Average: " + std::to_string(stats.avgProcessingTimeMs) + " ms");
    logSafe("AudioProcessingEngine/statistics", "info", 
           "  Minimum: " + std::to_string(stats.minProcessingTimeMs) + " ms");
    logSafe("AudioProcessingEngine/statistics", "info", 
           "  Maximum: " + std::to_string(stats.maxProcessingTimeMs) + " ms");
    
    if (stats.bufferUnderruns > 0 || stats.parameterUpdateErrors > 0 || stats.midiProcessingErrors > 0) {
        logSafe("AudioProcessingEngine/statistics", "warn", 
               "Error Counts:");
        logSafe("AudioProcessingEngine/statistics", "warn", 
               "  Buffer Underruns: " + std::to_string(stats.bufferUnderruns));
        logSafe("AudioProcessingEngine/statistics", "warn", 
               "  Parameter Update Errors: " + std::to_string(stats.parameterUpdateErrors));
        logSafe("AudioProcessingEngine/statistics", "warn", 
               "  MIDI Processing Errors: " + std::to_string(stats.midiProcessingErrors));
    } else {
        logSafe("AudioProcessingEngine/statistics", "info", 
               "No processing errors detected");
    }
    
    // Performance health assessment
    double healthScore = getPerformanceHealthScore();
    std::string healthStatus;
    if (healthScore >= 0.9) healthStatus = "Excellent";
    else if (healthScore >= 0.7) healthStatus = "Good";
    else if (healthScore >= 0.5) healthStatus = "Fair";
    else healthStatus = "Poor";
    
    logSafe("AudioProcessingEngine/statistics", "info", 
           "Performance Health: " + healthStatus + " (" + std::to_string(healthScore * 100.0) + "%)");
    logSafe("AudioProcessingEngine/statistics", "info", 
           "Recommended Max Polyphony: " + std::to_string(getRecommendedMaxPolyphony()));
    
    logSafe("AudioProcessingEngine/statistics", "info", 
           "==========================================");
}

//==============================================================================
// Advanced Performance Analysis

bool AudioProcessingEngine::isPerformanceStressed() const noexcept
{
    return (estimatedCpuUsage_.load() > 0.8) || // High CPU usage
           isTimingStressed() ||                // Timing issues
           (bufferUnderruns_.load() > 0);       // Buffer problems
}

double AudioProcessingEngine::getPerformanceHealthScore() const noexcept
{
    double score = 1.0;
    
    // CPU usage penalty
    double cpuUsage = estimatedCpuUsage_.load();
    if (cpuUsage > 0.5) {
        score *= (1.0 - (cpuUsage - 0.5) * 2.0); // Reduce score for high CPU
    }
    
    // Error penalty
    int totalErrors = bufferUnderruns_.load() + parameterUpdateErrors_.load() + midiProcessingErrors_.load();
    int totalBlocks = totalBlocksProcessed_.load();
    if (totalBlocks > 0 && totalErrors > 0) {
        double errorRate = static_cast<double>(totalErrors) / totalBlocks;
        score *= (1.0 - errorRate * 10.0); // Heavy penalty for errors
    }
    
    // Timing consistency penalty
    if (isTimingStressed()) {
        score *= 0.7; // Reduce score for timing issues
    }
    
    return std::max(0.0, std::min(1.0, score));
}

int AudioProcessingEngine::getRecommendedMaxPolyphony() const noexcept
{
    double cpuUsage = estimatedCpuUsage_.load();
    double healthScore = getPerformanceHealthScore();
    
    // Base recommendation on current performance
    int basePolyphony = 64; // Conservative default
    
    if (healthScore > 0.9 && cpuUsage < 0.3) {
        return 128; // Full polyphony
    } else if (healthScore > 0.7 && cpuUsage < 0.6) {
        return 96;  // High polyphony
    } else if (healthScore > 0.5 && cpuUsage < 0.8) {
        return 64;  // Medium polyphony
    } else {
        return 32;  // Conservative polyphony
    }
}

//==============================================================================
// RT-Safe Private Methods

void AudioProcessingEngine::processMidiEvents(const juce::MidiBuffer& midiMessages) noexcept
{
    for (const auto& midiMetadata : midiMessages) {
        auto message = midiMetadata.getMessage();
        totalMidiEventsProcessed_.fetch_add(1);
        
        if (message.isNoteOn()) {
            uint8_t midiNote = static_cast<uint8_t>(message.getNoteNumber());
            uint8_t velocity = static_cast<uint8_t>(message.getVelocity());
            samplerManager_.processMidiNote(midiNote, true, velocity);
        }
        else if (message.isNoteOff()) {
            uint8_t midiNote = static_cast<uint8_t>(message.getNoteNumber());
            samplerManager_.processMidiNote(midiNote, false);
        }
        // Future expansion: CC messages, pitch bend, etc.
    }
}

void AudioProcessingEngine::updateSamplerParameters() noexcept
{
    // RT-SAFE parameter updates (no logging in RT context)
    samplerManager_.setMasterGain(parameterManager_.getMasterGainMIDI());
    samplerManager_.setMasterPan(parameterManager_.getMasterPanMIDI());
}

void AudioProcessingEngine::clearAudioBuffer(juce::AudioBuffer<float>& buffer) noexcept
{
    buffer.clear();
}

void AudioProcessingEngine::updatePerformanceMetrics(long long processingTimeMicros, int blockSize) noexcept
{
    // Update cumulative timing
    totalProcessingTimeMicros_.fetch_add(processingTimeMicros);
    
    // Update min/max timing
    long long currentMin = minProcessingTimeMicros_.load();
    while (processingTimeMicros < currentMin && 
           !minProcessingTimeMicros_.compare_exchange_weak(currentMin, processingTimeMicros)) {
        // Retry if another thread updated the value
    }
    
    long long currentMax = maxProcessingTimeMicros_.load();
    while (processingTimeMicros > currentMax && 
           !maxProcessingTimeMicros_.compare_exchange_weak(currentMax, processingTimeMicros)) {
        // Retry if another thread updated the value
    }
    
    // Update CPU usage estimate
    double cpuUsage = calculateCpuUsage(processingTimeMicros, blockSize);
    estimatedCpuUsage_.store(cpuUsage);
}

double AudioProcessingEngine::calculateCpuUsage(long long processingTimeMicros, int blockSize) const noexcept
{
    if (currentSampleRate_ <= 0.0 || blockSize <= 0) return 0.0;
    
    // Calculate time available for this block
    double blockDurationMicros = (blockSize / currentSampleRate_) * 1000000.0;
    
    // Calculate CPU usage as ratio of processing time to available time
    double cpuUsage = processingTimeMicros / blockDurationMicros;
    
    // Clamp to reasonable range
    return std::max(0.0, std::min(1.0, cpuUsage));
}

//==============================================================================
// Non-RT Private Methods

void AudioProcessingEngine::initializePerformanceMonitoring()
{
    resetStats();
}

bool AudioProcessingEngine::validateAudioConfiguration(double sampleRate, int maxBlockSize) const
{
    // Validate sample rate
    if (sampleRate < ITHACA_MIN_SAMPLE_RATE || sampleRate > ITHACA_MAX_SAMPLE_RATE) {
        return false;
    }
    
    // Validate block size
    if (maxBlockSize < ITHACA_MIN_JUCE_BLOCK_SIZE || maxBlockSize > ITHACA_MAX_JUCE_BLOCK_SIZE) {
        return false;
    }
    
    return true;
}

void AudioProcessingEngine::logSafe(const std::string& component, const std::string& severity, 
                                  const std::string& message) const
{
    auto logger = std::make_unique<Logger>(".");
    if (logger) {
        logger->log(component, severity, message);
    }
}

//==============================================================================
// Performance Analysis Helpers

double AudioProcessingEngine::calculateAverageProcessingTimeMs() const noexcept
{
    int totalBlocks = totalBlocksProcessed_.load();
    if (totalBlocks == 0) return 0.0;
    
    long long totalTime = totalProcessingTimeMicros_.load();
    return (totalTime / static_cast<double>(totalBlocks)) / 1000.0;
}

double AudioProcessingEngine::calculateAverageBlockSize() const noexcept
{
    // Simple approximation - in a real implementation, we'd track this more precisely
    return static_cast<double>(lastBlockSize_.load());
}

bool AudioProcessingEngine::isTimingStressed() const noexcept
{
    long long maxTime = maxProcessingTimeMicros_.load();
    long long avgTime = totalBlocksProcessed_.load() > 0 ? 
                       totalProcessingTimeMicros_.load() / totalBlocksProcessed_.load() : 0;
    
    // Check if max time is significantly higher than average (indicates timing spikes)
    return maxTime > (avgTime * 3);
}