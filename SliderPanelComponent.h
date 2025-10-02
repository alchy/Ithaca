/**
 * @file SliderPanelComponent.h (COMPLETE with MidiLearnSlider)
 * @brief Slider panel s MIDI Learn funkcionalitou
 */

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "GuiHelpers.h"
#include "ParameterAttachmentManager.h"
#include "MidiLearnManager.h"
#include "MidiLearnSlider.h"  // Custom slider with right-click support

class SliderPanelComponent : public juce::Component,
                            private juce::Slider::Listener {
public:
    explicit SliderPanelComponent(juce::AudioProcessorValueTreeState& parameters,
                                 MidiLearnManager* midiLearnManager = nullptr);
    ~SliderPanelComponent() override;
    
    // Component overrides
    void paint(juce::Graphics&) override;
    void resized() override;
    
    // NOTE: mouseDown() REMOVED - handled by MidiLearnSlider now
    
    // Slider listener
    void sliderValueChanged(juce::Slider*) override;
    
    // Debug mode
    void setDebugMode(bool enabled);
    
    // MIDI Learn callback
    void onLearningStateChanged(bool isLearning, const juce::String& parameterID);

private:
    juce::AudioProcessorValueTreeState& parameters_;
    MidiLearnManager* midiLearnManager_;
    
    // CHANGED: Všechny slidery jsou nyní MidiLearnSlider
    std::unique_ptr<MidiLearnSlider> masterGainSlider;
    std::unique_ptr<juce::Label> masterGainLabel;
    std::unique_ptr<MidiLearnSlider> stereoFieldSlider;
    std::unique_ptr<juce::Label> stereoFieldLabel;
    
    std::unique_ptr<MidiLearnSlider> lfoPanDepthSlider;
    std::unique_ptr<juce::Label> lfoPanDepthLabel;
    std::unique_ptr<MidiLearnSlider> lfoPanSpeedSlider;
    std::unique_ptr<juce::Label> lfoPanSpeedLabel;
    
    std::unique_ptr<MidiLearnSlider> attackSlider;
    std::unique_ptr<juce::Label> attackLabel;
    std::unique_ptr<MidiLearnSlider> releaseSlider;
    std::unique_ptr<juce::Label> releaseLabel;
    
    std::unique_ptr<MidiLearnSlider> sustainLevelSlider;
    std::unique_ptr<juce::Label> sustainLevelLabel;
    std::unique_ptr<MidiLearnSlider> masterPanSlider;
    std::unique_ptr<juce::Label> masterPanLabel;
    
    ParameterAttachmentManager attachmentManager_;
    
    bool debugMode_ = false;
    std::vector<int> separatorPositions_;
    
    // MIDI Learn state
    juce::String currentLearningParameterID_;
    
    // Setup methods
    void setupAllControls();
    void setupSliderAttachments();
    void createMasterControls();
    void createLFOControls();
    void createADSRControls();
    void createPanControl();
    
    // Layout methods
    void layoutBackgroundMode(juce::Rectangle<int> bounds);
    void layoutDebugMode(juce::Rectangle<int> bounds);
    void layoutSliderRow(juce::Rectangle<int>& bounds,
                        juce::Label* leftLabel, juce::Slider* leftSlider,
                        juce::Label* rightLabel, juce::Slider* rightSlider);
    void drawSeparator(juce::Rectangle<int>& bounds);
    void paintSeparators(juce::Graphics& g);
    
    // MIDI Learn methods
    void showMidiLearnMenu(juce::Slider* slider, juce::Point<int> position);
    juce::String getParameterIDForSlider(juce::Slider* slider) const;
    juce::String getDisplayNameForSlider(juce::Slider* slider) const;
    void updateSliderLearningVisuals(juce::Slider* slider, bool isLearning);
    juce::Slider* findSliderByParameterID(const juce::String& parameterID);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SliderPanelComponent)
};