/**
 * @file SampleBankSelectorComponent.h
 * @brief Sample bank selector with folder picker
 *
 * ============================================================================
 * SAMPLE BANK SELECTOR COMPONENT
 * ============================================================================
 *
 * Features:
 * - Display current sample bank status (name or "Sine Wave Test Tone")
 * - "Load Sample Bank" button with file browser
 * - Simple timer updates for status display
 * - Rounded overlay (80% alpha, 6px radius)
 *
 * Layout:
 * ┌────────────────────────────────────────────┐
 * │ Sample Bank: VintageV Electric Piano       │
 * │ [Load Sample Bank...]                      │
 * └────────────────────────────────────────────┘
 * ============================================================================
 */

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "ithaca/audio/IthacaPluginProcessor.h"

/**
 * @class SampleBankSelectorComponent
 * @brief Component for selecting sample banks via folder picker
 *
 * Displays:
 * - Current sample bank name (or "Sine Wave Test Tone")
 * - Load button for opening file browser
 */
class SampleBankSelectorComponent : public juce::Component,
                                    private juce::Timer {
public:
    explicit SampleBankSelectorComponent(IthacaPluginProcessor& processor);
    ~SampleBankSelectorComponent() override;

    // ========================================================================
    // Component overrides
    // ========================================================================

    void paint(juce::Graphics& g) override;
    void resized() override;

    // ========================================================================
    // Timer override
    // ========================================================================

    void timerCallback() override;

    // ========================================================================
    // Public control
    // ========================================================================

    /**
     * @brief Start live updates (timer)
     */
    void startUpdates();

    /**
     * @brief Stop live updates (timer)
     */
    void stopUpdates();

private:
    // Reference to processor
    IthacaPluginProcessor& processorRef_;

    // ========================================================================
    // GUI Components
    // ========================================================================

    /// Label showing current sample bank name
    juce::Label sampleBankLabel_;

    /// Button to load sample bank
    juce::TextButton loadButton_;

    // ========================================================================
    // File chooser
    // ========================================================================

    std::unique_ptr<juce::FileChooser> fileChooser_;

    // ========================================================================
    // Private methods
    // ========================================================================

    /**
     * @brief Setup all GUI components (labels, button)
     */
    void setupComponents();

    /**
     * @brief Update labels with current state
     */
    void updateStatus();

    /**
     * @brief Handle load button click - open file browser
     */
    void loadButtonClicked();

    /**
     * @brief Get sample bank name from path
     * @param path Full path to sample bank directory
     * @return Directory name (last component of path)
     */
    juce::String getSampleBankNameFromPath(const juce::String& path) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SampleBankSelectorComponent)
};
