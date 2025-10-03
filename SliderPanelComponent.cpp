/**
 * @file SliderPanelComponent.cpp (REFACTORED with SliderFactory)
 * @brief Kompletní implementace s MIDI Learn - použitím DRY principu
 */

#include "SliderPanelComponent.h"
#include "MidiLearnManager.h"
#include "GuiConstants.h"
#include "ParameterDefaults.h"
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
    GUI_DEBUG("SliderPanelComponent: Constructor with SliderFactory (REFACTORED)");

    setupAllControls();
    setupSliderAttachments();

    GUI_DEBUG("SliderPanelComponent: Constructor completed");
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

    // REFACTORED: Update všech sliderů pomocí mapy (místo 8 manuálních volání)
    for (auto& container : sliders_) {
        if (container.slider) {
            bool isThisSlider = (isLearning && container.config.parameterID == parameterID);
            updateSliderLearningVisuals(container.slider.get(), isThisSlider);
        }
    }
}

// ============================================================================
// Private Methods - Setup
// ============================================================================

void SliderPanelComponent::setupAllControls()
{
    GUI_DEBUG("SliderPanelComponent: Setting up all controls with SliderFactory");

    removeAllChildren();

    // REFACTORED: Vytvoření všech sliderů pomocí factory (1 volání místo 4 funkcí)
    SliderFactory factory;

    auto configs = SliderFactory::getIthacaSliderConfigs();

    auto rightClickCallback = [this](juce::Slider* slider, juce::Point<int> pos) {
        GUI_DEBUG("Slider right-clicked!");
        showMidiLearnMenu(slider, pos);
    };

    sliders_ = factory.createAllSliders(configs, debugMode_, this, rightClickCallback);

    // Build lookup mapy (automaticky generované)
    SliderFactory::buildMappings(sliders_, sliderToID_, sliderToDisplayName_, idToSlider_);

    // Přidat všechny komponenty do view
    for (auto& container : sliders_) {
        if (container.label) {
            addAndMakeVisible(container.label.get());
        }
        if (container.slider) {
            addAndMakeVisible(container.slider.get());
        }
    }

    GUI_DEBUG("SliderPanelComponent: All controls created (" << sliders_.size() << " sliders)");
}

void SliderPanelComponent::setupSliderAttachments()
{
    GUI_DEBUG("SliderPanelComponent: Setting up slider attachments");

    // REFACTORED: Zaplnit SliderSet pomocí mapy (místo manuálního přiřazení)
    ParameterAttachmentManager::SliderSet sliderSet;
    sliderSet.masterGain = idToSlider_[PARAM_ID_MASTER_GAIN];
    sliderSet.masterPan = idToSlider_[PARAM_ID_MASTER_PAN];
    sliderSet.attack = idToSlider_[PARAM_ID_ATTACK];
    sliderSet.release = idToSlider_[PARAM_ID_RELEASE];
    sliderSet.sustainLevel = idToSlider_[PARAM_ID_SUSTAIN_LEVEL];
    sliderSet.lfoPanSpeed = idToSlider_[PARAM_ID_LFO_PAN_SPEED];
    sliderSet.lfoPanDepth = idToSlider_[PARAM_ID_LFO_PAN_DEPTH];
    sliderSet.stereoField = idToSlider_[PARAM_ID_STEREO_FIELD];
    sliderSet.bbeDefinition = idToSlider_[PARAM_ID_BBE_DEFINITION];
    sliderSet.bbeBassBoost = idToSlider_[PARAM_ID_BBE_BASS_BOOST];

    bool success = attachmentManager_.createAllAttachments(parameters_, sliderSet);

    if (success) {
        GUI_DEBUG("SliderPanelComponent: All attachments created successfully");
    }
}

// ============================================================================
// Private Methods - Layout
// ============================================================================

