/**
 * @file IthacaPluginEditor.cpp (COMPLETE with MIDI Learn)
 * @brief Implementace hlavního editoru s MIDI Learn podporou
 * 
 * ============================================================================
 * GUI REFACTORING - MAIN EDITOR IMPLEMENTATION (COMPLETE)
 * ============================================================================
 */

#include "ithaca/gui/IthacaPluginEditor.h"
#include "ithaca/gui/components/SliderPanelComponent.h"
#include "ithaca/gui/components/InfoHeaderComponent.h"
#include "ithaca/gui/helpers/GuiHelpers.h"
#include "ithaca/gui/helpers/GuiConstants.h"
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
    GUI_DEBUG("IthacaGUI: Constructor - Hierarchical Layout with MIDI Learn");
    
    // Set window size
    setSize(GuiConstants::DEFAULT_WINDOW_WIDTH, 
            GuiConstants::DEFAULT_WINDOW_HEIGHT);
    
    // Setup components in order
    setupBackground();           // Background image (if not debug mode)
    initializeComponents();      // Create InfoHeader and SliderPanel
    setupMidiLearnCallbacks();   // Register MIDI Learn callbacks
    resized();                   // Layout components
    
    GUI_DEBUG("IthacaGUI: Constructor completed");
}

IthacaPluginEditor::~IthacaPluginEditor()
{
    GUI_DEBUG("IthacaGUI: Destructor - Unregistering MIDI Learn callbacks");
    
    // Unregister MIDI Learn callback to prevent dangling pointer
    if (auto* midiLearnManager = processorRef.getMidiLearnManager()) {
        midiLearnManager->setLearningStateCallback(nullptr);
        GUI_DEBUG("IthacaGUI: MIDI Learn callbacks unregistered");
    }
    
    GUI_DEBUG("IthacaGUI: Destructor completed");
}

// ============================================================================
// Component Overrides
// ============================================================================

void IthacaPluginEditor::paint(juce::Graphics& g)
{
    juce::ignoreUnused(g);
    
#if BACKGROUND_PICTURE_OFF
    // Debug mode: Šedé pozadí s titulkem
    GuiHelpers::applyDebugBackground(g, getLocalBounds());
    GUI_DEBUG("IthacaGUI: Paint - Debug mode background rendered");
#else
    // Background mode: Komponenty se vykreslují samy přes background image
    // (imageComponent je už viditelný jako child component)
    GUI_DEBUG("IthacaGUI: Paint - Background mode (no custom painting needed)");
#endif
}

void IthacaPluginEditor::resized()
{
    auto bounds = getLocalBounds();
    
    GUI_DEBUG("IthacaGUI: Resized - Window size: " << bounds.getWidth() 
              << "x" << bounds.getHeight());

    #if !BACKGROUND_PICTURE_OFF
    // === BACKGROUND MODE: Hierarchical layout bez mezer ===
    
    // Background image na celé okno (non-interactive)
    imageComponent.setBounds(bounds);
    GUI_DEBUG("IthacaGUI: Background image set to full window");
    
    // Vypočítat výšky sekcí podle ratio
    int windowHeight = bounds.getHeight();
    int infoHeight = static_cast<int>(windowHeight * 
                                     GuiConstants::INFO_SECTION_HEIGHT_RATIO);
    
    GUI_DEBUG("IthacaGUI: Info header height: " << infoHeight 
              << "px (~" << (GuiConstants::INFO_SECTION_HEIGHT_RATIO * 100) << "%)");
    
    // ZMĚNĚNO: Content area BEZ paddingu - použije celou plochu okna
    auto contentArea = bounds;  // Původně: bounds.reduced(8)
    
    // Info header nahoře (~30%)
    if (infoHeader) {
        infoHeader->setBounds(contentArea.removeFromTop(infoHeight));
        GUI_DEBUG("IthacaGUI: Info header positioned");
    }
    
    // Mezera mezi sekcemi - NYNÍ 0, takže žádná mezera
    contentArea.removeFromTop(GuiConstants::SECTION_GAP);  // = 0
    
    // Slider panel dole (~70%, zbytek prostoru)
    if (sliderPanel) {
        sliderPanel->setBounds(contentArea);
        GUI_DEBUG("IthacaGUI: Slider panel positioned - height: " 
                  << contentArea.getHeight() << "px");
    }
    
    #else
    // === DEBUG MODE: Kompaktní layout ===
    
    // ZMĚNĚNO: BEZ paddingu v debug režimu
    auto contentArea = bounds;  // Původně: bounds.reduced(GuiConstants::SECTION_PADDING)
    
    if (infoHeader) {
        infoHeader->setBounds(contentArea.removeFromTop(120));
        contentArea.removeFromTop(GuiConstants::SECTION_GAP);  // = 0
        GUI_DEBUG("IthacaGUI: Debug - Info header: 120px");
    }
    
    if (sliderPanel) {
        sliderPanel->setBounds(contentArea);
        GUI_DEBUG("IthacaGUI: Debug - Slider panel: " 
                  << contentArea.getHeight() << "px");
    }
#endif
    
    GUI_DEBUG("IthacaGUI: Layout completed - "
              << "Info: " << (infoHeader ? infoHeader->getHeight() : 0) << "px, "
              << "Sliders: " << (sliderPanel ? sliderPanel->getHeight() : 0) << "px");
}

