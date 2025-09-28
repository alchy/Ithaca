/**
 * @file GuiHelpers.cpp
 * @brief Implementace GUI helper funkcí
 */

#include "GuiHelpers.h"
#include <iostream>

// DEBUG: Přepínač pro vypnutí background obrázku (zachovat původní logiku)
#define BACKGROUND_PICTURE_OFF 0

// Makro pro debug výpisy (zachovat původní logiku)
#if BACKGROUND_PICTURE_OFF
#define GUI_DEBUG(msg) std::cout << msg << std::endl
#else
#define GUI_DEBUG(msg)
#endif

std::unique_ptr<juce::Slider> GuiHelpers::createCompactSlider(
    double min, double max, double defaultVal, double interval)
{
    // ZMĚNA: LinearHorizontal místo LinearVertical
    auto slider = std::make_unique<juce::Slider>(
        juce::Slider::LinearHorizontal, 
        juce::Slider::NoTextBox);  // ZMĚNA: NoTextBox místo TextBoxBelow
    
    slider->setRange(min, max, interval);
    slider->setValue(defaultVal);
    
    // ZACHOVAT: Původní styling logiku
    styleSlider(*slider, isDebugModeEnabled());
    
    GUI_DEBUG("GuiHelpers: Created horizontal slider (" << min << "-" << max << ", default=" << defaultVal << ")");
    
    return slider;
}

void GuiHelpers::styleSlider(juce::Slider& slider, bool debugMode)
{
    // ZACHOVAT: Přesně původní styling logiku
    if (debugMode) {
        // Debug mode: jasné barvy na šedém pozadí
        slider.setColour(juce::Slider::trackColourId, juce::Colour(GuiConstants::DEBUG_SLIDER_TRACK));
        slider.setColour(juce::Slider::thumbColourId, juce::Colour(GuiConstants::DEBUG_SLIDER_THUMB));
        slider.setColour(juce::Slider::textBoxTextColourId, juce::Colour(GuiConstants::DEBUG_TEXT_COLOR));
        slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xffffffff));
    } else {
        // Background mode: kontrastní barvy pro overlay
        slider.setColour(juce::Slider::trackColourId, juce::Colour(GuiConstants::SLIDER_TRACK_COLOR));
        slider.setColour(juce::Slider::thumbColourId, juce::Colour(GuiConstants::SLIDER_THUMB_COLOR));
        slider.setColour(juce::Slider::textBoxTextColourId, juce::Colour(GuiConstants::SLIDER_TEXT_COLOR));
        slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::black.withAlpha(0.7f));
        slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::white.withAlpha(0.3f));
    }
}

std::unique_ptr<juce::Label> GuiHelpers::createSliderLabel(const juce::String& text, bool debugMode)
{
    auto label = std::make_unique<juce::Label>();
    label->setText(text, juce::dontSendNotification);
    label->setJustificationType(juce::Justification::centred);
    label->setFont(juce::FontOptions(GuiConstants::LABEL_FONT_SIZE, juce::Font::bold));
    
    // ZACHOVAT: Původní color scheme
    if (debugMode) {
        label->setColour(juce::Label::textColourId, juce::Colour(GuiConstants::DEBUG_TEXT_COLOR));
        label->setColour(juce::Label::backgroundColourId, juce::Colours::transparentWhite);
    } else {
        label->setColour(juce::Label::textColourId, juce::Colours::white);
        label->setColour(juce::Label::backgroundColourId, juce::Colours::black.withAlpha(0.6f));
        label->setColour(juce::Label::outlineColourId, juce::Colours::white.withAlpha(0.3f));
    }
    
    return label;
}

std::unique_ptr<juce::Label> GuiHelpers::createSmallLabel(const juce::String& text, bool debugMode)
{
    auto label = std::make_unique<juce::Label>();
    label->setText(text, juce::dontSendNotification);
    label->setJustificationType(juce::Justification::centredLeft);
    
    // ZACHOVAT: Původní color scheme
    if (debugMode) {
        label->setColour(juce::Label::textColourId, juce::Colour(GuiConstants::DEBUG_TEXT_COLOR));
        label->setColour(juce::Label::backgroundColourId, juce::Colour(0xffffffff));
        label->setColour(juce::Label::outlineColourId, juce::Colour(0xff333333));
    } else {
        label->setColour(juce::Label::textColourId, juce::Colours::white);
        label->setColour(juce::Label::backgroundColourId, juce::Colours::black.withAlpha(0.75f));
        label->setColour(juce::Label::outlineColourId, juce::Colours::white.withAlpha(0.4f));
    }
    
    label->setFont(juce::FontOptions(GuiConstants::SMALL_LABEL_FONT_SIZE));
    
    return label;
}

