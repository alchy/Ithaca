#include "GuiHelpers.h"
#include <iostream>

// Static member initialization
bool DebugHelper::debugEnabled = false;

//==============================================================================
// SLIDER CREATION

std::unique_ptr<juce::Slider> GuiHelpers::createCompactSlider(double min, double max, double defaultVal, double interval)
{
    auto slider = std::make_unique<juce::Slider>(juce::Slider::LinearHorizontal, juce::Slider::NoTextBox);
    
    slider->setRange(min, max, interval);
    slider->setValue(defaultVal);
    
    // Apply default styling
    styleSlider(*slider, false);
    
    return slider;
}

void GuiHelpers::styleSlider(juce::Slider& slider, bool debugMode)
{
    if (debugMode) {
        // Debug mode: jasné barvy na šedém pozadí
        slider.setColour(juce::Slider::trackColourId, juce::Colour(0xff333333));
        slider.setColour(juce::Slider::thumbColourId, juce::Colour(0xff0066cc));
    } else {
        // Background mode: kontrastní barvy pro overlay
        slider.setColour(juce::Slider::trackColourId, juce::Colour(SLIDER_TRACK_COLOR));
        slider.setColour(juce::Slider::thumbColourId, juce::Colour(SLIDER_THUMB_COLOR));
    }
}

//==============================================================================
// LABEL CREATION

std::unique_ptr<juce::Label> GuiHelpers::createSliderLabel(const juce::String& text)
{
    auto label = std::make_unique<juce::Label>();
    label->setText(text, juce::dontSendNotification);
    label->setJustificationType(juce::Justification::centred);
    label->setFont(juce::FontOptions(11.0f, juce::Font::bold));
    
    styleLabel(*label, true, false);
    
    return label;
}

std::unique_ptr<juce::Label> GuiHelpers::createInfoLabel(const juce::String& text)
{
    auto label = std::make_unique<juce::Label>();
    label->setText(text, juce::dontSendNotification);
    label->setJustificationType(juce::Justification::centredLeft);
    label->setFont(juce::FontOptions(10.0f));
    
    styleLabel(*label, false, false);
    
    return label;
}

void GuiHelpers::styleLabel(juce::Label& label, bool isSliderLabel, bool debugMode)
{
    if (debugMode) {
        // Debug mode: černý text na bílém pozadí
        label.setColour(juce::Label::textColourId, juce::Colour(0xff000000));
        label.setColour(juce::Label::backgroundColourId, juce::Colour(0xffffffff));
        label.setColour(juce::Label::outlineColourId, juce::Colour(0xff333333));
    } else {
        // Background mode: bílý text s tmavým pozadím
        label.setColour(juce::Label::textColourId, juce::Colours::white);
        
        if (isSliderLabel) {
            label.setColour(juce::Label::backgroundColourId, juce::Colours::black.withAlpha(0.6f));
            label.setColour(juce::Label::outlineColourId, juce::Colours::white.withAlpha(0.3f));
        } else {
            label.setColour(juce::Label::backgroundColourId, juce::Colours::black.withAlpha(0.75f));
            label.setColour(juce::Label::outlineColourId, juce::Colours::white.withAlpha(0.4f));
        }
    }
}

//==============================================================================
// LAYOUT HELPERS

void GuiHelpers::positionSliderWithLabel(juce::Rectangle<int>& area, juce::Label* label, juce::Slider* slider)
{
    if (label) {
        label->setBounds(area.removeFromTop(LABEL_HEIGHT));
        area.removeFromTop(LABEL_SPACING);
    }
    
    if (slider) {
        slider->setBounds(area.removeFromTop(SLIDER_HEIGHT));
        area.removeFromTop(SLIDER_SPACING);
    }
}

void GuiHelpers::positionInfoLabel(juce::Rectangle<int>& area, juce::Label* label, int spacing)
{
    if (label) {
        label->setBounds(area.removeFromTop(18));
        area.removeFromTop(spacing);
    }
}

//==============================================================================
// DEBUG HELPER

void DebugHelper::print(const juce::String& message)
{
    if (debugEnabled) {
        std::cout << "[IthacaGUI] " << message.toStdString() << std::endl;
    }
}