/**
 * @file GuiHelpers.cpp
 * @brief Implementace GUI helper funkcí
 * 
 * ============================================================================
 * GUI REFACTORING - HIERARCHICAL LAYOUT
 * ============================================================================
 * 
 * Nové implementace:
 * - createTitleLabel() - Velký font (18px bold)
 * - drawRoundedOverlay() - Zaoblené panely s alpha
 * - drawSeparatorLine() - Průhledné separátory
 * 
 * Aktualizováno:
 * - Font sizes podle hierarchie (18px, 14px, 11px)
 * - Label styling pro overlay mode
 * ============================================================================
 */

#include "ithaca/gui/helpers/GuiHelpers.h"
#include <iostream>

#define BACKGROUND_PICTURE_OFF 0

#if BACKGROUND_PICTURE_OFF
#define GUI_DEBUG(msg) std::cout << msg << std::endl
#else
#define GUI_DEBUG(msg)
#endif

// =========================================================================
// SLIDER CREATION
// =========================================================================

std::unique_ptr<juce::Slider> GuiHelpers::createCompactSlider(
    double min, double max, double defaultVal, double interval)
{
    auto slider = std::make_unique<juce::Slider>(
        juce::Slider::LinearHorizontal, 
        juce::Slider::NoTextBox);
    
    slider->setRange(min, max, interval);
    slider->setValue(defaultVal);
    
    styleSlider(*slider, isDebugModeEnabled());
    
    GUI_DEBUG("GuiHelpers: Created horizontal slider (" << min << "-" << max 
              << ", default=" << defaultVal << ")");
    
    return slider;
}

void GuiHelpers::styleSlider(juce::Slider& slider, bool debugMode)
{
    if (debugMode) {
        slider.setColour(juce::Slider::trackColourId,
                        juce::Colour(Constants::Gui::Colors::DEBUG_SLIDER_TRACK));
        slider.setColour(juce::Slider::thumbColourId,
                        juce::Colour(Constants::Gui::Colors::DEBUG_SLIDER_THUMB));
        slider.setColour(juce::Slider::textBoxTextColourId,
                        juce::Colour(Constants::Gui::Colors::DEBUG_TEXT));
        slider.setColour(juce::Slider::textBoxBackgroundColourId,
                        juce::Colour(0xffffffff));
    } else {
        slider.setColour(juce::Slider::trackColourId,
                        juce::Colour(Constants::Gui::Colors::SLIDER_TRACK));
        slider.setColour(juce::Slider::thumbColourId,
                        juce::Colour(Constants::Gui::Colors::SLIDER_THUMB));
        slider.setColour(juce::Slider::textBoxTextColourId,
                        juce::Colour(Constants::Gui::Colors::SLIDER_TEXT));
        slider.setColour(juce::Slider::textBoxBackgroundColourId,
                        juce::Colours::black.withAlpha(0.7f));
        slider.setColour(juce::Slider::textBoxOutlineColourId,
                        juce::Colours::white.withAlpha(0.3f));
    }
}

// =========================================================================
// LABEL CREATION - Hierarchie fontů
// =========================================================================

std::unique_ptr<juce::Label> GuiHelpers::createTitleLabel(
    const juce::String& text, bool debugMode)
{
    auto label = std::make_unique<juce::Label>();
    label->setText(text, juce::dontSendNotification);
    label->setJustificationType(juce::Justification::centredLeft);
    
    // Velký font pro title (18px, bold)
    label->setFont(juce::FontOptions(Constants::Gui::Fonts::TITLE_SIZE,
                                     juce::Font::bold));

    if (debugMode) {
        label->setColour(juce::Label::textColourId,
                        juce::Colour(Constants::Gui::Colors::DEBUG_TEXT));
        label->setColour(juce::Label::backgroundColourId, 
                        juce::Colours::transparentWhite);
    } else {
        label->setColour(juce::Label::textColourId, juce::Colours::white);
        label->setColour(juce::Label::backgroundColourId, 
                        juce::Colours::transparentBlack);
    }
    
    return label;
}

std::unique_ptr<juce::Label> GuiHelpers::createSliderLabel(
    const juce::String& text, bool debugMode)
{
    auto label = std::make_unique<juce::Label>();
    label->setText(text, juce::dontSendNotification);
    label->setJustificationType(juce::Justification::centredLeft);
    
    // Střední font pro slider labels (14px, bold)
    label->setFont(juce::FontOptions(Constants::Gui::Fonts::INFO_SIZE,
                                     juce::Font::bold));

    if (debugMode) {
        label->setColour(juce::Label::textColourId,
                        juce::Colour(Constants::Gui::Colors::DEBUG_TEXT));
        label->setColour(juce::Label::backgroundColourId, 
                        juce::Colours::transparentWhite);
    } else {
        label->setColour(juce::Label::textColourId, juce::Colours::white);
        label->setColour(juce::Label::backgroundColourId, 
                        juce::Colours::transparentBlack);
    }
    
    return label;
}

std::unique_ptr<juce::Label> GuiHelpers::createInfoLabel(
    const juce::String& text, bool debugMode)
{
    auto label = std::make_unique<juce::Label>();
    label->setText(text, juce::dontSendNotification);
    label->setJustificationType(juce::Justification::centredLeft);
    
    // Střední font pro info labels (14px)
    label->setFont(juce::FontOptions(Constants::Gui::Fonts::INFO_SIZE));

    if (debugMode) {
        label->setColour(juce::Label::textColourId,
                        juce::Colour(Constants::Gui::Colors::DEBUG_TEXT));
        label->setColour(juce::Label::backgroundColourId, 
                        juce::Colour(0xffffffff));
        label->setColour(juce::Label::outlineColourId, 
                        juce::Colour(0xff333333));
    } else {
        label->setColour(juce::Label::textColourId, juce::Colours::white);
        label->setColour(juce::Label::backgroundColourId, 
                        juce::Colours::transparentBlack);
    }
    
    return label;
}

