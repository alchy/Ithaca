/**
 * @file SliderPanelComponent.cpp
 * @brief Implementace komponenty pro horizontální slidery
 */

#include "SliderPanelComponent.h"
#include "GuiConstants.h"
#include <iostream>

// DEBUG: Přepínač pro vypnutí background obrázku (zachovat původní logiku)
#define BACKGROUND_PICTURE_OFF 0

// Makro pro debug výpisy (zachovat původní logiku)
#if BACKGROUND_PICTURE_OFF
#define GUI_DEBUG(msg) std::cout << msg << std::endl
#else
#define GUI_DEBUG(msg)
#endif

SliderPanelComponent::SliderPanelComponent(juce::AudioProcessorValueTreeState& parameters)
    : parameters_(parameters), debugMode_(GuiHelpers::isDebugModeEnabled())
{
    GUI_DEBUG("SliderPanelComponent: Constructor starting with HORIZONTAL SLIDERS");
    
    setupAllControls();           // Vytvoří všechny slidery a labely
    setupSliderAttachments();     // Propojí s parametry
    
    GUI_DEBUG("SliderPanelComponent: Constructor completed - " << getNumChildComponents() << " child components");
}

SliderPanelComponent::~SliderPanelComponent()
{
    // Slider attachments se automaticky vyčistí
    GUI_DEBUG("SliderPanelComponent: Destructor called");
}

void SliderPanelComponent::paint(juce::Graphics& g)
{
    // TRANSPARENTNÍ - bez pozadí, nechá hlavní editor vykreslit overlay
    juce::ignoreUnused(g);
    GUI_DEBUG("SliderPanelComponent: Paint method - transparent mode");
}

void SliderPanelComponent::resized()
{
    auto bounds = getLocalBounds();
    
    if (debugMode_) {
        layoutDebugMode(bounds);
    } else {
        layoutBackgroundMode(bounds);
    }
    
    GUI_DEBUG("SliderPanelComponent: Resized with bounds " << bounds.toString().toStdString());
}

void SliderPanelComponent::sliderValueChanged(juce::Slider* slider)
{
    // Parameter attachments automaticky synchronizují s parametry
    // Můžeme přidat custom akce zde pokud potřebujeme
    
    if (slider == lfoPanSpeedSlider.get() || slider == lfoPanDepthSlider.get()) {
        // LFO parametry - můžeme přidat visual feedback
        GUI_DEBUG("SliderPanelComponent: LFO parameter changed");
    }
    
    // Pro debug můžeme logovat změny
    GUI_DEBUG("SliderPanelComponent: Slider value changed: " << slider->getValue());
}

void SliderPanelComponent::setDebugMode(bool enabled)
{
    if (debugMode_ != enabled) {
        debugMode_ = enabled;
        
        // Přestylovat všechny komponenty
        setupAllControls();  // Znovu vytvoří s novými barvami
        resized();           // Přelayoutuje
        
        GUI_DEBUG("SliderPanelComponent: Debug mode " << (enabled ? "ENABLED" : "DISABLED"));
    }
}

void SliderPanelComponent::setupAllControls()
{
    GUI_DEBUG("SliderPanelComponent: Setting up all controls - START");
    
    // Vymazat existující komponenty
    removeAllChildren();
    
    createMasterControls();
    createADSRControls();
    createLFOControls();
    createStereoFieldControls();
    
    GUI_DEBUG("SliderPanelComponent: All controls setup completed");
}

void SliderPanelComponent::createMasterControls()
{
    // === MASTER CONTROLS ===
    masterGainLabel = GuiHelpers::createSliderLabel(GuiConstants::TextConstants::MASTER_GAIN_LABEL, debugMode_);
    if (masterGainLabel) {
        addAndMakeVisible(masterGainLabel.get());
        GUI_DEBUG("SliderPanelComponent: masterGainLabel created");
    }
    
    masterGainSlider = GuiHelpers::createCompactSlider(0.0, 127.0, 100.0, 1.0);
    if (masterGainSlider) {
        masterGainSlider->addListener(this);
        addAndMakeVisible(masterGainSlider.get());
        GUI_DEBUG("SliderPanelComponent: masterGainSlider created (horizontal)");
    }
    
    masterPanLabel = GuiHelpers::createSliderLabel(GuiConstants::TextConstants::MASTER_PAN_LABEL, debugMode_);
    if (masterPanLabel) {
        addAndMakeVisible(masterPanLabel.get());
        GUI_DEBUG("SliderPanelComponent: masterPanLabel created");
    }
    
    masterPanSlider = GuiHelpers::createCompactSlider(0.0, 127.0, 64.0, 1.0);
    if (masterPanSlider) {
        masterPanSlider->addListener(this);
        addAndMakeVisible(masterPanSlider.get());
        GUI_DEBUG("SliderPanelComponent: masterPanSlider created (horizontal)");
    }
}

