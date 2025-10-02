/**
 * @file IthacaPluginEditor.h
 * @brief Hlavní GUI editor s hierarchickým layoutem
 * 
 * ============================================================================
 * GUI REFACTORING - MAIN EDITOR (CLEAN VERSION - BEZ LEGACY)
 * ============================================================================
 * 
 * Změny v této verzi:
 * - InfoHeaderComponent (nahoře, ~30%, 80% alpha)
 * - SliderPanelComponent (dole, ~70%, 60% alpha)
 * - Hierarchical font sizes (18px, 14px, 11px)
 * - Rounded overlays (6px radius)
 * - Separátory mezi slider řádky
 * - ODSTRANĚNO: VoiceActivityComponent (legacy)
 * 
 * Zachováno:
 * - Background image (480x650px okno)
 * - Debug mode (BACKGROUND_PICTURE_OFF)
 * - Parameter attachments
 * - Timer management
 * ============================================================================
 */

#pragma once

#include "IthacaPluginProcessor.h"
#include <juce_gui_basics/juce_gui_basics.h>

// Forward declarations
class SliderPanelComponent;
class InfoHeaderComponent;

/**
 * @class IthacaPluginEditor
 * @brief Hlavní GUI editor s hierarchickým layoutem
 * 
 * Layout:
 * ┌──────────────────────────────────────────┐
 * │ ▓▓ INFO HEADER (~30%, 80% alpha)      ▓▓ │
 * │ ▓▓ - Ithaca Grand Piano (18px)        ▓▓ │
 * │ ▓▓ - Version, Sample Rate, Stats      ▓▓ │
 * ├──────────────────────────────────────────┤
 * │ ▒▒ SLIDER PANEL (~70%, 60% alpha)     ▒▒ │
 * │ ▒▒ - 4 řádky sliderů (50/50 split)    ▒▒ │
 * │ ▒▒ - Separátory mezi řádky            ▒▒ │
 * └──────────────────────────────────────────┘
 */
class IthacaPluginEditor final : public juce::AudioProcessorEditor {
public:
    explicit IthacaPluginEditor(IthacaPluginProcessor&);
    ~IthacaPluginEditor() override;

    // ========================================================================
    // JUCE AudioProcessorEditor Interface
    // ========================================================================
    
    void paint(juce::Graphics&) override;
    void resized() override;
    void parentHierarchyChanged() override;

private:
    IthacaPluginProcessor& processorRef;

    // ========================================================================
    // Hierarchické komponenty
    // ========================================================================
    
    /// Info header nahoře (~30% výšky)
    std::unique_ptr<InfoHeaderComponent> infoHeader;
    
    /// Slider panel dole (~70% výšky)
    std::unique_ptr<SliderPanelComponent> sliderPanel;
    
    /// Background image component
    juce::ImageComponent imageComponent;

    // ========================================================================
    // State
    // ========================================================================
    
    bool debugMode_;  // BACKGROUND_PICTURE_OFF flag
    
    // ========================================================================
    // Setup Methods
    // ========================================================================
    
    void initializeComponents();
    void setupBackground();
    bool isDebugModeEnabled() const;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(IthacaPluginEditor)
};