std::unique_ptr<juce::Label> GuiHelpers::createInfoLabel(const juce::String& text, bool debugMode)
{
    auto label = std::make_unique<juce::Label>();
    label->setText(text, juce::dontSendNotification);
    label->setJustificationType(juce::Justification::centredLeft);
    
    // ZACHOVAT: Původní color scheme
    if (debugMode) {
        label->setColour(juce::Label::textColourId, juce::Colour(GuiConstants::DEBUG_TEXT_COLOR));
        label->setColour(juce::Label::backgroundColourId, juce::Colour(0xffffffff));
        label->setColour(juce::Label::outlineColourId, juce::Colour(0xff333333));
    } else {
        label->setColour(juce::Label::textColourId, juce::Colours::white);
        label->setColour(juce::Label::backgroundColourId, juce::Colours::black.withAlpha(0.85f));
        label->setColour(juce::Label::outlineColourId, juce::Colours::white.withAlpha(0.6f));
    }
    
    label->setFont(juce::FontOptions(GuiConstants::INFO_LABEL_FONT_SIZE));
    
    return label;
}

void GuiHelpers::positionHorizontalSliderWithLabel(juce::Rectangle<int>& area, 
                                                   juce::Label* label, 
                                                   juce::Slider* slider)
{
    if (label) {
        label->setBounds(area.removeFromTop(GuiConstants::LABEL_HEIGHT));
        area.removeFromTop(GuiConstants::LABEL_SPACING);
    }
    
    if (slider) {
        slider->setBounds(area.removeFromTop(GuiConstants::SLIDER_HEIGHT_HORIZONTAL));
        area.removeFromTop(GuiConstants::SLIDER_SPACING);
    }
}

juce::Rectangle<int> GuiHelpers::layoutTwoColumnSliders(
    juce::Rectangle<int> totalArea, 
    juce::Rectangle<int>& leftColumn, 
    juce::Rectangle<int>& rightColumn)
{
    // ZACHOVAT: Původní layout proporce
    leftColumn = totalArea.removeFromLeft(150);
    rightColumn = totalArea.removeFromLeft(150);
    
    return totalArea; // Zbývající prostor
}

void GuiHelpers::applyControlAreaOverlay(juce::Graphics& g, juce::Rectangle<int> area)
{
    // ZACHOVAT: Přesně původní overlay vykreslování
    g.setColour(juce::Colours::black.withAlpha(0.6f));
    g.fillRect(area);
    
    // Jemnější border
    g.setColour(juce::Colours::white.withAlpha(0.2f));
    g.drawRect(area, 1);
}

void GuiHelpers::applyDebugBackground(juce::Graphics& g, juce::Rectangle<int> area)
{
    // ZACHOVAT: Přesně původní debug background
    g.fillAll(juce::Colour(GuiConstants::DEBUG_BG_COLOR));
    
    g.setColour(juce::Colour(0xff333333));
    g.setFont(juce::FontOptions(GuiConstants::TITLE_FONT_SIZE));
    g.drawText("IthacaCore Sampler - Controls", 10, 10, area.getWidth() - 20, 30, 
               juce::Justification::centred);
    
    g.setColour(juce::Colour(0xffcccccc));
    g.drawLine(10.0f, 45.0f, static_cast<float>(area.getWidth() - 10), 45.0f, 1.0f);
}

bool GuiHelpers::isDebugModeEnabled()
{
    // ZACHOVAT: Původní debug mode logiku
    return BACKGROUND_PICTURE_OFF != 0;
}

void GuiHelpers::updateLabelText(juce::Label* label, const juce::String& newText)
{
    if (label && label->getText() != newText) {
        label->setText(newText, juce::dontSendNotification);
    }
}