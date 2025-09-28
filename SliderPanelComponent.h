/**
 * @file SliderPanelComponent.h
 * @brief Specializovaná komponenta pro všechny slidery s horizontálním layoutem
 */

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "GuiHelpers.h"
#include "ParameterAttachmentManager.h"

class SliderPanelComponent : public juce::Component,
                            private juce::Slider::Listener {
public:
    explicit SliderPanelComponent(juce::AudioProcessorValueTreeState& parameters);
    ~SliderPanelComponent() override;
    
    // Component overrides
    void paint(juce::Graphics&) override;  // Transparentní - bez pozadí
    void resized() override;               // Layout horizontálních sliderů
    
    // Slider listener
    void sliderValueChanged(juce::Slider*) override;
    
    // Debug mode control
    void setDebugMode(bool enabled);

private:
    // Reference k parametrům
    juce::AudioProcessorValueTreeState& parameters_;
    
    // ZMĚNA: Všechny slidery horizontální (LinearHorizontal)
    // Master controls
    std::unique_ptr<juce::Slider> masterGainSlider;
    std::unique_ptr<juce::Label> masterGainLabel;
    std::unique_ptr<juce::Slider> masterPanSlider;
    std::unique_ptr<juce::Label> masterPanLabel;
    
    // ADSR Envelope
    std::unique_ptr<juce::Slider> attackSlider;
    std::unique_ptr<juce::Label> attackLabel;
    std::unique_ptr<juce::Slider> releaseSlider;
    std::unique_ptr<juce::Label> releaseLabel;
    std::unique_ptr<juce::Slider> sustainLevelSlider;
    std::unique_ptr<juce::Label> sustainLevelLabel;
    
    // LFO Panning
    std::unique_ptr<juce::Slider> lfoPanSpeedSlider;
    std::unique_ptr<juce::Label> lfoPanSpeedLabel;
    std::unique_ptr<juce::Slider> lfoPanDepthSlider;
    std::unique_ptr<juce::Label> lfoPanDepthLabel;
    
    // Parameter attachments manager
    ParameterAttachmentManager attachmentManager_;
    
    // State
    bool debugMode_ = false;
    
    // Setup methods
    void setupAllControls();           // Vytvoří horizontální slidery
    void setupSliderAttachments();     // Propojí s parametry
    
    // Layout methods
    void layoutBackgroundMode(juce::Rectangle<int> bounds);  // Dva sloupce jako původně
    void layoutDebugMode(juce::Rectangle<int> bounds);       // Vertikální řazení
    
    // Helper methods
    void createMasterControls();
    void createADSRControls();
    void createLFOControls();
    
    // Disable copy
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SliderPanelComponent)
};