/**
 * @file SliderPanelComponent.h
 * @brief Specializovaná komponenta pro hierarchický slider layout
 * 
 * ============================================================================
 * GUI REFACTORING - SLIDER PANEL COMPONENT (AKTUALIZOVÁNO)
 * ============================================================================
 * 
 * Nový layout (4 řádky, 50/50 split):
 * - Row 1: Master Gain | Stereo Field
 * - Row 2: LFO Depth | LFO Speed
 * - Row 3: Attack | Release
 * - Row 4: Sustain | Master Pan
 * 
 * Features:
 * - Rounded overlay (60% alpha, 6px radius)
 * - Separátory mezi řádky (60% alpha, 1px)
 * - Labels nad slidery (střední font, 14px)
 * - 50/50 column split pro všechny řádky
 * - Parameter attachments (JUCE APVTS)
 * ============================================================================
 */

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "GuiHelpers.h"
#include "ParameterAttachmentManager.h"

/**
 * @class SliderPanelComponent
 * @brief Dolní sekce GUI s 4 řádky sliderů (50/50 layout)
 */
class SliderPanelComponent : public juce::Component,
                            private juce::Slider::Listener {
public:
    explicit SliderPanelComponent(juce::AudioProcessorValueTreeState& parameters);
    ~SliderPanelComponent() override;
    
    // ========================================================================
    // Component overrides
    // ========================================================================
    
    void paint(juce::Graphics&) override;
    void resized() override;
    
    // ========================================================================
    // Slider listener
    // ========================================================================
    
    void sliderValueChanged(juce::Slider*) override;
    
    // ========================================================================
    // Public control
    // ========================================================================
    
    void setDebugMode(bool enabled);

private:
    // Reference k parametrům
    juce::AudioProcessorValueTreeState& parameters_;
    
    // ========================================================================
    // ROW 1: Master Controls (50/50)
    // ========================================================================
    
    std::unique_ptr<juce::Slider> masterGainSlider;
    std::unique_ptr<juce::Label> masterGainLabel;
    std::unique_ptr<juce::Slider> stereoFieldSlider;
    std::unique_ptr<juce::Label> stereoFieldLabel;
    
    // ========================================================================
    // ROW 2: LFO Controls (50/50)
    // ========================================================================
    
    std::unique_ptr<juce::Slider> lfoPanDepthSlider;
    std::unique_ptr<juce::Label> lfoPanDepthLabel;
    std::unique_ptr<juce::Slider> lfoPanSpeedSlider;
    std::unique_ptr<juce::Label> lfoPanSpeedLabel;
    
    // ========================================================================
    // ROW 3: ADSR Controls - Attack, Release (50/50)
    // ========================================================================
    
    std::unique_ptr<juce::Slider> attackSlider;
    std::unique_ptr<juce::Label> attackLabel;
    std::unique_ptr<juce::Slider> releaseSlider;
    std::unique_ptr<juce::Label> releaseLabel;
    
    // ========================================================================
    // ROW 4: ADSR (Sustain) + Pan (50/50)
    // ========================================================================
    
    std::unique_ptr<juce::Slider> sustainLevelSlider;
    std::unique_ptr<juce::Label> sustainLevelLabel;
    std::unique_ptr<juce::Slider> masterPanSlider;
    std::unique_ptr<juce::Label> masterPanLabel;
    
    // ========================================================================
    // Parameter attachments
    // ========================================================================
    
    ParameterAttachmentManager attachmentManager_;
    
    // ========================================================================
    // State
    // ========================================================================
    
    bool debugMode_ = false;
    std::vector<int> separatorPositions_;  // Y pozice separátorů
    
    // ========================================================================
    // Setup methods
    // ========================================================================
    
    void setupAllControls();
    void setupSliderAttachments();
    
    // Control creation (organizované po řádcích)
    void createMasterControls();   // Row 1
    void createLFOControls();       // Row 2
    void createADSRControls();      // Row 3-4
    void createPanControl();        // Row 4
    
    // ========================================================================
    // Layout methods
    // ========================================================================
    
    void layoutBackgroundMode(juce::Rectangle<int> bounds);
    void layoutDebugMode(juce::Rectangle<int> bounds);
    
    // ========================================================================
    // Helper methods
    // ========================================================================
    
    /**
     * @brief Rozloží jeden řádek sliderů (50/50 split)
     * @param bounds Reference na dostupný prostor (bude změněna)
     * @param leftLabel Label pro levý slider
     * @param leftSlider Levý slider
     * @param rightLabel Label pro pravý slider
     * @param rightSlider Pravý slider
     */
    void layoutSliderRow(juce::Rectangle<int>& bounds,
                        juce::Label* leftLabel, juce::Slider* leftSlider,
                        juce::Label* rightLabel, juce::Slider* rightSlider);
    
    /**
     * @brief Přidá separator do pozice (uloží Y pozici pro paint())
     * @param bounds Reference na bounds (odebere prostor pro separator)
     */
    void drawSeparator(juce::Rectangle<int>& bounds);
    
    /**
     * @brief Vykreslí všechny uložené separátory
     * @param g Graphics context
     */
    void paintSeparators(juce::Graphics& g);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SliderPanelComponent)
};