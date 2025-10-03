/**
 * @file IthacaPluginEditor.h (COMPLETE with MIDI Learn)
 * @brief Hlavní GUI editor s hierarchickým layoutem a MIDI Learn podporou
 * 
 * ============================================================================
 * GUI REFACTORING - MAIN EDITOR (COMPLETE WITH MIDI LEARN)
 * ============================================================================
 * 
 * Features:
 * - InfoHeaderComponent (nahoře, ~30%, 80% alpha)
 * - SliderPanelComponent (dole, ~70%, 60% alpha) with MIDI Learn
 * - Hierarchical font sizes (18px, 14px, 11px)
 * - Rounded overlays (6px radius)
 * - Separátory mezi slider řádky
 * - MIDI Learn callback integration
 * - Background image (480x650px okno)
 * - Debug mode (BACKGROUND_PICTURE_OFF)
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
 * │ ▒▒ - Right-click MIDI Learn           ▒▒ │
 * └──────────────────────────────────────────┘
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
 * @brief Hlavní GUI editor s hierarchickým layoutem a MIDI Learn
 * 
 * Responsibilities:
 * - Layout management (hierarchical 30/70 split)
 * - Component initialization (InfoHeader, SliderPanel)
 * - MIDI Learn callback registration
 * - Background image display
 * - Debug mode switching
 * - Timer lifecycle management
 * 
 * Component hierarchy:
 * IthacaPluginEditor
 * ├── imageComponent (background, full window)
 * ├── infoHeader (top 30%, InfoHeaderComponent)
 * │   ├── instrumentNameLabel (18px bold)
 * │   ├── versionLabel (14px)
 * │   ├── sampleRateLabel (11px)
 * │   ├── activeVoicesLabel (11px)
 * │   └── sustainingVoicesLabel (11px)
 * └── sliderPanel (bottom 70%, SliderPanelComponent)
 *     ├── masterGainSlider + label
 *     ├── stereoFieldSlider + label
 *     ├── lfoPanDepthSlider + label
 *     ├── lfoPanSpeedSlider + label
 *     ├── attackSlider + label
 *     ├── releaseSlider + label
 *     ├── sustainLevelSlider + label
 *     └── masterPanSlider + label
 */
class IthacaPluginEditor final : public juce::AudioProcessorEditor {
public:
    /**
     * @brief Constructor - initializes GUI with MIDI Learn support
     * @param p Reference to IthacaPluginProcessor
     */
    explicit IthacaPluginEditor(IthacaPluginProcessor& p);
    
    /**
     * @brief Destructor - unregisters MIDI Learn callbacks
     */
    ~IthacaPluginEditor() override;

    // ========================================================================
    // JUCE AudioProcessorEditor Interface
    // ========================================================================
    
    /**
     * @brief Paint background (or debug overlay)
     * @param g Graphics context
     * @note In background mode, components paint themselves over image
     */
    void paint(juce::Graphics& g) override;
    
    /**
     * @brief Layout components (hierarchical 30/70 split)
     * @note Info header: ~30% height, Slider panel: ~70% height
     */
    void resized() override;
    
    /**
     * @brief Called when parent hierarchy changes (component shown/hidden)
     * @note Starts InfoHeader timer when component becomes visible
     */
    void parentHierarchyChanged() override;

private:
    // ========================================================================
    // Processor Reference
    // ========================================================================
    
    IthacaPluginProcessor& processorRef;

    // ========================================================================
    // Hierarchické komponenty
    // ========================================================================
    
    /// Info header nahoře (~30% výšky, 80% alpha overlay)
    std::unique_ptr<InfoHeaderComponent> infoHeader;
    
    /// Slider panel dole (~70% výšky, 60% alpha overlay) with MIDI Learn
    std::unique_ptr<SliderPanelComponent> sliderPanel;
    
    /// Background image component (full window, non-interactive)
    juce::ImageComponent imageComponent;

    // ========================================================================
    // State
    // ========================================================================
    
    /// Debug mode flag (BACKGROUND_PICTURE_OFF)
    bool debugMode_;
    
    // ========================================================================
    // Setup Methods
    // ========================================================================
    
    /**
     * @brief Initialize all GUI components
     * @note Creates InfoHeaderComponent and SliderPanelComponent
     * @note Passes MidiLearnManager pointer to SliderPanel
     */
    void initializeComponents();
    
    /**
     * @brief Setup background image (if not in debug mode)
     * @note Loads ithacaplayer1.jpg from BinaryData
     * @note Sets image to stretch-to-fit and non-interactive
     */
    void setupBackground();
    
    /**
     * @brief Create fallback background when image not found
     * @note Creates gradient with text message
     */
    void createFallbackBackground();

    /**
     * @brief Register MIDI Learn callbacks with processor
     * @note Sets up lambda callback for learning state changes
     * @note Forwards state changes to SliderPanel for visual updates
     * @note Called from constructor after component initialization
     */
    void setupMidiLearnCallbacks();
    
    /**
     * @brief Check if debug mode is enabled
     * @return true if BACKGROUND_PICTURE_OFF != 0
     */
    bool isDebugModeEnabled() const;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(IthacaPluginEditor)
};