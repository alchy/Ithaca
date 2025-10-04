/**
 * @file SliderPanelComponent.cpp (REFACTORED with SliderFactory)
 * @brief Refaktorovaná implementace využívající SliderFactory
 */

#include "ithaca/gui/components/SliderPanelComponent.h"
#include "ithaca/midi/MidiLearnManager.h"
#include "ithaca/gui/helpers/GuiConstants.h"
#include <iostream>

#define BACKGROUND_PICTURE_OFF 0

#if BACKGROUND_PICTURE_OFF
#define GUI_DEBUG(msg) std::cout << msg << std::endl
#else
#define GUI_DEBUG(msg)
#endif

// ============================================================================
// Constructor / Destructor
// ============================================================================

SliderPanelComponent::SliderPanelComponent(
    juce::AudioProcessorValueTreeState& parameters,
    MidiLearnManager* midiLearnManager)
    : parameters_(parameters),
      midiLearnManager_(midiLearnManager),
      debugMode_(GuiHelpers::isDebugModeEnabled())
{
    GUI_DEBUG("SliderPanelComponent: Constructor with SliderFactory");

    setupAllControls();

    GUI_DEBUG("SliderPanelComponent: Constructor completed - "
              << sliders_.size() << " sliders created");
}

SliderPanelComponent::~SliderPanelComponent()
{
    GUI_DEBUG("SliderPanelComponent: Destructor");
}

// ============================================================================
// Component Overrides
// ============================================================================

void SliderPanelComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    
    if (!debugMode_) {
        GuiHelpers::drawRoundedOverlay(g, bounds, 
                                      GuiConstants::SLIDER_OVERLAY_ALPHA,
                                      GuiConstants::PANEL_CORNER_RADIUS);
        paintSeparators(g);
    }
}

void SliderPanelComponent::resized()
{
    separatorPositions_.clear();
    
    auto bounds = getLocalBounds().reduced(GuiConstants::SECTION_PADDING);
    
    if (debugMode_) {
        layoutDebugMode(bounds);
    } else {
        layoutBackgroundMode(bounds);
    }
}

// NOTE: mouseDown() REMOVED - handled by MidiLearnSlider

// ============================================================================
// Slider Listener
// ============================================================================

void SliderPanelComponent::sliderValueChanged(juce::Slider* slider)
{
    juce::ignoreUnused(slider);
}

// ============================================================================
// Public Control
// ============================================================================

void SliderPanelComponent::setDebugMode(bool enabled)
{
    if (debugMode_ != enabled) {
        debugMode_ = enabled;
        setupAllControls();
        resized();
        GUI_DEBUG("SliderPanelComponent: Debug mode " << (enabled ? "ENABLED" : "DISABLED"));
    }
}

void SliderPanelComponent::onLearningStateChanged(bool isLearning, const juce::String& parameterID)
{
    currentLearningParameterID_ = isLearning ? parameterID : juce::String();

    // REFACTORED: Iterace přes všechny slidery - DRY
    for (auto& bundle : sliders_) {
        bool isThisSliderLearning = (isLearning && bundle.config.parameterID == parameterID);
        updateSliderLearningVisuals(bundle.slider.get(), isThisSliderLearning);
    }
}

// ============================================================================
// Private Methods - Setup
// ============================================================================

void SliderPanelComponent::setupAllControls()
{
    GUI_DEBUG("SliderPanelComponent: Setting up all controls with SliderFactory");

    removeAllChildren();
    sliders_.clear();
    sliderMap_.clear();

    createAllSliders();

    GUI_DEBUG("SliderPanelComponent: All controls created via factory");
}

void SliderPanelComponent::createAllSliders()
{
    // REFACTORED: Všechny slidery vytvoří SliderFactory podle konfigurace
    // Callback pro right-click (MIDI Learn menu)
    auto rightClickCallback = [this](MidiLearnSlider* slider, juce::Point<int> pos) {
        showMidiLearnMenu(slider, pos);
    };

    // Definice všech sliderů v pořadí podle layoutu
    std::vector<SliderConfig> configs = {
        // Row 1: Master Gain, Stereo Field
        SliderConfig("masterGain", "Master Gain", GuiConstants::TextConstants::MASTER_GAIN_LABEL, 100.0),
        SliderConfig("stereoField", "Stereo Field", GuiConstants::TextConstants::STEREO_FIELD_LABEL, 0.0),

        // Row 2: LFO Depth, LFO Speed
        SliderConfig("lfoPanDepth", "LFO Depth", GuiConstants::TextConstants::LFO_DEPTH_LABEL, 0.0),
        SliderConfig("lfoPanSpeed", "LFO Speed", GuiConstants::TextConstants::LFO_SPEED_LABEL, 0.0),

        // Row 3: Attack, Release
        SliderConfig("attack", "Attack", GuiConstants::TextConstants::ATTACK_LABEL, 0.0),
        SliderConfig("release", "Release", GuiConstants::TextConstants::RELEASE_LABEL, 4.0),

        // Row 4: Sustain Level, Master Pan
        SliderConfig("sustainLevel", "Sustain Level", GuiConstants::TextConstants::SUSTAIN_LABEL, 127.0),
        SliderConfig("masterPan", "Master Pan", GuiConstants::TextConstants::MASTER_PAN_LABEL, 64.0)
    };

    // Vytvoření všech sliderů pomocí factory
    sliders_.reserve(configs.size());
    for (const auto& config : configs) {
        auto bundle = SliderFactory::createSlider(config, parameters_, debugMode_, rightClickCallback);

        // Přidat listener pro value changes
        if (bundle.slider) {
            bundle.slider->addListener(this);
        }

        // Přidat do komponenty
        SliderFactory::addToComponent(*this, bundle);

        // Uložit bundle
        sliders_.push_back(std::move(bundle));
    }

    // Vytvořit mapu pro rychlý lookup
    sliderMap_ = SliderFactory::createParameterMap(sliders_);

    GUI_DEBUG("SliderPanelComponent: Created " << sliders_.size() << " sliders via factory");
}

