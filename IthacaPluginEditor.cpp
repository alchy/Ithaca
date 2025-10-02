/**
 * @file IthacaPluginEditor.cpp
 * @brief Implementace hlavního editoru
 * 
 * ============================================================================
 * GUI REFACTORING - MAIN EDITOR IMPLEMENTATION (CLEAN VERSION)
 * ============================================================================
 */

#include "IthacaPluginEditor.h"
#include "SliderPanelComponent.h"
#include "InfoHeaderComponent.h"
#include "GuiHelpers.h"
#include "GuiConstants.h"
#include "decorators/BinaryData.h"
#include <iostream>

#define BACKGROUND_PICTURE_OFF 0
#define CURRENT_INSTRUMENT "Ithaca Grand Piano"
#define PLUGIN_VERSION "1.0.0"

#if BACKGROUND_PICTURE_OFF
#define GUI_DEBUG(msg) std::cout << msg << std::endl
#else
#define GUI_DEBUG(msg)
#endif

// ============================================================================
// Constructor / Destructor
// ============================================================================

IthacaPluginEditor::IthacaPluginEditor(IthacaPluginProcessor& p)
    : AudioProcessorEditor(&p), 
      processorRef(p), 
      debugMode_(isDebugModeEnabled())
{
    GUI_DEBUG("IthacaGUI: Constructor - Hierarchical Layout");
    
    setSize(GuiConstants::DEFAULT_WINDOW_WIDTH, 
            GuiConstants::DEFAULT_WINDOW_HEIGHT);
    
    setupBackground();
    initializeComponents();
    resized();
    
    GUI_DEBUG("IthacaGUI: Constructor completed");
}

IthacaPluginEditor::~IthacaPluginEditor()
{
    GUI_DEBUG("IthacaGUI: Destructor");
}

// ============================================================================
// Component Overrides
// ============================================================================

void IthacaPluginEditor::paint(juce::Graphics& g)
{
    juce::ignoreUnused(g);
    #if BACKGROUND_PICTURE_OFF
        // Debug mode: šedé pozadí
        GuiHelpers::applyDebugBackground(g, getLocalBounds());
    #endif
    // Background mode: komponenty se vykreslují samy přes background image
}

void IthacaPluginEditor::resized()
{
    auto bounds = getLocalBounds();
    
#if !BACKGROUND_PICTURE_OFF
    // === BACKGROUND MODE: Hierarchical layout ===
    
    // Background image na celé okno
    imageComponent.setBounds(bounds);
    
    // Vypočítat výšky sekcí podle ratio
    int windowHeight = bounds.getHeight();
    int infoHeight = static_cast<int>(windowHeight * 
                                     GuiConstants::INFO_SECTION_HEIGHT_RATIO);
    
    // Content area s paddingem
    auto contentArea = bounds.reduced(8);
    
    // Info header nahoře (~30%)
    if (infoHeader) {
        infoHeader->setBounds(contentArea.removeFromTop(infoHeight));
    }
    
    // Mezera mezi sekcemi
    contentArea.removeFromTop(GuiConstants::SECTION_GAP);
    
    // Slider panel dole (~70%, zbytek prostoru)
    if (sliderPanel) {
        sliderPanel->setBounds(contentArea);
    }
    
#else
    // === DEBUG MODE: Kompaktní layout ===
    
    auto contentArea = bounds.reduced(GuiConstants::SECTION_PADDING);
    
    // Info header nahoře (fixní výška v debug mode)
    if (infoHeader) {
        infoHeader->setBounds(contentArea.removeFromTop(120));
        contentArea.removeFromTop(GuiConstants::SECTION_GAP);
    }
    
    // Slider panel (zbytek)
    if (sliderPanel) {
        sliderPanel->setBounds(contentArea);
    }
#endif
    
    GUI_DEBUG("IthacaGUI: Resized - "
              << "info: " << (infoHeader ? infoHeader->getHeight() : 0) << "px, "
              << "sliders: " << (sliderPanel ? sliderPanel->getHeight() : 0) << "px");
}

void IthacaPluginEditor::parentHierarchyChanged()
{
    // Start timer pro live updates v info header
    if (isShowing() && infoHeader) {
        infoHeader->startUpdates();
        GUI_DEBUG("IthacaGUI: Info header timer started");
    }
}

// ============================================================================
// Private Methods - Setup
// ============================================================================

void IthacaPluginEditor::initializeComponents()
{
    GUI_DEBUG("IthacaGUI: Initializing hierarchical components");
    
    // Info header (nahoře)
    infoHeader = std::make_unique<InfoHeaderComponent>(processorRef);
    if (infoHeader) {
        infoHeader->setDebugMode(debugMode_);
        addAndMakeVisible(infoHeader.get());
        GUI_DEBUG("IthacaGUI: InfoHeaderComponent created");
    }
    
    // Slider panel (dole)
    sliderPanel = std::make_unique<SliderPanelComponent>(
        processorRef.getParameters());
    if (sliderPanel) {
        sliderPanel->setDebugMode(debugMode_);
        addAndMakeVisible(sliderPanel.get());
        GUI_DEBUG("IthacaGUI: SliderPanelComponent created");
    }
}

void IthacaPluginEditor::setupBackground()
{
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
    GUI_DEBUG("IthacaGUI: Background DISABLED (debug mode)");
#endif
}

bool IthacaPluginEditor::isDebugModeEnabled() const
{
    return BACKGROUND_PICTURE_OFF != 0;
}