void IthacaPluginEditor::parentHierarchyChanged()
{
    // Start timer pro live updates v info header when component becomes visible
    if (isShowing() && infoHeader) {
        infoHeader->startUpdates();
        GUI_DEBUG("IthacaGUI: Component shown - Info header timer started");
    } else if (!isShowing() && infoHeader) {
        infoHeader->stopUpdates();
        GUI_DEBUG("IthacaGUI: Component hidden - Info header timer stopped");
    }
}

// ============================================================================
// Private Methods - Setup
// ============================================================================

void IthacaPluginEditor::initializeComponents()
{
    GUI_DEBUG("IthacaGUI: Initializing hierarchical components with MIDI Learn");
    
    // Info header (nahoře) - shows instrument info, stats, loading status
    infoHeader = std::make_unique<InfoHeaderComponent>(processorRef);
    if (infoHeader) {
        infoHeader->setDebugMode(debugMode_);
        addAndMakeVisible(infoHeader.get());
        GUI_DEBUG("IthacaGUI: InfoHeaderComponent created");
    } else {
        GUI_DEBUG("IthacaGUI: ERROR - Failed to create InfoHeaderComponent");
    }
    
    // Slider panel (dole) - parameter controls with MIDI Learn
    sliderPanel = std::make_unique<SliderPanelComponent>(
        processorRef.getParameters(),
        processorRef.getMidiLearnManager()  // Pass MIDI Learn Manager pointer
    );
    if (sliderPanel) {
        sliderPanel->setDebugMode(debugMode_);
        addAndMakeVisible(sliderPanel.get());
        GUI_DEBUG("IthacaGUI: SliderPanelComponent created with MIDI Learn support");
    } else {
        GUI_DEBUG("IthacaGUI: ERROR - Failed to create SliderPanelComponent");
    }
    
    GUI_DEBUG("IthacaGUI: Component initialization completed");
}

void IthacaPluginEditor::setupBackground()
{
#if !BACKGROUND_PICTURE_OFF
    // ZMĚNĚNO: Dynamické načítání background.jpg ze složky decorators
    
    // 1. Zkusit načíst relativně k executable (pro development)
    juce::File backgroundFile = juce::File::getSpecialLocation(
        juce::File::currentExecutableFile).getParentDirectory().getChildFile("decorators/background.jpg");
    
    // 2. Fallback: Zkusit relativně k current working directory
    if (!backgroundFile.existsAsFile()) {
        backgroundFile = juce::File::getCurrentWorkingDirectory().getChildFile("decorators/background.jpg");
    }
    
    // 3. Fallback: Zkusit absolutní cestu (pro installed plugin)
    #ifdef _WIN32
    if (!backgroundFile.existsAsFile()) {
        backgroundFile = juce::File("C:/ProgramData/IthacaPlayer/decorators/background.jpg");
    }
    #elif __APPLE__
    if (!backgroundFile.existsAsFile()) {
        backgroundFile = juce::File("~/Library/Application Support/IthacaPlayer/decorators/background.jpg");
    }
    #endif
    
    // Načíst obrázek ze souboru
    if (backgroundFile.existsAsFile()) {
        juce::Image image = juce::ImageFileFormat::loadFrom(backgroundFile);
        
        if (image.isValid()) {
            imageComponent.setImage(image);
            imageComponent.setImagePlacement(juce::RectanglePlacement::stretchToFit);
            imageComponent.setInterceptsMouseClicks(false, false);  // Non-interactive
            addAndMakeVisible(imageComponent);
            
            GUI_DEBUG("IthacaGUI: Background image loaded successfully from: " 
                      << backgroundFile.getFullPathName().toStdString());
            GUI_DEBUG("IthacaGUI: Image size: " << image.getWidth() << "x" 
                      << image.getHeight() << "px");
        } else {
            GUI_DEBUG("IthacaGUI: ERROR - Failed to decode image from file: " 
                      << backgroundFile.getFullPathName().toStdString());
            createFallbackBackground();
        }
    } else {
        GUI_DEBUG("IthacaGUI: WARNING - background.jpg not found at: " 
                  << backgroundFile.getFullPathName().toStdString());
        GUI_DEBUG("IthacaGUI: Searched paths:");
        GUI_DEBUG("  1. " << juce::File::getSpecialLocation(
            juce::File::currentExecutableFile).getParentDirectory()
            .getChildFile("decorators/background.jpg").getFullPathName().toStdString());
        GUI_DEBUG("  2. " << juce::File::getCurrentWorkingDirectory()
            .getChildFile("decorators/background.jpg").getFullPathName().toStdString());
        
        createFallbackBackground();
    }
#else
    GUI_DEBUG("IthacaGUI: Background DISABLED (debug mode - BACKGROUND_PICTURE_OFF)");
#endif
}