// ============================================================================
// Private Methods - Layout
// ============================================================================

void SliderPanelComponent::layoutBackgroundMode(juce::Rectangle<int> bounds)
{
    // REFACTORED: Layout sliderů ve dvojicích (2 slidery per řádek)
    // Indexy: 0-1, 2-3, 4-5, 6-7
    for (size_t i = 0; i < sliders_.size(); i += 2) {
        if (i + 1 < sliders_.size()) {
            layoutSliderRow(bounds,
                          sliders_[i].label.get(), sliders_[i].slider.get(),
                          sliders_[i + 1].label.get(), sliders_[i + 1].slider.get());
            if (i + 2 < sliders_.size()) { // Separátor pouze mezi řádky
                drawSeparator(bounds);
            }
        }
    }
}

void SliderPanelComponent::layoutDebugMode(juce::Rectangle<int> bounds)
{
    const int spacing = 4;

    // REFACTORED: Iterace přes všechny slidery - single column layout
    for (auto& bundle : sliders_) {
        GuiHelpers::positionHorizontalSliderWithLabel(bounds,
                                                     bundle.label.get(),
                                                     bundle.slider.get());
        bounds.removeFromTop(spacing);
    }
}

void SliderPanelComponent::layoutSliderRow(juce::Rectangle<int>& bounds,
                                           juce::Label* leftLabel, juce::Slider* leftSlider,
                                           juce::Label* rightLabel, juce::Slider* rightSlider)
{
    auto rowArea = bounds.removeFromTop(GuiConstants::SLIDER_ROW_HEIGHT);
    
    int halfWidth = rowArea.getWidth() / 2;
    int columnSpacing = GuiConstants::COLUMN_SPACING;
    
    auto leftColumn = rowArea.removeFromLeft(halfWidth - columnSpacing / 2);
    rowArea.removeFromLeft(columnSpacing);
    auto rightColumn = rowArea;
    
    GuiHelpers::positionHorizontalSliderWithLabel(leftColumn, leftLabel, leftSlider);
    GuiHelpers::positionHorizontalSliderWithLabel(rightColumn, rightLabel, rightSlider);
    
    bounds.removeFromTop(GuiConstants::SECTION_SPACING);
}

void SliderPanelComponent::drawSeparator(juce::Rectangle<int>& bounds)
{
    separatorPositions_.push_back(bounds.getY());
    bounds.removeFromTop(2);
}

void SliderPanelComponent::paintSeparators(juce::Graphics& g)
{
    if (debugMode_) return;
    
    auto bounds = getLocalBounds();
    int leftMargin = GuiConstants::SECTION_PADDING + 4;
    int rightMargin = bounds.getWidth() - GuiConstants::SECTION_PADDING - 4;
    
    for (int y : separatorPositions_) {
        GuiHelpers::drawSeparatorLine(g, leftMargin, y, rightMargin, y);
    }
}

// ============================================================================
// MIDI Learn Methods
// ============================================================================

void SliderPanelComponent::showMidiLearnMenu(MidiLearnSlider* slider, juce::Point<int> position)
{
    juce::ignoreUnused(position);

    if (!slider || !midiLearnManager_) return;

    juce::PopupMenu menu;

    // REFACTORED: Najít config podle pointeru na slider
    const SliderConfig* config = nullptr;
    for (const auto& bundle : sliders_) {
        if (bundle.slider.get() == slider) {
            config = &bundle.config;
            break;
        }
    }

    if (!config) return;

    int assignedCC = midiLearnManager_->getCCNumberForParameter(config->parameterID);

    if (assignedCC >= 0) {
        menu.addItem(1, "Learn MIDI CC (currently: CC " + juce::String(assignedCC) + ")");
        menu.addItem(2, "Clear MIDI CC");
    } else {
        menu.addItem(1, "Learn MIDI CC...");
    }

    menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(this),
        [this, parameterID = config->parameterID, displayName = config->displayName](int result) {
            if (result == 1) {
                midiLearnManager_->startLearning(parameterID, displayName);

                juce::AlertWindow::showMessageBoxAsync(
                    juce::AlertWindow::InfoIcon,
                    "MIDI Learn Active",
                    "Move a control on your MIDI controller to assign it to " + displayName,
                    "OK");
            }
            else if (result == 2) {
                midiLearnManager_->removeMappingForParameter(parameterID);
            }
        });
}

void SliderPanelComponent::updateSliderLearningVisuals(MidiLearnSlider* slider, bool isLearning)
{
    // REFACTORED: Využívá SliderFactory pro nastavení visuálů
    SliderFactory::setLearningVisuals(slider, isLearning, debugMode_);
}

MidiLearnSlider* SliderPanelComponent::findSliderByParameterID(const juce::String& parameterID)
{
    // REFACTORED: Využívá mapu pro O(log n) lookup
    auto it = sliderMap_.find(parameterID);
    return (it != sliderMap_.end()) ? it->second : nullptr;
}

// ============================================================================
// Helper Methods
// ============================================================================

SliderBundle* SliderPanelComponent::getSlider(size_t index)
{
    return (index < sliders_.size()) ? &sliders_[index] : nullptr;
}