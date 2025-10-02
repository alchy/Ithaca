/**
 * @file SliderPanelComponent.cpp (COMPLETE VERSION)
 * @brief Hierarchický layout sliderů s rounded overlay
 * 
 * ============================================================================
 * GUI REFACTORING - SLIDER PANEL IMPLEMENTATION (COMPLETE)
 * ============================================================================
 * 
 * Layout struktura (4 řádky, každý 50/50 split):
 * Row 1: Master Gain | Stereo Field
 * Row 2: LFO Depth | LFO Speed
 * Row 3: Attack | Release
 * Row 4: Sustain | Master Pan
 * 
 * Mezi řádky: Průhledné separátory (60% alpha, 1px)
 * Overlay: 60% alpha, 6px rounded corners
 * ============================================================================
 */

#include "SliderPanelComponent.h"
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

SliderPanelComponent::SliderPanelComponent(juce::AudioProcessorValueTreeState& parameters)
    : parameters_(parameters), debugMode_(GuiHelpers::isDebugModeEnabled())
{
    GUI_DEBUG("SliderPanelComponent: Constructor - Hierarchical Layout");
    
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
        // Zaoblené overlay pozadí (60% alpha, 6px radius)
        GuiHelpers::drawRoundedOverlay(g, bounds, 
                                      GuiConstants::SLIDER_OVERLAY_ALPHA,
                                      GuiConstants::PANEL_CORNER_RADIUS);
        
        // Vykreslí separátory mezi řádky
        paintSeparators(g);
    }
}

