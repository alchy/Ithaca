/**
 * @file InfoHeaderComponent.h
 * @brief Horní info sekce s hierarchickým layoutem
 * 
 * ============================================================================
 * GUI REFACTORING - INFO HEADER COMPONENT (NOVÝ SOUBOR)
 * ============================================================================
 * 
 * Nahrazuje: InfoPanelComponent (legacy)
 * 
 * Layout struktura:
 * ┌────────────────────────────────────────────┐
 * │ Ithaca Grand Piano (velký font, 18px)     │
 * │ Version: 1.0.0 (střední font, 14px)       │
 * │ Sample Rate: 48000 Hz (malý font, 11px)   │
 * │ Active: 5    Sustaining: 2 (malý, 50/50)  │
 * └────────────────────────────────────────────┘
 * 
 * Features:
 * - Rounded overlay (80% alpha, 6px radius)
 * - Hierarchical font sizes (18px, 14px, 11px)
 * - Loading status display ("Loading samples...")
 * - Voice statistics (Active | Sustaining, 50/50 layout)
 * - Timer updates (300ms interval)
 * ============================================================================
 */

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "ithaca/audio/IthacaPluginProcessor.h"
#include "ithaca/gui/helpers/GuiHelpers.h"
#include "ithaca/gui/factories/InfoHeaderLabelFactory.h"
#include "ithaca/gui/layout/InfoHeaderLayout.h"

/**
 * @class InfoHeaderComponent
 * @brief Specializovaná komponenta pro info header (horní sekce GUI)
 * 
 * Zobrazuje:
 * - Instrument name (18px, bold)
 * - Version (14px)
 * - Sample rate (11px)
 * - Voice statistics: Active, Sustaining (11px, 50/50)
 * - Loading status (async loading support)
 */
class InfoHeaderComponent : public juce::Component,
                           private juce::Timer {
public:
    explicit InfoHeaderComponent(IthacaPluginProcessor& processor);
    ~InfoHeaderComponent() override;
    
    // ========================================================================
    // Component overrides
    // ========================================================================
    
    void paint(juce::Graphics&) override;
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
     * @note Volá se z IthacaPluginEditor::parentHierarchyChanged()
     */
    void startUpdates();
    
    /**
     * @brief Stop live updates (timer)
     */
    void stopUpdates();
    
    /**
     * @brief Set debug mode (změní styling labelů)
     * @param enabled true = debug colors, false = overlay colors
     */
    void setDebugMode(bool enabled);

private:
    // Reference k processoru
    IthacaPluginProcessor& processorRef_;

    // ========================================================================
    // Labels - managed by factory
    // ========================================================================

    InfoHeaderLabelBundle labelBundle_;

    // ========================================================================
    // State
    // ========================================================================

    bool debugMode_ = false;

    static constexpr int TIMER_INTERVAL_MS = 300;

    // ========================================================================
    // Private methods
    // ========================================================================

    void setupAllLabels();
    void updateLiveData();

    /**
     * @brief Get InfoHeaderLabels structure for layout
     * @return Labels structure with pointers to all labels
     */
    InfoHeaderLabels getLabelsForLayout() const;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(InfoHeaderComponent)
};