void SliderPanelComponent::layoutBackgroundMode(juce::Rectangle<int> bounds)
{
    // REFACTORED: Použít helpery pro přístup k sliderům podle indexu
    auto* s0 = getSliderByIndex(0);  // masterGain
    auto* s1 = getSliderByIndex(1);  // stereoField
    auto* s2 = getSliderByIndex(2);  // lfoPanDepth
    auto* s3 = getSliderByIndex(3);  // lfoPanSpeed
    auto* s4 = getSliderByIndex(4);  // attack
    auto* s5 = getSliderByIndex(5);  // release
    auto* s6 = getSliderByIndex(6);  // sustainLevel
    auto* s7 = getSliderByIndex(7);  // masterPan
    auto* s8 = getSliderByIndex(8);  // bbeDefinition
    auto* s9 = getSliderByIndex(9);  // bbeBassBoost

    layoutSliderRow(bounds,
                    s0 ? s0->label.get() : nullptr, s0 ? s0->slider.get() : nullptr,
                    s1 ? s1->label.get() : nullptr, s1 ? s1->slider.get() : nullptr);
    drawSeparator(bounds);

    layoutSliderRow(bounds,
                    s2 ? s2->label.get() : nullptr, s2 ? s2->slider.get() : nullptr,
                    s3 ? s3->label.get() : nullptr, s3 ? s3->slider.get() : nullptr);
    drawSeparator(bounds);

    layoutSliderRow(bounds,
                    s4 ? s4->label.get() : nullptr, s4 ? s4->slider.get() : nullptr,
                    s5 ? s5->label.get() : nullptr, s5 ? s5->slider.get() : nullptr);
    drawSeparator(bounds);

    layoutSliderRow(bounds,
                    s6 ? s6->label.get() : nullptr, s6 ? s6->slider.get() : nullptr,
                    s7 ? s7->label.get() : nullptr, s7 ? s7->slider.get() : nullptr);
    drawSeparator(bounds);

    layoutSliderRow(bounds,
                    s8 ? s8->label.get() : nullptr, s8 ? s8->slider.get() : nullptr,
                    s9 ? s9->label.get() : nullptr, s9 ? s9->slider.get() : nullptr);
}

void SliderPanelComponent::layoutDebugMode(juce::Rectangle<int> bounds)
{
    const int spacing = 4;

    // REFACTORED: Iterovat přes všechny slidery (místo 8 manuálních volání)
    for (auto& container : sliders_) {
        GuiHelpers::positionHorizontalSliderWithLabel(bounds,
            container.label.get(), container.slider.get());
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
// MIDI Learn Methods (REFACTORED)
// ============================================================================

void SliderPanelComponent::showMidiLearnMenu(juce::Slider* slider, juce::Point<int> position)
{
    juce::ignoreUnused(position);

    juce::PopupMenu menu;

    // REFACTORED: Použít mapy (místo manuálních if-else podmínek)
    auto it = sliderToID_.find(slider);
    if (it == sliderToID_.end()) return;

    juce::String parameterID = it->second;
    juce::String displayName = sliderToDisplayName_[slider];

    int assignedCC = midiLearnManager_->getCCNumberForParameter(parameterID);

    if (assignedCC >= 0) {
        menu.addItem(1, "Learn MIDI CC (currently: CC " + juce::String(assignedCC) + ")");
        menu.addItem(2, "Clear MIDI CC");
    } else {
        menu.addItem(1, "Learn MIDI CC...");
    }

    menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(this),
        [this, parameterID, displayName](int result) {
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

void SliderPanelComponent::updateSliderLearningVisuals(juce::Slider* slider, bool isLearning)
{
    if (!slider) return;

    if (isLearning) {
        slider->setColour(juce::Slider::thumbColourId, juce::Colours::red);
        slider->setColour(juce::Slider::trackColourId, juce::Colours::orange.darker());
    } else {
        GuiHelpers::styleSlider(*slider, debugMode_);
    }

    // REMOVED: slider->repaint(); - JUCE automatically repaints when colors change via setColour()
}

// ============================================================================
// Helper Methods
// ============================================================================

SliderContainer* SliderPanelComponent::getSliderByIndex(size_t index)
{
    if (index < sliders_.size()) {
        return &sliders_[index];
    }
    return nullptr;
}
