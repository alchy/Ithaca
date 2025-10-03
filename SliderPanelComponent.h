/**
 * @file SliderPanelComponent.h (REFACTORED with SliderFactory)
 * @brief Slider panel s MIDI Learn funkcionalitou - DRY princip
 */

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "GuiHelpers.h"
#include "ParameterAttachmentManager.h"
#include "MidiLearnManager.h"
#include "SliderFactory.h"
#include <vector>
#include <unordered_map>

class SliderPanelComponent : public juce::Component,
                            private juce::Slider::Listener {
public:
    explicit SliderPanelComponent(juce::AudioProcessorValueTreeState& parameters,
                                 MidiLearnManager* midiLearnManager = nullptr);
    ~SliderPanelComponent() override;

    // Component overrides
    void paint(juce::Graphics&) override;
    void resized() override;

    // Slider listener
    void sliderValueChanged(juce::Slider*) override;

    // Debug mode
    void setDebugMode(bool enabled);

    // MIDI Learn callback
    void onLearningStateChanged(bool isLearning, const juce::String& parameterID);

private:
    juce::AudioProcessorValueTreeState& parameters_;
    MidiLearnManager* midiLearnManager_;

    // REFACTORED: Jednotný kontejner pro všechny slidery (místo 16 individuálních proměnných)
    std::vector<SliderContainer> sliders_;

    // Lookup mapy (generované automaticky)
    std::unordered_map<juce::Slider*, juce::String> sliderToID_;
    std::unordered_map<juce::Slider*, juce::String> sliderToDisplayName_;
    std::unordered_map<juce::String, juce::Slider*> idToSlider_;

    ParameterAttachmentManager attachmentManager_;

    bool debugMode_ = false;
    std::vector<int> separatorPositions_;
    juce::String currentLearningParameterID_;

    // Setup methods (SIMPLIFIED)
    void setupAllControls();
    void setupSliderAttachments();

    // Layout methods
    void layoutBackgroundMode(juce::Rectangle<int> bounds);
    void layoutDebugMode(juce::Rectangle<int> bounds);
    void layoutSliderRow(juce::Rectangle<int>& bounds,
                        juce::Label* leftLabel, juce::Slider* leftSlider,
                        juce::Label* rightLabel, juce::Slider* rightSlider);
    void drawSeparator(juce::Rectangle<int>& bounds);
    void paintSeparators(juce::Graphics& g);

    // MIDI Learn methods (SIMPLIFIED - používají mapy)
    void showMidiLearnMenu(juce::Slider* slider, juce::Point<int> position);
    void updateSliderLearningVisuals(juce::Slider* slider, bool isLearning);

    // Helper pro získání sliderů podle indexu (pro layout)
    SliderContainer* getSliderByIndex(size_t index);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SliderPanelComponent)
};