/**
 * @file SliderPanelComponent.cpp (COMPLETE with MidiLearnSlider)
 * @brief Kompletní implementace s MIDI Learn funkčností
 */

#include "SliderPanelComponent.h"
#include "MidiLearnManager.h"
#include "GuiConstants.h"
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
    GUI_DEBUG("SliderPanelComponent: Constructor with MIDI Learn");
    
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
    
    // Update visuals pro všechny slidery
    updateSliderLearningVisuals(masterGainSlider.get(), 
        isLearning && parameterID == "masterGain");
    updateSliderLearningVisuals(masterPanSlider.get(), 
        isLearning && parameterID == "masterPan");
    updateSliderLearningVisuals(attackSlider.get(), 
        isLearning && parameterID == "attack");
    updateSliderLearningVisuals(releaseSlider.get(), 
        isLearning && parameterID == "release");
    updateSliderLearningVisuals(sustainLevelSlider.get(), 
        isLearning && parameterID == "sustainLevel");
    updateSliderLearningVisuals(lfoPanSpeedSlider.get(), 
        isLearning && parameterID == "lfoPanSpeed");
    updateSliderLearningVisuals(lfoPanDepthSlider.get(), 
        isLearning && parameterID == "lfoPanDepth");
    updateSliderLearningVisuals(stereoFieldSlider.get(), 
        isLearning && parameterID == "stereoField");
}

// ============================================================================
// Private Methods - Setup
// ============================================================================

void SliderPanelComponent::setupAllControls()
{
    GUI_DEBUG("SliderPanelComponent: Setting up all controls");
    
    removeAllChildren();
    
    createMasterControls();
    createLFOControls();
    createADSRControls();
    createPanControl();
    
    GUI_DEBUG("SliderPanelComponent: All controls created");
}

void SliderPanelComponent::createMasterControls()
{
    // Master Gain (left)
    masterGainLabel = GuiHelpers::createSliderLabel(
        GuiConstants::TextConstants::MASTER_GAIN_LABEL, debugMode_);
    if (masterGainLabel) {
        addAndMakeVisible(masterGainLabel.get());
    }
    
    // CHANGED: Create MidiLearnSlider
    masterGainSlider = std::make_unique<MidiLearnSlider>(
        juce::Slider::LinearHorizontal, 
        juce::Slider::NoTextBox);
    if (masterGainSlider) {
        masterGainSlider->setRange(0.0, 127.0, 1.0);
        masterGainSlider->setValue(100.0);
        GuiHelpers::styleSlider(*masterGainSlider, debugMode_);
        masterGainSlider->addListener(this);
        
        // ADDED: Set right-click callback
        masterGainSlider->setRightClickCallback([this](juce::Point<int> pos) {
            GUI_DEBUG("Master Gain slider right-clicked!");
            showMidiLearnMenu(masterGainSlider.get(), pos);
        });
        
        addAndMakeVisible(masterGainSlider.get());
    }
    
    // Stereo Field (right)
    stereoFieldLabel = GuiHelpers::createSliderLabel(
        GuiConstants::TextConstants::STEREO_FIELD_LABEL, debugMode_);
    if (stereoFieldLabel) {
        addAndMakeVisible(stereoFieldLabel.get());
    }
    
    stereoFieldSlider = std::make_unique<MidiLearnSlider>(
        juce::Slider::LinearHorizontal, 
        juce::Slider::NoTextBox);
    if (stereoFieldSlider) {
        stereoFieldSlider->setRange(0.0, 127.0, 1.0);
        stereoFieldSlider->setValue(0.0);
        GuiHelpers::styleSlider(*stereoFieldSlider, debugMode_);
        stereoFieldSlider->addListener(this);
        
        stereoFieldSlider->setRightClickCallback([this](juce::Point<int> pos) {
            GUI_DEBUG("Stereo Field slider right-clicked!");
            showMidiLearnMenu(stereoFieldSlider.get(), pos);
        });
        
        addAndMakeVisible(stereoFieldSlider.get());
    }
}

