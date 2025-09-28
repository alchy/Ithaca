/**
 * @file IthacaPluginEditor.cpp (Refactored)
 * @brief Zjednodušená implementace hlavního editoru s delegací na komponenty
 */

#include "IthacaPluginEditor.h"
#include "SliderPanelComponent.h"
#include "InfoPanelComponent.h"
#include "GuiHelpers.h"
#include "GuiConstants.h"
#include "decorators/BinaryData.h"
#include <iostream>

// ZACHOVAT: Všechny původní makra a konstanty
#define BACKGROUND_PICTURE_OFF 0
#define CURRENT_INSTRUMENT "Ithaca Grand Piano"
#define PLUGIN_VERSION "1.0.0"

// Makro pro debug výpisy (zachovat původní logiku)
#if BACKGROUND_PICTURE_OFF
#define GUI_DEBUG(msg) std::cout << msg << std::endl
#else
#define GUI_DEBUG(msg)
#endif

//==============================================================================
IthacaPluginEditor::IthacaPluginEditor(IthacaPluginProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p), debugMode_(isDebugModeEnabled())
{
    GUI_DEBUG("IthacaGUI: Constructor starting with HORIZONTAL SLIDERS");
    
    // ZACHOVAT: Původní velikost okna
    setSize(GuiConstants::DEFAULT_WINDOW_WIDTH, GuiConstants::DEFAULT_WINDOW_HEIGHT);
    
    setupBackground();        // Původní background logic
    initializeComponents();   // Vytvoření specializovaných komponent
    resized();               // Původní resized() volání
    
    GUI_DEBUG("IthacaGUI: Constructor completed with refactored components");
}

IthacaPluginEditor::~IthacaPluginEditor()
{
    // Specializované komponenty se automaticky vyčistí
    GUI_DEBUG("IthacaGUI: Destructor called");
}

//==============================================================================
void IthacaPluginEditor::paint(juce::Graphics& g)
{
    // ZACHOVAT: Přesně původní paint logiku
#if BACKGROUND_PICTURE_OFF
    GuiHelpers::applyDebugBackground(g, getLocalBounds());
#else
    // BACKGROUND MODE: Overlay pro čitelnost (původní logika)
    auto controlArea = getLocalBounds().removeFromTop(GuiConstants::CONTROL_AREA_HEIGHT);
    GuiHelpers::applyControlAreaOverlay(g, controlArea);
#endif
    
    GUI_DEBUG("IthacaGUI: Paint method - " << (debugMode_ ? "DEBUG" : "BACKGROUND") << " mode");
}

void IthacaPluginEditor::resized()
{
    auto bounds = getLocalBounds();
    
#if !BACKGROUND_PICTURE_OFF
    // ZACHOVAT: Původní layout logiku pro background mode
    imageComponent.setBounds(bounds);
    
    // === PŮVODNÍ LAYOUT: Sliders vlevo, info vpravo ===
    auto workArea = bounds.reduced(GuiConstants::COMPONENT_PADDING);
    
    // Levá část: Sliders (původní proporce)
    auto slidersArea = workArea.removeFromLeft(GuiConstants::SLIDERS_AREA_WIDTH);
    slidersArea.removeFromTop(20); // Top spacing
    
    // Pravá část: Info labels (původní pozice)
    auto infoArea = workArea;
    infoArea.removeFromTop(60); // Top spacing
    
    // Delegace layoutu na specializované komponenty
    if (sliderPanel) {
        sliderPanel->setBounds(slidersArea);
    }
    
    if (infoPanel) {
        infoPanel->setBounds(infoArea);
    }
    
    // Version label speciální umístění (zachovat původní pozici)
    // Bude handled v InfoPanelComponent, ale můžeme přepsat zde pro přesnou kontrolu
    
#else
    // DEBUG MODE: Původní debug layout
    auto controlArea = bounds.removeFromRight(150).removeFromTop(450).reduced(8, 20);
    
    if (infoPanel) {
        infoPanel->setBounds(controlArea);
    }
    
    // Slidery vlevo v debug módu
    auto slidersArea = bounds.removeFromLeft(300).reduced(8, 20);
    
    if (sliderPanel) {
        sliderPanel->setBounds(slidersArea);
    }
#endif
    
    GUI_DEBUG("IthacaGUI: Resized with specialized components layout");
}