void SliderPanelComponent::resized()
{
    // Vyčistit předchozí separator pozice
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
    // Parameter attachments automatically sync
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

// ============================================================================
// Private Methods - Setup
// ============================================================================

void SliderPanelComponent::setupAllControls()
{
    GUI_DEBUG("SliderPanelComponent: Setting up all controls");
    
    removeAllChildren();
    
    createMasterControls();    // Row 1: Master Gain, Stereo Field
    createLFOControls();        // Row 2: LFO Depth, LFO Speed
    createADSRControls();       // Row 3: Attack, Release, Sustain
    createPanControl();         // Row 4: Master Pan
    
    GUI_DEBUG("SliderPanelComponent: All controls created");
}

void SliderPanelComponent::createMasterControls()
{
    // === ROW 1: Master Gain | Stereo Field ===
    
    // Master Gain (left)
    masterGainLabel = GuiHelpers::createSliderLabel(
        GuiConstants::TextConstants::MASTER_GAIN_LABEL, debugMode_);
    if (masterGainLabel) {
        addAndMakeVisible(masterGainLabel.get());
    }
    
    masterGainSlider = GuiHelpers::createCompactSlider(0.0, 127.0, 100.0, 1.0);
    if (masterGainSlider) {
        masterGainSlider->addListener(this);
        addAndMakeVisible(masterGainSlider.get());
    }
    
    // Stereo Field (right)
    stereoFieldLabel = GuiHelpers::createSliderLabel(
        GuiConstants::TextConstants::STEREO_FIELD_LABEL, debugMode_);
    if (stereoFieldLabel) {
        addAndMakeVisible(stereoFieldLabel.get());
    }
    
    stereoFieldSlider = GuiHelpers::createCompactSlider(0.0, 127.0, 0.0, 1.0);
    if (stereoFieldSlider) {
        stereoFieldSlider->addListener(this);
        addAndMakeVisible(stereoFieldSlider.get());
    }
}

void SliderPanelComponent::createLFOControls()
{
    // === ROW 2: LFO Depth | LFO Speed ===
    
    // LFO Depth (left)
    lfoPanDepthLabel = GuiHelpers::createSliderLabel(
        GuiConstants::TextConstants::LFO_DEPTH_LABEL, debugMode_);
    if (lfoPanDepthLabel) {
        addAndMakeVisible(lfoPanDepthLabel.get());
    }
    
    lfoPanDepthSlider = GuiHelpers::createCompactSlider(0.0, 127.0, 0.0, 1.0);
    if (lfoPanDepthSlider) {
        lfoPanDepthSlider->addListener(this);
        addAndMakeVisible(lfoPanDepthSlider.get());
    }
    
    // LFO Speed (right)
    lfoPanSpeedLabel = GuiHelpers::createSliderLabel(
        GuiConstants::TextConstants::LFO_SPEED_LABEL, debugMode_);
    if (lfoPanSpeedLabel) {
        addAndMakeVisible(lfoPanSpeedLabel.get());
    }
    
    lfoPanSpeedSlider = GuiHelpers::createCompactSlider(0.0, 127.0, 0.0, 1.0);
    if (lfoPanSpeedSlider) {
        lfoPanSpeedSlider->addListener(this);
        addAndMakeVisible(lfoPanSpeedSlider.get());
    }
}

void SliderPanelComponent::createADSRControls()
{
    // === ROW 3: Attack | Release ===
    
    // Attack (left)
    attackLabel = GuiHelpers::createSliderLabel(
        GuiConstants::TextConstants::ATTACK_LABEL, debugMode_);
    if (attackLabel) {
        addAndMakeVisible(attackLabel.get());
    }
    
    attackSlider = GuiHelpers::createCompactSlider(0.0, 127.0, 0.0, 1.0);
    if (attackSlider) {
        attackSlider->addListener(this);
        addAndMakeVisible(attackSlider.get());
    }
    
    // Release (right)
    releaseLabel = GuiHelpers::createSliderLabel(
        GuiConstants::TextConstants::RELEASE_LABEL, debugMode_);
    if (releaseLabel) {
        addAndMakeVisible(releaseLabel.get());
    }
    
    releaseSlider = GuiHelpers::createCompactSlider(0.0, 127.0, 4.0, 1.0);
    if (releaseSlider) {
        releaseSlider->addListener(this);
        addAndMakeVisible(releaseSlider.get());
    }
    
    // === ROW 4: Sustain (left) ===
    sustainLevelLabel = GuiHelpers::createSliderLabel(
        GuiConstants::TextConstants::SUSTAIN_LABEL, debugMode_);
    if (sustainLevelLabel) {
        addAndMakeVisible(sustainLevelLabel.get());
    }
    
    sustainLevelSlider = GuiHelpers::createCompactSlider(0.0, 127.0, 127.0, 1.0);
    if (sustainLevelSlider) {
        sustainLevelSlider->addListener(this);
        addAndMakeVisible(sustainLevelSlider.get());
    }
}

void SliderPanelComponent::createPanControl()
{
    // === ROW 4: Master Pan (right) ===
    masterPanLabel = GuiHelpers::createSliderLabel(
        GuiConstants::TextConstants::MASTER_PAN_LABEL, debugMode_);
    if (masterPanLabel) {
        addAndMakeVisible(masterPanLabel.get());
    }
    
    masterPanSlider = GuiHelpers::createCompactSlider(0.0, 127.0, 64.0, 1.0);
    if (masterPanSlider) {
        masterPanSlider->addListener(this);
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
    } else {
        GUI_DEBUG("SliderPanelComponent: Some attachments failed");
    }
}

// ============================================================================
// Private Methods - Layout
// ============================================================================

void SliderPanelComponent::layoutBackgroundMode(juce::Rectangle<int> bounds)
{
    // Hierarchický layout - 4 řádky sliderů (50/50 split) se separátory
    
    // === ROW 1: Master Gain | Stereo Field ===
    layoutSliderRow(bounds, 
                    masterGainLabel.get(), masterGainSlider.get(),
                    stereoFieldLabel.get(), stereoFieldSlider.get());
    
    // Separator
    drawSeparator(bounds);
    
    // === ROW 2: LFO Depth | LFO Speed ===
    layoutSliderRow(bounds,
                    lfoPanDepthLabel.get(), lfoPanDepthSlider.get(),
                    lfoPanSpeedLabel.get(), lfoPanSpeedSlider.get());
    
    // Separator
    drawSeparator(bounds);
    
    // === ROW 3: Attack | Release ===
    layoutSliderRow(bounds,
                    attackLabel.get(), attackSlider.get(),
                    releaseLabel.get(), releaseSlider.get());
    
    // Separator
    drawSeparator(bounds);
    
    // === ROW 4: Sustain | Master Pan ===
    layoutSliderRow(bounds,
                    sustainLevelLabel.get(), sustainLevelSlider.get(),
                    masterPanLabel.get(), masterPanSlider.get());
    
    GUI_DEBUG("SliderPanelComponent: Background mode layout completed");
}

void SliderPanelComponent::layoutDebugMode(juce::Rectangle<int> bounds)
{
    // Debug mode: vertikální layout bez separátorů
    const int spacing = 4;
    
    // Master controls
    GuiHelpers::positionHorizontalSliderWithLabel(bounds, 
        masterGainLabel.get(), masterGainSlider.get());
    bounds.removeFromTop(spacing);
    
    GuiHelpers::positionHorizontalSliderWithLabel(bounds,
        stereoFieldLabel.get(), stereoFieldSlider.get());
    bounds.removeFromTop(spacing);
    
    // LFO controls
    GuiHelpers::positionHorizontalSliderWithLabel(bounds,
        lfoPanDepthLabel.get(), lfoPanDepthSlider.get());
    bounds.removeFromTop(spacing);
    
    GuiHelpers::positionHorizontalSliderWithLabel(bounds,
        lfoPanSpeedLabel.get(), lfoPanSpeedSlider.get());
    bounds.removeFromTop(spacing);
    
    // ADSR controls
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
    
    GUI_DEBUG("SliderPanelComponent: Debug mode layout completed");
}

// ============================================================================
// Private Helper Methods
// ============================================================================

void SliderPanelComponent::layoutSliderRow(juce::Rectangle<int>& bounds,
                                           juce::Label* leftLabel, juce::Slider* leftSlider,
                                           juce::Label* rightLabel, juce::Slider* rightSlider)
{
    // Vezme řádek z bounds a rozloží ho na 50/50 sloupce
    auto rowArea = bounds.removeFromTop(GuiConstants::SLIDER_ROW_HEIGHT);
    
    // 50/50 split s malou mezerou
    int halfWidth = rowArea.getWidth() / 2;
    int columnSpacing = GuiConstants::COLUMN_SPACING;
    
    auto leftColumn = rowArea.removeFromLeft(halfWidth - columnSpacing / 2);
    rowArea.removeFromLeft(columnSpacing); // Mezera mezi sloupci
    auto rightColumn = rowArea;
    
    // Layout levého sloupce
    GuiHelpers::positionHorizontalSliderWithLabel(leftColumn, leftLabel, leftSlider);
    
    // Layout pravého sloupce
    GuiHelpers::positionHorizontalSliderWithLabel(rightColumn, rightLabel, rightSlider);
    
    // Mezera za řádkem
    bounds.removeFromTop(GuiConstants::SECTION_SPACING);
}

void SliderPanelComponent::drawSeparator(juce::Rectangle<int>& bounds)
{
    // Uloží si pozici pro vykreslení separátoru
    separatorPositions_.push_back(bounds.getY());
    
    // Malá mezera pro separator
    bounds.removeFromTop(2);
}

void SliderPanelComponent::paintSeparators(juce::Graphics& g)
{
    // Vykreslí všechny separátory (volá se z paint())
    if (debugMode_) {
        return; // Žádné separátory v debug mode
    }
    
    auto bounds = getLocalBounds();
    int leftMargin = GuiConstants::SECTION_PADDING + 4;
    int rightMargin = bounds.getWidth() - GuiConstants::SECTION_PADDING - 4;
    
    for (int y : separatorPositions_) {
        GuiHelpers::drawSeparatorLine(g, leftMargin, y, rightMargin, y);
    }
}