void SliderPanelComponent::createADSRControls()
{
    // === ADSR ENVELOPE ===
    attackLabel = GuiHelpers::createSliderLabel(GuiConstants::TextConstants::ATTACK_LABEL, debugMode_);
    if (attackLabel) {
        addAndMakeVisible(attackLabel.get());
        GUI_DEBUG("SliderPanelComponent: attackLabel created");
    }
    
    attackSlider = GuiHelpers::createCompactSlider(0.0, 127.0, 0.0, 1.0);
    if (attackSlider) {
        attackSlider->addListener(this);
        addAndMakeVisible(attackSlider.get());
        GUI_DEBUG("SliderPanelComponent: attackSlider created (horizontal)");
    }
    
    releaseLabel = GuiHelpers::createSliderLabel(GuiConstants::TextConstants::RELEASE_LABEL, debugMode_);
    if (releaseLabel) {
        addAndMakeVisible(releaseLabel.get());
        GUI_DEBUG("SliderPanelComponent: releaseLabel created");
    }
    
    releaseSlider = GuiHelpers::createCompactSlider(0.0, 127.0, 4.0, 1.0);
    if (releaseSlider) {
        releaseSlider->addListener(this);
        addAndMakeVisible(releaseSlider.get());
        GUI_DEBUG("SliderPanelComponent: releaseSlider created (horizontal)");
    }
    
    sustainLevelLabel = GuiHelpers::createSliderLabel(GuiConstants::TextConstants::SUSTAIN_LABEL, debugMode_);
    if (sustainLevelLabel) {
        addAndMakeVisible(sustainLevelLabel.get());
        GUI_DEBUG("SliderPanelComponent: sustainLevelLabel created");
    }
    
    sustainLevelSlider = GuiHelpers::createCompactSlider(0.0, 127.0, 127.0, 1.0);
    if (sustainLevelSlider) {
        sustainLevelSlider->addListener(this);
        addAndMakeVisible(sustainLevelSlider.get());
        GUI_DEBUG("SliderPanelComponent: sustainLevelSlider created (horizontal)");
    }
}

void SliderPanelComponent::createLFOControls()
{
    // === LFO PANNING ===
    lfoPanSpeedLabel = GuiHelpers::createSliderLabel(GuiConstants::TextConstants::LFO_SPEED_LABEL, debugMode_);
    if (lfoPanSpeedLabel) {
        addAndMakeVisible(lfoPanSpeedLabel.get());
        GUI_DEBUG("SliderPanelComponent: lfoPanSpeedLabel created");
    }
    
    lfoPanSpeedSlider = GuiHelpers::createCompactSlider(0.0, 127.0, 0.0, 1.0);
    if (lfoPanSpeedSlider) {
        lfoPanSpeedSlider->addListener(this);
        addAndMakeVisible(lfoPanSpeedSlider.get());
        GUI_DEBUG("SliderPanelComponent: lfoPanSpeedSlider created (horizontal)");
    }
    
    lfoPanDepthLabel = GuiHelpers::createSliderLabel(GuiConstants::TextConstants::LFO_DEPTH_LABEL, debugMode_);
    if (lfoPanDepthLabel) {
        addAndMakeVisible(lfoPanDepthLabel.get());
        GUI_DEBUG("SliderPanelComponent: lfoPanDepthLabel created");
    }
    
    lfoPanDepthSlider = GuiHelpers::createCompactSlider(0.0, 127.0, 0.0, 1.0);
    if (lfoPanDepthSlider) {
        lfoPanDepthSlider->addListener(this);
        addAndMakeVisible(lfoPanDepthSlider.get());
        GUI_DEBUG("SliderPanelComponent: lfoPanDepthSlider created (horizontal)");
    }
}

void SliderPanelComponent::createStereoFieldControls()
{
    // === STEREO FIELD ===
    stereoFieldLabel = GuiHelpers::createSliderLabel("Stereo Field", debugMode_);
    if (stereoFieldLabel) {
        addAndMakeVisible(stereoFieldLabel.get());
        GUI_DEBUG("SliderPanelComponent: stereoFieldLabel created");
    }
    
    stereoFieldSlider = GuiHelpers::createCompactSlider(0.0, 127.0, 0.0, 1.0);
    if (stereoFieldSlider) {
        stereoFieldSlider->addListener(this);
        addAndMakeVisible(stereoFieldSlider.get());
        GUI_DEBUG("SliderPanelComponent: stereoFieldSlider created (horizontal)");
    }
}