void IthacaPluginEditor::parentHierarchyChanged()
{
    GUI_DEBUG("IthacaGUI: parentHierarchyChanged - delegating to InfoPanel");
    
    // ZACHOVAT: Původní timer management, ale delegovat na InfoPanel
    if (isShowing() && infoPanel) {
        infoPanel->startUpdates();
    }
}

//==============================================================================
void IthacaPluginEditor::initializeComponents()
{
    GUI_DEBUG("IthacaGUI: Initializing specialized components");
    
    // Vytvoření specializovaných komponent
    sliderPanel = std::make_unique<SliderPanelComponent>(processorRef.getParameters());
    if (sliderPanel) {
        sliderPanel->setDebugMode(debugMode_);
        addAndMakeVisible(sliderPanel.get());
        GUI_DEBUG("IthacaGUI: SliderPanelComponent created and added");
    }
    
    infoPanel = std::make_unique<InfoPanelComponent>(processorRef);
    if (infoPanel) {
        infoPanel->setDebugMode(debugMode_);
        addAndMakeVisible(infoPanel.get());
        GUI_DEBUG("IthacaGUI: InfoPanelComponent created and added");
    }
    
    // ZACHOVAT: VoiceActivityComponent pro kompatibilitu (prázdná implementace)
    voiceActivityGrid = std::make_unique<VoiceActivityComponent>();
    // Nepřidáváme do GUI - pouze pro kompatibilitu
}

void IthacaPluginEditor::setupBackground()
{
    // ZACHOVAT: Původní background logic
#if !BACKGROUND_PICTURE_OFF
    juce::Image image = juce::ImageCache::getFromMemory(
        BinaryData::ithacaplayer1_jpg, 
        BinaryData::ithacaplayer1_jpgSize);
    
    imageComponent.setImage(image);
    imageComponent.setImagePlacement(juce::RectanglePlacement::stretchToFit);
    imageComponent.setInterceptsMouseClicks(false, false);
    addAndMakeVisible(imageComponent);
    GUI_DEBUG("IthacaGUI: Background image loaded");
#else
    GUI_DEBUG("IthacaGUI: Background image DISABLED (debug mode)");
#endif
}

bool IthacaPluginEditor::isDebugModeEnabled() const
{
    return BACKGROUND_PICTURE_OFF != 0;
}

//==============================================================================
// ZACHOVAT: VoiceActivityComponent implementace pro kompatibilitu

IthacaPluginEditor::VoiceActivityComponent::VoiceActivityComponent()
{
    voiceStates.fill(0);
}

void IthacaPluginEditor::VoiceActivityComponent::paint(juce::Graphics& g)
{
    // Prázdná implementace pro kompatibilitu
    juce::ignoreUnused(g);
}

void IthacaPluginEditor::VoiceActivityComponent::resized()
{
    // Prázdná implementace pro kompatibilitu
}

void IthacaPluginEditor::VoiceActivityComponent::updateVoiceStates(int active, int sustaining, int releasing)
{
    // Prázdná implementace pro kompatibilitu
    juce::ignoreUnused(active, sustaining, releasing);
}

void IthacaPluginEditor::VoiceActivityComponent::setVoiceState(uint8_t midiNote, bool isActive, int voiceState)
{
    // Prázdná implementace pro kompatibilitu
    juce::ignoreUnused(midiNote, isActive, voiceState);
}

juce::Rectangle<int> IthacaPluginEditor::VoiceActivityComponent::getCellBounds(int row, int col) const
{
    // Prázdná implementace pro kompatibilitu
    juce::ignoreUnused(row, col);
    return juce::Rectangle<int>();
}

juce::Colour IthacaPluginEditor::VoiceActivityComponent::getStateColour(int state) const
{
    // Prázdná implementace pro kompatibilitu
    juce::ignoreUnused(state);
    return juce::Colour(0xff000000);
}

int IthacaPluginEditor::VoiceActivityComponent::getMidiNoteFromGrid(int row, int col) const
{
    // Prázdná implementace pro kompatibilitu
    juce::ignoreUnused(row, col);
    return 0;
}