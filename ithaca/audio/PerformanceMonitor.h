/**
 * @file PerformanceMonitor.h
 * @brief Real-time audio performance monitoring and dropout detection
 *
 * Thread-safe performance metrics for audio processing monitoring.
 * Tracks processing time, CPU usage, and dropout detection.
 */

#pragma once

#include <atomic>
#include <chrono>
#include <array>
#include <mutex>

/**
 * @class PerformanceMonitor
 * @brief RT-safe performance monitoring for audio processing
 *
 * Features:
 * - High-resolution timing of processBlock() calls
 * - Sliding window average for CPU usage
 * - Dropout detection (processing time > available time)
 * - Thread-safe read access for GUI
 * - Minimal overhead in RT thread
 */
class PerformanceMonitor {
public:
    /**
     * @struct PerformanceMetrics
     * @brief Thread-safe snapshot of performance data
     */
    struct PerformanceMetrics {
        double avgProcessingTimeMs = 0.0;      ///< Average processing time (ms)
        double maxProcessingTimeMs = 0.0;      ///< Peak processing time (ms)
        double cpuUsagePercent = 0.0;          ///< CPU usage percentage
        int dropoutCount = 0;                  ///< Total dropout count
        bool isDropoutRisk = false;            ///< Warning flag (>80% CPU)
    };

    /**
     * @brief Constructor
     * @param sampleRate Audio sample rate
     * @param bufferSize Audio buffer size (samples per block)
     */
    PerformanceMonitor(double sampleRate = 48000.0, int bufferSize = 512);

    /**
     * @brief Update audio settings (call from prepareToPlay)
     * @param sampleRate New sample rate
     * @param bufferSize New buffer size
     */
    void setAudioSettings(double sampleRate, int bufferSize);

    /**
     * @brief Start timing measurement (call at start of processBlock)
     */
    void startMeasurement();

    /**
     * @brief End timing measurement (call at end of processBlock)
     */
    void endMeasurement();

    /**
     * @brief Get current performance metrics (thread-safe)
     * @return Snapshot of performance data
     */
    PerformanceMetrics getMetrics() const;

    /**
     * @brief Reset all statistics
     */
    void reset();

private:
    // Audio settings
    std::atomic<double> sampleRate_;
    std::atomic<int> bufferSize_;
    std::atomic<double> availableTimeMs_;  ///< Time available per block

    // Timing
    std::chrono::high_resolution_clock::time_point measurementStart_;

    // Statistics (sliding window)
    static constexpr int WINDOW_SIZE = 100;  ///< Number of samples for averaging
    std::array<double, WINDOW_SIZE> processingTimes_;
    std::atomic<int> windowIndex_;
    std::atomic<int> windowFilled_;

    // Metrics (atomic for thread safety)
    std::atomic<double> avgProcessingTimeMs_;
    std::atomic<double> maxProcessingTimeMs_;
    std::atomic<double> cpuUsagePercent_;
    std::atomic<int> dropoutCount_;
    std::atomic<bool> isDropoutRisk_;

    // Mutex for window array access (non-RT read only)
    mutable std::mutex metricsMutex_;

    // Thresholds
    static constexpr double WARNING_THRESHOLD = 0.80;  ///< 80% CPU = warning
    static constexpr double DROPOUT_THRESHOLD = 1.00;  ///< 100% CPU = dropout

    /**
     * @brief Calculate available time per audio block
     */
    void updateAvailableTime();

    /**
     * @brief Update statistics from sliding window
     */
    void updateStatistics(double processingTimeMs);
};