void SliderPanelComponent::setupSliderAttachments()
{
    GUI_DEBUG("SliderPanelComponent: Setting up slider attachments - START");
    
    // Připravi SliderSet strukturu
    ParameterAttachmentManager::SliderSet sliderSet;
    sliderSet.masterGain = masterGainSlider.get();
    sliderSet.masterPan = masterPanSlider.get();
    sliderSet.attack = attackSlider.get();
    sliderSet.release = releaseSlider.get();
    sliderSet.sustainLevel = sustainLevelSlider.get();
    sliderSet.lfoPanSpeed = lfoPanSpeedSlider.get();
    sliderSet.lfoPanDepth = lfoPanDepthSlider.get();
    sliderSet.stereoField = stereoFieldSlider.get(); 
    
    // Vytvoř všechny attachments
    bool success = attachmentManager_.createAllAttachments(parameters_, sliderSet);
    
    if (success) {
        GUI_DEBUG("SliderPanelComponent: All slider attachments created successfully");
    } else {
        GUI_DEBUG("SliderPanelComponent: Some slider attachments failed - check parameter IDs");
    }
    
    attachmentManager_.logAttachmentStatus();
}

void SliderPanelComponent::layoutBackgroundMode(juce::Rectangle<int> bounds)
{
    // ZACHOVAT: Původní dvousloupcový layout pro background mode
    auto leftColumn = bounds.removeFromLeft(150);
    auto rightColumn = bounds.removeFromLeft(150);
    
    // Levé slidery: Master controls
    GuiHelpers::positionHorizontalSliderWithLabel(leftColumn, masterGainLabel.get(), masterGainSlider.get());
    leftColumn.removeFromTop(4); // Extra spacing
    GuiHelpers::positionHorizontalSliderWithLabel(leftColumn, masterPanLabel.get(), masterPanSlider.get());
    
    // Pravé slidery: ADSR + LFO (kompaktně)
    GuiHelpers::positionHorizontalSliderWithLabel(rightColumn, attackLabel.get(), attackSlider.get());
    GuiHelpers::positionHorizontalSliderWithLabel(rightColumn, releaseLabel.get(), releaseSlider.get());
    rightColumn.removeFromTop(4); // Extra spacing
    GuiHelpers::positionHorizontalSliderWithLabel(rightColumn, sustainLevelLabel.get(), sustainLevelSlider.get());
    rightColumn.removeFromTop(4); // Extra spacing před LFO
    
    // LFO menší
    if (lfoPanSpeedLabel) {
        lfoPanSpeedLabel->setBounds(rightColumn.removeFromTop(14));
        rightColumn.removeFromTop(1);
    }
    if (lfoPanSpeedSlider) {
        lfoPanSpeedSlider->setBounds(rightColumn.removeFromTop(20)); // Menší výška
        rightColumn.removeFromTop(3);
    }
    
    if (lfoPanDepthLabel) {
        lfoPanDepthLabel->setBounds(rightColumn.removeFromTop(14));
        rightColumn.removeFromTop(1);
    }
    if (lfoPanDepthSlider) {
        lfoPanDepthSlider->setBounds(rightColumn.removeFromTop(20)); // Menší výška
    }

    rightColumn.removeFromTop(4); // Extra spacing
    if (stereoFieldLabel) {
        stereoFieldLabel->setBounds(rightColumn.removeFromTop(14));
        rightColumn.removeFromTop(1);
    }
    if (stereoFieldSlider) {
        stereoFieldSlider->setBounds(rightColumn.removeFromTop(20));
    }

    
    GUI_DEBUG("SliderPanelComponent: Background mode layout completed");
}

void SliderPanelComponent::layoutDebugMode(juce::Rectangle<int> bounds)
{
    // DEBUG MODE: Jednoduché vertikální řazení
    auto slidersArea = bounds.reduced(GuiConstants::COMPONENT_PADDING);
    
    // Master controls
    GuiHelpers::positionHorizontalSliderWithLabel(slidersArea, masterGainLabel.get(), masterGainSlider.get());
    GuiHelpers::positionHorizontalSliderWithLabel(slidersArea, masterPanLabel.get(), masterPanSlider.get());
    
    slidersArea.removeFromTop(8); // Section spacing
    
    // ADSR
    GuiHelpers::positionHorizontalSliderWithLabel(slidersArea, attackLabel.get(), attackSlider.get());
    GuiHelpers::positionHorizontalSliderWithLabel(slidersArea, releaseLabel.get(), releaseSlider.get());
    GuiHelpers::positionHorizontalSliderWithLabel(slidersArea, sustainLevelLabel.get(), sustainLevelSlider.get());
    
    slidersArea.removeFromTop(8); // Section spacing
    
    // LFO
    GuiHelpers::positionHorizontalSliderWithLabel(slidersArea, lfoPanSpeedLabel.get(), lfoPanSpeedSlider.get());
    GuiHelpers::positionHorizontalSliderWithLabel(slidersArea, lfoPanDepthLabel.get(), lfoPanDepthSlider.get());

    slidersArea.removeFromTop(8); // Section spacing
    
    // Stereo Field
    GuiHelpers::positionHorizontalSliderWithLabel(slidersArea, stereoFieldLabel.get(), stereoFieldSlider.get());
    
    GUI_DEBUG("SliderPanelComponent: Debug mode layout completed");
}