/**
 * @file InfoHeaderLabelFactory.h
 * @brief Factory for creating InfoHeader labels
 *
 * Centralizes label creation logic for consistency and DRY principle.
 * Follows same pattern as SliderFactory.
 */

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <memory>

// Forward declaration
class IthacaPluginProcessor;

/**
 * @struct InfoHeaderLabelBundle
 * @brief Bundle of all InfoHeader labels
 */
struct InfoHeaderLabelBundle {
    std::unique_ptr<juce::Label> instrumentNameLabel;
    std::unique_ptr<juce::Label> versionLabel;
    std::unique_ptr<juce::Label> sampleRateLabel;
    std::unique_ptr<juce::Label> activeVoicesLabel;
    std::unique_ptr<juce::Label> sustainingVoicesLabel;
    std::unique_ptr<juce::Label> cpuUsageLabel;
};

/**
 * @class InfoHeaderLabelFactory
 * @brief Factory for creating InfoHeaderComponent labels
 *
 * Provides:
 * - Centralized label creation
 * - Consistent styling (debug vs normal mode)
 * - Initial values from processor
 */
class InfoHeaderLabelFactory {
public:
    /**
     * @brief Create all labels for InfoHeaderComponent
     * @param processor Reference to processor for initial values
     * @param debugMode Whether debug styling should be applied
     * @return Bundle with all created labels
     */
    static InfoHeaderLabelBundle createAllLabels(IthacaPluginProcessor& processor,
                                                 bool debugMode);

    /**
     * @brief Add labels to parent component
     * @param parent Parent component to add labels to
     * @param bundle Bundle of labels to add
     */
    static void addToComponent(juce::Component& parent,
                               InfoHeaderLabelBundle& bundle);

private:
    /**
     * @brief Get initial instrument name from processor
     * @param processor Reference to processor
     * @return Instrument name or "Loading..." if not available
     */
    static juce::String getInstrumentName(IthacaPluginProcessor& processor);
};
