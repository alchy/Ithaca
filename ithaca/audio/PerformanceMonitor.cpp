/**
 * @file PerformanceMonitor.cpp
 * @brief Implementation of real-time performance monitoring
 */

#include "ithaca/audio/PerformanceMonitor.h"
#include <algorithm>
#include <numeric>

//==============================================================================
// Constructor / Destructor

PerformanceMonitor::PerformanceMonitor(double sampleRate, int bufferSize)
    : sampleRate_(sampleRate),
      bufferSize_(bufferSize),
      availableTimeMs_(0.0),
      windowIndex_(0),
      windowFilled_(0),
      avgProcessingTimeMs_(0.0),
      maxProcessingTimeMs_(0.0),
      cpuUsagePercent_(0.0),
      dropoutCount_(0),
      isDropoutRisk_(false)
{
    processingTimes_.fill(0.0);
    updateAvailableTime();
}

//==============================================================================
// Configuration

void PerformanceMonitor::setAudioSettings(double sampleRate, int bufferSize)
{
    sampleRate_.store(sampleRate);
    bufferSize_.store(bufferSize);
    updateAvailableTime();
}

void PerformanceMonitor::reset()
{
    std::lock_guard<std::mutex> lock(metricsMutex_);

    processingTimes_.fill(0.0);
    windowIndex_.store(0);
    windowFilled_.store(0);
    avgProcessingTimeMs_.store(0.0);
    maxProcessingTimeMs_.store(0.0);
    cpuUsagePercent_.store(0.0);
    dropoutCount_.store(0);
    isDropoutRisk_.store(false);
}

//==============================================================================
// Measurement

void PerformanceMonitor::startMeasurement()
{
    measurementStart_ = std::chrono::high_resolution_clock::now();
}

void PerformanceMonitor::endMeasurement()
{
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
        end - measurementStart_);

    double processingTimeMs = duration.count() / 1000.0;

    updateStatistics(processingTimeMs);
}

//==============================================================================
// Query

PerformanceMonitor::PerformanceMetrics PerformanceMonitor::getMetrics() const
{
    PerformanceMetrics metrics;

    metrics.avgProcessingTimeMs = avgProcessingTimeMs_.load();
    metrics.maxProcessingTimeMs = maxProcessingTimeMs_.load();
    metrics.cpuUsagePercent = cpuUsagePercent_.load();
    metrics.dropoutCount = dropoutCount_.load();
    metrics.isDropoutRisk = isDropoutRisk_.load();

    return metrics;
}

//==============================================================================
// Private Methods

void PerformanceMonitor::updateAvailableTime()
{
    double sr = sampleRate_.load();
    int bs = bufferSize_.load();

    if (sr > 0.0 && bs > 0) {
        // Available time = (buffer size / sample rate) * 1000 ms
        double timeMs = (static_cast<double>(bs) / sr) * 1000.0;
        availableTimeMs_.store(timeMs);
    } else {
        availableTimeMs_.store(0.0);
    }
}

void PerformanceMonitor::updateStatistics(double processingTimeMs)
{
    // Update sliding window (RT-safe write)
    int index = windowIndex_.load();
    processingTimes_[index] = processingTimeMs;

    // Advance window index (circular buffer)
    int nextIndex = (index + 1) % WINDOW_SIZE;
    windowIndex_.store(nextIndex);

    // Track how many samples we have
    int filled = windowFilled_.load();
    if (filled < WINDOW_SIZE) {
        windowFilled_.store(filled + 1);
    }

    // Calculate statistics
    int sampleCount = std::min(filled + 1, WINDOW_SIZE);

    // Average processing time
    double sum = 0.0;
    double maxTime = 0.0;

    for (int i = 0; i < sampleCount; ++i) {
        double time = processingTimes_[i];
        sum += time;
        maxTime = std::max(maxTime, time);
    }

    double avgTime = sum / static_cast<double>(sampleCount);
    avgProcessingTimeMs_.store(avgTime);
    maxProcessingTimeMs_.store(maxTime);

    // CPU usage calculation
    double availableTime = availableTimeMs_.load();
    double cpuPercent = 0.0;

    if (availableTime > 0.0) {
        cpuPercent = (avgTime / availableTime) * 100.0;
        cpuUsagePercent_.store(cpuPercent);

        // Dropout detection
        if (processingTimeMs > availableTime * DROPOUT_THRESHOLD) {
            dropoutCount_.fetch_add(1);
        }

        // Warning threshold
        if (cpuPercent > (WARNING_THRESHOLD * 100.0)) {
            isDropoutRisk_.store(true);
        } else {
            isDropoutRisk_.store(false);
        }
    }
}
