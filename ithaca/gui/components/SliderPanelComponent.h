/**
 * @file SliderPanelComponent.h (REFACTORED with SliderFactory)
 * @brief Slider panel s MIDI Learn funkcionalitou - využívá SliderFactory
 */

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "GuiHelpers.h"
#include "MidiLearnManager.h"
#include "MidiLearnSlider.h"
#include "SliderFactory.h"
#include <vector>
#include <map>

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

    // REFACTORED: Všechny slidery uloženy ve vektoru bundlů
    std::vector<SliderBundle> sliders_;

    // ADDED: Mapa pro rychlé vyhledávání slideru podle parameterID
    std::map<juce::String, MidiLearnSlider*> sliderMap_;

    bool debugMode_ = false;
    std::vector<int> separatorPositions_;

    // MIDI Learn state
    juce::String currentLearningParameterID_;

    // Setup methods
    void setupAllControls();
    void createAllSliders();

    // Layout methods
    void layoutBackgroundMode(juce::Rectangle<int> bounds);
    void layoutDebugMode(juce::Rectangle<int> bounds);
    void layoutSliderRow(juce::Rectangle<int>& bounds,
                        juce::Label* leftLabel, juce::Slider* leftSlider,
                        juce::Label* rightLabel, juce::Slider* rightSlider);
    void drawSeparator(juce::Rectangle<int>& bounds);
    void paintSeparators(juce::Graphics& g);

    // MIDI Learn methods
    void showMidiLearnMenu(MidiLearnSlider* slider, juce::Point<int> position);
    void updateSliderLearningVisuals(MidiLearnSlider* slider, bool isLearning);
    MidiLearnSlider* findSliderByParameterID(const juce::String& parameterID);

    // Helper methods pro získání slideru podle indexu
    SliderBundle* getSlider(size_t index);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SliderPanelComponent)
};