std::unique_ptr<juce::Label> GuiHelpers::createSmallLabel(
    const juce::String& text, bool debugMode)
{
    auto label = std::make_unique<juce::Label>();
    label->setText(text, juce::dontSendNotification);
    label->setJustificationType(juce::Justification::centredLeft);
    
    // Malý font pro detail info (11px)
    label->setFont(juce::FontOptions(Constants::Gui::Fonts::SMALL_SIZE));

    if (debugMode) {
        label->setColour(juce::Label::textColourId,
                        juce::Colour(Constants::Gui::Colors::DEBUG_TEXT));
        label->setColour(juce::Label::backgroundColourId, 
                        juce::Colour(0xffffffff));
        label->setColour(juce::Label::outlineColourId, 
                        juce::Colour(0xff333333));
    } else {
        label->setColour(juce::Label::textColourId, juce::Colours::white);
        label->setColour(juce::Label::backgroundColourId, 
                        juce::Colours::transparentBlack);
    }
    
    return label;
}

// =========================================================================
// LAYOUT HELPERS
// =========================================================================

void GuiHelpers::positionHorizontalSliderWithLabel(
    juce::Rectangle<int>& area, 
    juce::Label* label, 
    juce::Slider* slider)
{
    if (label) {
        label->setBounds(area.removeFromTop(Constants::Gui::Slider::LABEL_HEIGHT));
        area.removeFromTop(Constants::Gui::Slider::LABEL_SPACING);
    }

    if (slider) {
        slider->setBounds(area.removeFromTop(Constants::Gui::Slider::HEIGHT_HORIZONTAL));
    }
}

juce::Rectangle<int> GuiHelpers::layoutTwoColumnSliders(
    juce::Rectangle<int> totalArea, 
    juce::Rectangle<int>& leftColumn, 
    juce::Rectangle<int>& rightColumn)
{
    int halfWidth = totalArea.getWidth() / 2;
    int spacing = Constants::Gui::Layout::COLUMN_SPACING;

    leftColumn = totalArea.removeFromLeft(halfWidth - spacing / 2);
    totalArea.removeFromLeft(spacing);
    rightColumn = totalArea;
    
    return totalArea;
}

// =========================================================================
// OVERLAY RENDERING - NOVÉ funkce
// =========================================================================

void GuiHelpers::drawRoundedOverlay(juce::Graphics& g, 
                                    juce::Rectangle<int> area, 
                                    float alpha, 
                                    float cornerRadius)
{
    // Zaoblené černé pozadí s alpha průhledností
    g.setColour(juce::Colours::black.withAlpha(alpha));
    g.fillRoundedRectangle(area.toFloat(), cornerRadius);
    
    // Jemný bílý border
    g.setColour(juce::Colours::white.withAlpha(alpha * 0.25f));
    g.drawRoundedRectangle(area.toFloat(), cornerRadius, 1.0f);
    
    GUI_DEBUG("GuiHelpers: Drew rounded overlay at " << area.toString().toStdString() 
              << " with alpha=" << alpha << " radius=" << cornerRadius);
}

void GuiHelpers::drawSeparatorLine(juce::Graphics& g, 
                                   int x1, int y1, int x2, int y2,
                                   juce::Colour baseColor)
{
    // Průhledná čára v barvě textu
    g.setColour(baseColor.withAlpha(Constants::Gui::Overlay::SEPARATOR_ALPHA));
    g.drawLine(static_cast<float>(x1), static_cast<float>(y1),
               static_cast<float>(x2), static_cast<float>(y2),
               static_cast<float>(Constants::Gui::Overlay::SEPARATOR_THICKNESS));
    
    GUI_DEBUG("GuiHelpers: Drew separator line from (" << x1 << "," << y1 
              << ") to (" << x2 << "," << y2 << ")");
}

// =========================================================================
// LEGACY FUNCTIONS
// =========================================================================

void GuiHelpers::applyControlAreaOverlay(juce::Graphics& g, 
                                        juce::Rectangle<int> area)
{
    // Legacy - nyní používá drawRoundedOverlay
    drawRoundedOverlay(g, area, Constants::Gui::Overlay::SLIDER_ALPHA,
                      Constants::Gui::Overlay::CORNER_RADIUS);
}

void GuiHelpers::applyDebugBackground(juce::Graphics& g, 
                                      juce::Rectangle<int> area)
{
    // ZACHOVÁNO: Přesně původní debug background
    g.fillAll(juce::Colour(Constants::Gui::Colors::DEBUG_BG));

    g.setColour(juce::Colour(0xff333333));
    g.setFont(juce::FontOptions(Constants::Gui::Fonts::TITLE_SIZE));
    g.drawText("IthacaCore Sampler - Controls", 10, 10, 
               area.getWidth() - 20, 30, 
               juce::Justification::centred);
    
    g.setColour(juce::Colour(0xffcccccc));
    g.drawLine(10.0f, 45.0f, static_cast<float>(area.getWidth() - 10), 45.0f, 1.0f);
}

// =========================================================================
// UTILITY FUNCTIONS
// =========================================================================

bool GuiHelpers::isDebugModeEnabled()
{
    return BACKGROUND_PICTURE_OFF != 0;
}

void GuiHelpers::updateLabelText(juce::Label* label, const juce::String& newText)
{
    if (label && label->getText() != newText) {
        label->setText(newText, juce::dontSendNotification);
    }
}