void SliderPanelComponent::createLFOControls()
{
    // LFO Depth (left)
    lfoPanDepthLabel = GuiHelpers::createSliderLabel(
        GuiConstants::TextConstants::LFO_DEPTH_LABEL, debugMode_);
    if (lfoPanDepthLabel) {
        addAndMakeVisible(lfoPanDepthLabel.get());
    }
    
    lfoPanDepthSlider = std::make_unique<MidiLearnSlider>(
        juce::Slider::LinearHorizontal, 
        juce::Slider::NoTextBox);
    if (lfoPanDepthSlider) {
        lfoPanDepthSlider->setRange(0.0, 127.0, 1.0);
        lfoPanDepthSlider->setValue(0.0);
        GuiHelpers::styleSlider(*lfoPanDepthSlider, debugMode_);
        lfoPanDepthSlider->addListener(this);
        
        lfoPanDepthSlider->setRightClickCallback([this](juce::Point<int> pos) {
            GUI_DEBUG("LFO Depth slider right-clicked!");
            showMidiLearnMenu(lfoPanDepthSlider.get(), pos);
        });
        
        addAndMakeVisible(lfoPanDepthSlider.get());
    }
    
    // LFO Speed (right)
    lfoPanSpeedLabel = GuiHelpers::createSliderLabel(
        GuiConstants::TextConstants::LFO_SPEED_LABEL, debugMode_);
    if (lfoPanSpeedLabel) {
        addAndMakeVisible(lfoPanSpeedLabel.get());
    }
    
    lfoPanSpeedSlider = std::make_unique<MidiLearnSlider>(
        juce::Slider::LinearHorizontal, 
        juce::Slider::NoTextBox);
    if (lfoPanSpeedSlider) {
        lfoPanSpeedSlider->setRange(0.0, 127.0, 1.0);
        lfoPanSpeedSlider->setValue(0.0);
        GuiHelpers::styleSlider(*lfoPanSpeedSlider, debugMode_);
        lfoPanSpeedSlider->addListener(this);
        
        lfoPanSpeedSlider->setRightClickCallback([this](juce::Point<int> pos) {
            GUI_DEBUG("LFO Speed slider right-clicked!");
            showMidiLearnMenu(lfoPanSpeedSlider.get(), pos);
        });
        
        addAndMakeVisible(lfoPanSpeedSlider.get());
    }
}

void SliderPanelComponent::createADSRControls()
{
    // Attack (left)
    attackLabel = GuiHelpers::createSliderLabel(
        GuiConstants::TextConstants::ATTACK_LABEL, debugMode_);
    if (attackLabel) {
        addAndMakeVisible(attackLabel.get());
    }
    
    attackSlider = std::make_unique<MidiLearnSlider>(
        juce::Slider::LinearHorizontal, 
        juce::Slider::NoTextBox);
    if (attackSlider) {
        attackSlider->setRange(0.0, 127.0, 1.0);
        attackSlider->setValue(0.0);
        GuiHelpers::styleSlider(*attackSlider, debugMode_);
        attackSlider->addListener(this);
        
        attackSlider->setRightClickCallback([this](juce::Point<int> pos) {
            GUI_DEBUG("Attack slider right-clicked!");
            showMidiLearnMenu(attackSlider.get(), pos);
        });
        
        addAndMakeVisible(attackSlider.get());
    }
    
    // Release (right)
    releaseLabel = GuiHelpers::createSliderLabel(
        GuiConstants::TextConstants::RELEASE_LABEL, debugMode_);
    if (releaseLabel) {
        addAndMakeVisible(releaseLabel.get());
    }
    
    releaseSlider = std::make_unique<MidiLearnSlider>(
        juce::Slider::LinearHorizontal, 
        juce::Slider::NoTextBox);
    if (releaseSlider) {
        releaseSlider->setRange(0.0, 127.0, 1.0);
        releaseSlider->setValue(4.0);
        GuiHelpers::styleSlider(*releaseSlider, debugMode_);
        releaseSlider->addListener(this);
        
        releaseSlider->setRightClickCallback([this](juce::Point<int> pos) {
            GUI_DEBUG("Release slider right-clicked!");
            showMidiLearnMenu(releaseSlider.get(), pos);
        });
        
        addAndMakeVisible(releaseSlider.get());
    }
    
    // Sustain (left)
    sustainLevelLabel = GuiHelpers::createSliderLabel(
        GuiConstants::TextConstants::SUSTAIN_LABEL, debugMode_);
    if (sustainLevelLabel) {
        addAndMakeVisible(sustainLevelLabel.get());
    }
    
    sustainLevelSlider = std::make_unique<MidiLearnSlider>(
        juce::Slider::LinearHorizontal, 
        juce::Slider::NoTextBox);
    if (sustainLevelSlider) {
        sustainLevelSlider->setRange(0.0, 127.0, 1.0);
        sustainLevelSlider->setValue(127.0);
        GuiHelpers::styleSlider(*sustainLevelSlider, debugMode_);
        sustainLevelSlider->addListener(this);
        
        sustainLevelSlider->setRightClickCallback([this](juce::Point<int> pos) {
            GUI_DEBUG("Sustain Level slider right-clicked!");
            showMidiLearnMenu(sustainLevelSlider.get(), pos);
        });
        
        addAndMakeVisible(sustainLevelSlider.get());
    }
}