void IthacaPluginEditor::setupMidiLearnCallbacks()
{
    GUI_DEBUG("IthacaGUI: Setting up MIDI Learn callbacks");
    
    // Get MIDI Learn Manager from processor
    auto* midiLearnManager = processorRef.getMidiLearnManager();
    if (!midiLearnManager) {
        GUI_DEBUG("IthacaGUI: WARNING - MidiLearnManager is nullptr, callbacks not registered");
        return;
    }
    
    // Register callback for learning state changes
    // Lambda captures 'this' to access sliderPanel member
    midiLearnManager->setLearningStateCallback(
        [this](bool isLearning, const juce::String& parameterID) {
            // Forward learning state changes to slider panel for visual updates
            if (sliderPanel) {
                sliderPanel->onLearningStateChanged(isLearning, parameterID);
                
                GUI_DEBUG("IthacaGUI: MIDI Learn state forwarded to SliderPanel - "
                         << "Learning: " << (isLearning ? "YES" : "NO") 
                         << ", Parameter: " << parameterID.toStdString());
            } else {
                GUI_DEBUG("IthacaGUI: WARNING - SliderPanel is nullptr, "
                         << "cannot forward learning state");
            }
            
            // Additional debug info
            if (isLearning) {
                GUI_DEBUG("IthacaGUI: MIDI Learn ACTIVE - Waiting for CC from controller");
                GUI_DEBUG("IthacaGUI: Target parameter: " << parameterID.toStdString());
            } else {
                GUI_DEBUG("IthacaGUI: MIDI Learn INACTIVE");
            }
        }
    );
    
    GUI_DEBUG("IthacaGUI: MIDI Learn callbacks registered successfully");
}

bool IthacaPluginEditor::isDebugModeEnabled() const
{
    return BACKGROUND_PICTURE_OFF != 0;
}

void IthacaPluginEditor::createFallbackBackground()
{
    // Vytvořit jednobarevné fallback pozadí když se nepodaří načíst obrázek
    juce::Image fallbackImage(juce::Image::RGB, 
                             GuiConstants::DEFAULT_WINDOW_WIDTH,
                             GuiConstants::DEFAULT_WINDOW_HEIGHT, 
                             true);
    
    juce::Graphics g(fallbackImage);
    
    // Gradient pozadí (tmavě šedý)
    juce::ColourGradient gradient(
        juce::Colour(0xff2a2a2a), 0.0f, 0.0f,
        juce::Colour(0xff1a1a1a), 0.0f, static_cast<float>(fallbackImage.getHeight()),
        false
    );
    g.setGradientFill(gradient);
    g.fillAll();
    
    // Nápis uprostřed
    g.setColour(juce::Colours::white.withAlpha(0.3f));
    g.setFont(juce::FontOptions(24.0f, juce::Font::bold));
    g.drawText("Background image not found", 
               fallbackImage.getBounds(), 
               juce::Justification::centred);
    
    g.setFont(juce::FontOptions(14.0f));
    g.drawText("Place background.jpg in decorators/ folder", 
               fallbackImage.getBounds().reduced(20).removeFromBottom(100), 
               juce::Justification::centred);
    
    imageComponent.setImage(fallbackImage);
    imageComponent.setImagePlacement(juce::RectanglePlacement::stretchToFit);
    imageComponent.setInterceptsMouseClicks(false, false);
    addAndMakeVisible(imageComponent);
    
    GUI_DEBUG("IthacaGUI: Created fallback background");
}