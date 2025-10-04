/**
 * @file InfoHeaderLayout.h
 * @brief Layout strategies for InfoHeaderComponent
 *
 * Separates layout logic from data update logic for better maintainability.
 */

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <memory>

/**
 * @struct InfoHeaderLabels
 * @brief Container for all InfoHeader labels
 */
struct InfoHeaderLabels {
    juce::Label* instrumentNameLabel = nullptr;
    juce::Label* versionLabel = nullptr;
    juce::Label* sampleRateLabel = nullptr;
    juce::Label* activeVoicesLabel = nullptr;
    juce::Label* sustainingVoicesLabel = nullptr;
    juce::Label* cpuUsageLabel = nullptr;
};

/**
 * @class InfoHeaderLayout
 * @brief Layout strategy for InfoHeaderComponent
 *
 * Provides two layout modes:
 * - Background mode: Hierarchical layout with different font sizes
 * - Debug mode: Compact vertical layout
 */
class InfoHeaderLayout {
public:
    /**
     * @enum LayoutMode
     * @brief Available layout modes
     */
    enum class LayoutMode {
        Background,  ///< Production layout with background image
        Debug        ///< Debug layout without background
    };

    /**
     * @brief Apply layout to labels
     * @param bounds Available rectangle for layout
     * @param labels Container with all labels
     * @param mode Layout mode to apply
     */
    static void applyLayout(juce::Rectangle<int> bounds,
                           const InfoHeaderLabels& labels,
                           LayoutMode mode);

private:
    /**
     * @brief Apply background mode layout
     * @param bounds Available rectangle
     * @param labels Label container
     */
    static void applyBackgroundMode(juce::Rectangle<int> bounds,
                                   const InfoHeaderLabels& labels);

    /**
     * @brief Apply debug mode layout
     * @param bounds Available rectangle
     * @param labels Label container
     */
    static void applyDebugMode(juce::Rectangle<int> bounds,
                              const InfoHeaderLabels& labels);
};