void SliderPanelComponent::createPanControl()
{
    // Master Pan (right)
    masterPanLabel = GuiHelpers::createSliderLabel(
        GuiConstants::TextConstants::MASTER_PAN_LABEL, debugMode_);
    if (masterPanLabel) {
        addAndMakeVisible(masterPanLabel.get());
    }
    
    masterPanSlider = std::make_unique<MidiLearnSlider>(
        juce::Slider::LinearHorizontal, 
        juce::Slider::NoTextBox);
    if (masterPanSlider) {
        masterPanSlider->setRange(0.0, 127.0, 1.0);
        masterPanSlider->setValue(64.0);
        GuiHelpers::styleSlider(*masterPanSlider, debugMode_);
        masterPanSlider->addListener(this);
        
        masterPanSlider->setRightClickCallback([this](juce::Point<int> pos) {
            GUI_DEBUG("Master Pan slider right-clicked!");
            showMidiLearnMenu(masterPanSlider.get(), pos);
        });
        
        addAndMakeVisible(masterPanSlider.get());
    }
}

void SliderPanelComponent::setupSliderAttachments()
{
    GUI_DEBUG("SliderPanelComponent: Setting up slider attachments");
    
    ParameterAttachmentManager::SliderSet sliderSet;
    sliderSet.masterGain = masterGainSlider.get();
    sliderSet.masterPan = masterPanSlider.get();
    sliderSet.attack = attackSlider.get();
    sliderSet.release = releaseSlider.get();
    sliderSet.sustainLevel = sustainLevelSlider.get();
    sliderSet.lfoPanSpeed = lfoPanSpeedSlider.get();
    sliderSet.lfoPanDepth = lfoPanDepthSlider.get();
    sliderSet.stereoField = stereoFieldSlider.get();
    
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
    layoutSliderRow(bounds, 
                    masterGainLabel.get(), masterGainSlider.get(),
                    stereoFieldLabel.get(), stereoFieldSlider.get());
    drawSeparator(bounds);
    
    layoutSliderRow(bounds,
                    lfoPanDepthLabel.get(), lfoPanDepthSlider.get(),
                    lfoPanSpeedLabel.get(), lfoPanSpeedSlider.get());
    drawSeparator(bounds);
    
    layoutSliderRow(bounds,
                    attackLabel.get(), attackSlider.get(),
                    releaseLabel.get(), releaseSlider.get());
    drawSeparator(bounds);
    
    layoutSliderRow(bounds,
                    sustainLevelLabel.get(), sustainLevelSlider.get(),
                    masterPanLabel.get(), masterPanSlider.get());
}

