/**
 * @file SliderFactory.cpp
 * @brief Implementace factory pro vytváření sliderů
 */

#include "SliderFactory.h"
#include <iostream>

#define BACKGROUND_PICTURE_OFF 0

#if BACKGROUND_PICTURE_OFF
#define GUI_DEBUG(msg) std::cout << msg << std::endl
#else
#define GUI_DEBUG(msg)
#endif

SliderBundle SliderFactory::createSlider(
    const SliderConfig& config,
    juce::AudioProcessorValueTreeState& parameters,
    bool debugMode,
    RightClickCallback rightClickCallback)
{
    SliderBundle bundle{};
    bundle.config = SliderConfig(config); // Explicit copy

    // 1. Vytvořit label
    bundle.label = GuiHelpers::createSliderLabel(config.labelText, debugMode);
    if (!bundle.label) {
        GUI_DEBUG("SliderFactory: ERROR - Failed to create label for " << config.parameterID.toStdString());
        return bundle;
    }

    // 2. Vytvořit MidiLearnSlider
    bundle.slider = std::make_unique<MidiLearnSlider>(
        juce::Slider::LinearHorizontal,
        juce::Slider::NoTextBox);

    if (!bundle.slider) {
        GUI_DEBUG("SliderFactory: ERROR - Failed to create slider for " << config.parameterID.toStdString());
        return bundle;
    }

    // 3. Nastavit range a default hodnotu
    bundle.slider->setRange(config.minValue, config.maxValue, config.interval);
    bundle.slider->setValue(config.defaultValue);

    // 4. Aplikovat styling
    GuiHelpers::styleSlider(*bundle.slider, debugMode);

    // 5. Nastavit right-click callback (MIDI Learn)
    if (rightClickCallback) {
        bundle.slider->setRightClickCallback(
            [rightClickCallback, slider = bundle.slider.get()](juce::Point<int> pos) {
                rightClickCallback(slider, pos);
            });
    }

    // 6. Vytvořit attachment na parametr
    auto* param = parameters.getParameter(config.parameterID);
    if (param) {
        bundle.attachment = std::make_unique<juce::SliderParameterAttachment>(
            *param,
            *bundle.slider,
            nullptr); // Undomanager (nullptr = no undo)

        GUI_DEBUG("SliderFactory: Created slider '" << config.displayName.toStdString()
                  << "' (ID: " << config.parameterID.toStdString() << ")");
    } else {
        GUI_DEBUG("SliderFactory: WARNING - Parameter '" << config.parameterID.toStdString()
                  << "' not found in APVTS");
    }

    return bundle;
}

void SliderFactory::addToComponent(juce::Component& parent, SliderBundle& bundle)
{
    if (bundle.label) {
        parent.addAndMakeVisible(bundle.label.get());
    }

    if (bundle.slider) {
        parent.addAndMakeVisible(bundle.slider.get());
    }
}

void SliderFactory::setLearningVisuals(MidiLearnSlider* slider, bool isLearning, bool debugMode)
{
    if (!slider) return;

    if (isLearning) {
        // MIDI Learn aktivní - červený thumb
        slider->setColour(juce::Slider::thumbColourId, juce::Colours::red);
        slider->setColour(juce::Slider::trackColourId, juce::Colours::orange.darker());
    } else {
        // Normální styling
        GuiHelpers::styleSlider(*slider, debugMode);
    }

    slider->repaint();
}

std::map<juce::String, MidiLearnSlider*> SliderFactory::createParameterMap(
    std::vector<SliderBundle>& bundles)
{
    std::map<juce::String, MidiLearnSlider*> paramMap;

    for (auto& bundle : bundles) {
        if (bundle.slider) {
            paramMap[bundle.config.parameterID] = bundle.slider.get();
            GUI_DEBUG("SliderFactory: Mapped parameter '"
                      << bundle.config.parameterID.toStdString() << "'");
        }
    }

    GUI_DEBUG("SliderFactory: Created parameter map with " << paramMap.size() << " entries");
    return paramMap;
}