void SliderPanelComponent::layoutDebugMode(juce::Rectangle<int> bounds)
{
    const int spacing = 4;
    
    GuiHelpers::positionHorizontalSliderWithLabel(bounds, 
        masterGainLabel.get(), masterGainSlider.get());
    bounds.removeFromTop(spacing);
    
    GuiHelpers::positionHorizontalSliderWithLabel(bounds,
        stereoFieldLabel.get(), stereoFieldSlider.get());
    bounds.removeFromTop(spacing);
    
    GuiHelpers::positionHorizontalSliderWithLabel(bounds,
        lfoPanDepthLabel.get(), lfoPanDepthSlider.get());
    bounds.removeFromTop(spacing);
    
    GuiHelpers::positionHorizontalSliderWithLabel(bounds,
        lfoPanSpeedLabel.get(), lfoPanSpeedSlider.get());
    bounds.removeFromTop(spacing);
    
    GuiHelpers::positionHorizontalSliderWithLabel(bounds,
        attackLabel.get(), attackSlider.get());
    bounds.removeFromTop(spacing);
    
    GuiHelpers::positionHorizontalSliderWithLabel(bounds,
        releaseLabel.get(), releaseSlider.get());
    bounds.removeFromTop(spacing);
    
    GuiHelpers::positionHorizontalSliderWithLabel(bounds,
        sustainLevelLabel.get(), sustainLevelSlider.get());
    bounds.removeFromTop(spacing);
    
    GuiHelpers::positionHorizontalSliderWithLabel(bounds,
        masterPanLabel.get(), masterPanSlider.get());
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

void SliderPanelComponent::showMidiLearnMenu(juce::Slider* slider, juce::Point<int> position)
{
    juce::ignoreUnused(position);

    juce::PopupMenu menu;
    
    juce::String parameterID = getParameterIDForSlider(slider);
    if (parameterID.isEmpty()) return;
    
    int assignedCC = midiLearnManager_->getCCNumberForParameter(parameterID);
    
    if (assignedCC >= 0) {
        menu.addItem(1, "Learn MIDI CC (currently: CC " + juce::String(assignedCC) + ")");
        menu.addItem(2, "Clear MIDI CC");
    } else {
        menu.addItem(1, "Learn MIDI CC...");
    }
    
    menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(this),
        [this, slider, parameterID](int result) {
            if (result == 1) {
                juce::String displayName = getDisplayNameForSlider(slider);
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

juce::String SliderPanelComponent::getParameterIDForSlider(juce::Slider* slider) const
{
    if (slider == masterGainSlider.get()) return "masterGain";
    if (slider == masterPanSlider.get()) return "masterPan";
    if (slider == attackSlider.get()) return "attack";
    if (slider == releaseSlider.get()) return "release";
    if (slider == sustainLevelSlider.get()) return "sustainLevel";
    if (slider == lfoPanSpeedSlider.get()) return "lfoPanSpeed";
    if (slider == lfoPanDepthSlider.get()) return "lfoPanDepth";
    if (slider == stereoFieldSlider.get()) return "stereoField";
    return {};
}

juce::String SliderPanelComponent::getDisplayNameForSlider(juce::Slider* slider) const
{
    if (slider == masterGainSlider.get()) return "Master Gain";
    if (slider == masterPanSlider.get()) return "Master Pan";
    if (slider == attackSlider.get()) return "Attack";
    if (slider == releaseSlider.get()) return "Release";
    if (slider == sustainLevelSlider.get()) return "Sustain Level";
    if (slider == lfoPanSpeedSlider.get()) return "LFO Speed";
    if (slider == lfoPanDepthSlider.get()) return "LFO Depth";
    if (slider == stereoFieldSlider.get()) return "Stereo Field";
    return {};
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
    
    slider->repaint();
}

juce::Slider* SliderPanelComponent::findSliderByParameterID(const juce::String& parameterID)
{
    if (parameterID == "masterGain") return masterGainSlider.get();
    if (parameterID == "masterPan") return masterPanSlider.get();
    if (parameterID == "attack") return attackSlider.get();
    if (parameterID == "release") return releaseSlider.get();
    if (parameterID == "sustainLevel") return sustainLevelSlider.get();
    if (parameterID == "lfoPanSpeed") return lfoPanSpeedSlider.get();
    if (parameterID == "lfoPanDepth") return lfoPanDepthSlider.get();
    if (parameterID == "stereoField") return stereoFieldSlider.get();
    return nullptr;
}