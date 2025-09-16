/**
 * @file IthacaPluginEditor.cpp
 * @brief Implementace GUI editoru pro IthacaPlugin s obrázkem v pozadí a funkčními overlays.
 */

#include "IthacaPluginEditor.h"
#include "decorators/BinaryData.h"
#include <iostream>

// DEBUG: Přepínač pro vypnutí background obrázku
#define BACKGROUND_PICTURE_OFF 0  // Nastav na 1 pro vypnutí obrázku, 0 pro zapnutí

// Definice konstant (chyběly tyto definice)
#ifndef CURRENT_INSTRUMENT
#define CURRENT_INSTRUMENT "IthacaCore"
#endif

#ifndef PLUGIN_VERSION  
#define PLUGIN_VERSION "1.0.0"
#endif

//==============================================================================
/**
 * @brief Konstruktor editoru s obrázkem na pozadí a funkčními overlays.
 */
IthacaPluginEditor::IthacaPluginEditor (IthacaPluginProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p), timerCallCounter(0)
{
    std::cout << "IthacaGUI: Constructor starting with IMPROVED LAYOUT" << std::endl;
    
    // Nastavení velikosti okna - širší pro dvousloupcový layout
    setSize(400, 600);
    
#if !BACKGROUND_PICTURE_OFF
    // Embeddovaný obrázek z BinaryData (načtení a nastavení)
    juce::Image image = juce::ImageCache::getFromMemory(BinaryData::ithacaplayer1_jpg, BinaryData::ithacaplayer1_jpgSize);
    imageComponent.setImage(image);
    imageComponent.setImagePlacement(juce::RectanglePlacement::stretchToFit);
    imageComponent.setInterceptsMouseClicks(false, false); // Neblokuje interakce s overlay komponentami
    addAndMakeVisible(imageComponent);
    std::cout << "IthacaGUI: Background image loaded and set" << std::endl;
#else
    std::cout << "IthacaGUI: Background image DISABLED (debug mode)" << std::endl;
#endif
    
    // Přidat základní komponenty (labels budou overlay přes obrázek nebo na bílém pozadí)
    setupMainComponents();
    
    std::cout << "IthacaGUI: Constructor completed successfully" << std::endl;
}

/**
 * @brief Destruktor editoru.
 */
IthacaPluginEditor::~IthacaPluginEditor()
{
    stopTimer();
    std::cout << "IthacaGUI: Destructor called" << std::endl;
}

//==============================================================================
/**
 * @brief Maluje pozadí podle debug režimu.
 */
void IthacaPluginEditor::paint (juce::Graphics& g)
{
#if BACKGROUND_PICTURE_OFF
    // DEBUG MODE: Čisté šedé pozadí
    g.fillAll(juce::Colour(0xff808080));
    
    // Pouze základní title 
    g.setColour(juce::Colour(0xff333333));
    g.setFont(juce::FontOptions(18.0f));
    g.drawText("IthacaCore Sampler - Debug Mode", 10, 10, getWidth() - 20, 30, 
               juce::Justification::centred);
               
    // Oddělení
    g.setColour(juce::Colour(0xffcccccc));
    g.drawLine(10.0f, 45.0f, static_cast<float>(getWidth() - 10), 45.0f, 1.0f);
    
    std::cout << "IthacaGUI: Paint method - DEBUG MODE (clean version)" << std::endl;
#else
    // BACKGROUND MODE: Pouze overlay pro čitelnost
    
    // Přidat tlumený overlay jen v horní části pro lepší čitelnost controlů
    auto controlArea = getLocalBounds().removeFromTop(280);
    g.setColour(juce::Colours::black.withAlpha(0.5f));
    g.fillRect(controlArea);
    
    // Debug border pro vizualizaci oblasti
    g.setColour(juce::Colours::yellow.withAlpha(0.3f));
    g.drawRect(controlArea, 2);
    
    std::cout << "IthacaGUI: Paint method - BACKGROUND MODE (with image)" << std::endl;
#endif
}

/**
 * @brief Resized - minimální layout v pravém sloupci pro lepší využití obrázku.
 */
void IthacaPluginEditor::resized()
{
    auto bounds = getLocalBounds();
    
#if !BACKGROUND_PICTURE_OFF
    // BACKGROUND MODE: Obrázek zabírá celou plochu
    imageComponent.setBounds(bounds);
    
    // Malá kontrolní oblast v pravém sloupci (kde je méně grafiky)
    auto controlArea = bounds.removeFromRight(150).removeFromTop(450).reduced(8);
#else
    // DEBUG MODE: Kompaktní layout vpravo
    auto controlArea = bounds.removeFromRight(150).removeFromTop(450).reduced(8, 20);
#endif
    
    // Všechny komponenty v pravém sloupci s malým fontem
    controlArea.removeFromTop(30); // Prostor od horního okraje
    
    if (activeVoicesLabel) {
        activeVoicesLabel->setBounds(controlArea.removeFromTop(18));
        controlArea.removeFromTop(4);
        std::cout << "IthacaGUI: activeVoicesLabel bounds set to " << activeVoicesLabel->getBounds().toString() << std::endl;
    }
    
    if (sustainingVoicesLabel) {
        sustainingVoicesLabel->setBounds(controlArea.removeFromTop(18));
        controlArea.removeFromTop(4);
    }
    
    if (releasingVoicesLabel) {
        releasingVoicesLabel->setBounds(controlArea.removeFromTop(18));
        controlArea.removeFromTop(8);
    }
    
    if (sampleRateLabel) {
        sampleRateLabel->setBounds(controlArea.removeFromTop(18));
        controlArea.removeFromTop(4);
    }
    
    if (instrumentLabel) {
        instrumentLabel->setBounds(controlArea.removeFromTop(18));
        controlArea.removeFromTop(20);
    }
    
    // Version úplně dole
    if (versionLabel) {
        auto versionArea = bounds.removeFromBottom(25).removeFromRight(150).reduced(8);
        versionLabel->setBounds(versionArea);
    }
    
    std::cout << "IthacaGUI: Resized method completed - MINIMAL RIGHT COLUMN layout" << std::endl;
    std::cout << "IthacaGUI: Total visible child components: " << getNumChildComponents() << std::endl;
}

/**
 * @brief Callback pro změnu hierarchie rodiče - spustí timer bezpečně.
 * PLUS: Alternative timer start v resized() pokud parentHierarchy nefunguje.
 */
void IthacaPluginEditor::parentHierarchyChanged()
{
    std::cout << "IthacaGUI: parentHierarchyChanged - showing=" << (isShowing() ? "true" : "false") 
              << " timerRunning=" << (isTimerRunning() ? "true" : "false") << std::endl;
    
    if (isShowing() && !isTimerRunning()) {
        std::cout << "IthacaGUI: Starting timer via parentHierarchyChanged" << std::endl;
        startTimer(300);
        timerCallback(); // Force první update
    }
}

//==============================================================================
/**
 * @brief Timer callback pro aktualizaci labels s live daty.
 */
void IthacaPluginEditor::timerCallback()
{
    // FIRST: Ensure all labels have proper bounds (MINIMAL LAYOUT VERSION)
    static bool boundsFixed = false;
    if (!boundsFixed) {
        std::cout << "IthacaGUI: Fixing bounds via timer callback - MINIMAL RIGHT COLUMN" << std::endl;
        
        // PRAVÝ SLOUPEC: Všechny komponenty v pravém sloupci (x=250, width=140)
        if (activeVoicesLabel && activeVoicesLabel->getBounds().isEmpty()) {
            activeVoicesLabel->setBounds(250, 60, 140, 18);
        }
        if (sustainingVoicesLabel && sustainingVoicesLabel->getBounds().isEmpty()) {
            sustainingVoicesLabel->setBounds(250, 82, 140, 18);
        }
        if (releasingVoicesLabel && releasingVoicesLabel->getBounds().isEmpty()) {
            releasingVoicesLabel->setBounds(250, 104, 140, 18);
        }
        if (sampleRateLabel && sampleRateLabel->getBounds().isEmpty()) {
            sampleRateLabel->setBounds(250, 130, 140, 18);
        }
        if (instrumentLabel && instrumentLabel->getBounds().isEmpty()) {
            instrumentLabel->setBounds(250, 152, 140, 18);
        }
        
        // Version úplně dole
        if (versionLabel && versionLabel->getBounds().isEmpty()) {
            versionLabel->setBounds(250, 570, 140, 18);
        }
        
        boundsFixed = true;
        std::cout << "IthacaGUI: All bounds fixed with MINIMAL LAYOUT - right column only" << std::endl;
    }
    
    // THEN: Update only live data (not static)
    try {
        if (processorRef.getVoiceManager()) {
            auto stats = processorRef.getSamplerStats();
            
            // Voice counters - živé údaje
            if (activeVoicesLabel) {
                activeVoicesLabel->setText("Active: " + juce::String(stats.activeVoices), juce::dontSendNotification);
            }
            
            if (sustainingVoicesLabel) {
                sustainingVoicesLabel->setText("Sustaining: " + juce::String(stats.sustainingVoices), juce::dontSendNotification);
            }
            
            if (releasingVoicesLabel) {
                releasingVoicesLabel->setText("Releasing: " + juce::String(stats.releasingVoices), juce::dontSendNotification);
            }
            
            // Sample rate - živý údaj
            if (sampleRateLabel) {
                sampleRateLabel->setText("Rate: " + juce::String(stats.currentSampleRate) + "Hz", juce::dontSendNotification);
            }
            
            // Instrument a Version jsou statické - nenastavujeme v timeru
            
        } else {
            // Engine není ready - zobrazit fallback hodnoty
            if (activeVoicesLabel) activeVoicesLabel->setText("Active: --", juce::dontSendNotification);
            if (sustainingVoicesLabel) sustainingVoicesLabel->setText("Sustaining: --", juce::dontSendNotification);
            if (releasingVoicesLabel) releasingVoicesLabel->setText("Releasing: --", juce::dontSendNotification);
            if (sampleRateLabel) sampleRateLabel->setText("Rate: --", juce::dontSendNotification);
        }
        
    } catch (...) {
        std::cout << "IthacaGUI: EXCEPTION in timer callback" << std::endl;
    }
}

/**
 * @brief Vytvoří label s malým fontem pro minimální layout.
 */
std::unique_ptr<juce::Label> IthacaPluginEditor::createSmallLabel(const juce::String& text, 
                                                                juce::Justification justification)
{
    auto label = std::make_unique<juce::Label>();
    label->setText(text, juce::dontSendNotification);
    label->setJustificationType(justification);
    
#if BACKGROUND_PICTURE_OFF
    // DEBUG MODE: Černý text na bílém pozadí
    label->setColour(juce::Label::textColourId, juce::Colour(0xff000000));
    label->setColour(juce::Label::backgroundColourId, juce::Colour(0xffffffff));
    label->setColour(juce::Label::outlineColourId, juce::Colour(0xff333333));
    std::cout << "IthacaGUI: Small label created with DEBUG styling" << std::endl;
#else
    // BACKGROUND MODE: Minimální styling pro malé labels
    label->setColour(juce::Label::textColourId, juce::Colours::white);
    label->setColour(juce::Label::backgroundColourId, juce::Colours::black.withAlpha(0.75f));
    label->setColour(juce::Label::outlineColourId, juce::Colours::white.withAlpha(0.4f));
    std::cout << "IthacaGUI: Small label created with MINIMAL BACKGROUND styling" << std::endl;
#endif
    
    // Malý font pro kompaktní layout
    label->setFont(juce::FontOptions(11.0f));
    
    return label;
}

/**
 * @brief Nastaví pouze potřebné komponenty s malým fontem pro pravý sloupec.
 */
void IthacaPluginEditor::setupMainComponents()
{
    std::cout << "IthacaGUI: setupMainComponents - creating MINIMAL components..." << std::endl;
    
    // Voice counters (malý font)
    activeVoicesLabel = createSmallLabel("Active: 0");
    addAndMakeVisible(activeVoicesLabel.get());
    std::cout << "IthacaGUI: activeVoicesLabel created and added" << std::endl;
    
    sustainingVoicesLabel = createSmallLabel("Sustaining: 0");
    addAndMakeVisible(sustainingVoicesLabel.get());
    std::cout << "IthacaGUI: sustainingVoicesLabel created and added" << std::endl;
    
    releasingVoicesLabel = createSmallLabel("Releasing: 0");
    addAndMakeVisible(releasingVoicesLabel.get());
    std::cout << "IthacaGUI: releasingVoicesLabel created and added" << std::endl;
    
    // Sample rate (malý font)
    sampleRateLabel = createSmallLabel("Sample Rate: 48000 Hz");
    addAndMakeVisible(sampleRateLabel.get());
    std::cout << "IthacaGUI: sampleRateLabel created and added" << std::endl;
    
    // Instrument info (malý font) 
    instrumentLabel = createSmallLabel("Instrument: " + juce::String(CURRENT_INSTRUMENT));
    addAndMakeVisible(instrumentLabel.get());
    std::cout << "IthacaGUI: instrumentLabel created and added" << std::endl;
    
    // Version info (malý font, úplně dole)
    versionLabel = createSmallLabel("Version: " + juce::String(PLUGIN_VERSION));
    addAndMakeVisible(versionLabel.get());
    std::cout << "IthacaGUI: versionLabel created and added" << std::endl;
    
    std::cout << "IthacaGUI: All MINIMAL components setup completed - " << getNumChildComponents() << " child components" << std::endl;
}

// Prázdné metody pro kompatibilitu s header
void IthacaPluginEditor::setupLabels() { /* prázdné */ }
void IthacaPluginEditor::setupVoiceActivityGrid() { /* prázdné */ }
void IthacaPluginEditor::updateStatsDisplay() { /* prázdné */ }
void IthacaPluginEditor::updateVoiceActivityDisplay() { /* prázdné */ }
void IthacaPluginEditor::updateMasterControlsDisplay() { /* prázdné */ }

/**
 * @brief Vytvoří label s vylepšeným stylem podle debug režimu.
 */
std::unique_ptr<juce::Label> IthacaPluginEditor::createLabel(const juce::String& text, 
                                                           juce::Justification justification)
{
    auto label = std::make_unique<juce::Label>();
    label->setText(text, juce::dontSendNotification);
    label->setJustificationType(justification);
    
#if BACKGROUND_PICTURE_OFF
    // DEBUG MODE: Černý text na bílém pozadí
    label->setColour(juce::Label::textColourId, juce::Colour(0xff000000));
    label->setColour(juce::Label::backgroundColourId, juce::Colour(0xffffffff));
    label->setColour(juce::Label::outlineColourId, juce::Colour(0xff333333));
    std::cout << "IthacaGUI: Label created with DEBUG styling (black on white)" << std::endl;
#else
    // BACKGROUND MODE: Vylepšený kontrast pro lepší čitelnost
    label->setColour(juce::Label::textColourId, juce::Colours::white);
    label->setColour(juce::Label::backgroundColourId, juce::Colours::black.withAlpha(0.85f)); // Silnější pozadí
    label->setColour(juce::Label::outlineColourId, juce::Colours::white.withAlpha(0.6f)); // Výraznější okraj
    std::cout << "IthacaGUI: Label created with IMPROVED BACKGROUND styling" << std::endl;
#endif
    
    // Větší font pro lepší čitelnost
    label->setFont(juce::FontOptions(14.0f)); // Zvětšeno z 12.0f
    
    return label;
}

/**
 * @brief Aktualizuje text labelu, pokud se liší.
 */
void IthacaPluginEditor::updateLabelText(juce::Label* label, const juce::String& newText)
{
    if (label && label->getText() != newText) {
        label->setText(newText, juce::dontSendNotification);
    }
}

//==============================================================================
// VoiceActivityComponent - prázdné implementace pro kompatibilitu s header

IthacaPluginEditor::VoiceActivityComponent::VoiceActivityComponent()
{
    voiceStates.fill(0);
}

void IthacaPluginEditor::VoiceActivityComponent::paint(juce::Graphics& g)
{
    juce::ignoreUnused(g);
}

void IthacaPluginEditor::VoiceActivityComponent::resized()
{
    // prázdné
}

void IthacaPluginEditor::VoiceActivityComponent::updateVoiceStates(int active, int sustaining, int releasing)
{
    juce::ignoreUnused(active, sustaining, releasing);
}

void IthacaPluginEditor::VoiceActivityComponent::setVoiceState(uint8_t midiNote, bool isActive, int voiceState)
{
    juce::ignoreUnused(midiNote, isActive, voiceState);
}

juce::Rectangle<int> IthacaPluginEditor::VoiceActivityComponent::getCellBounds(int row, int col) const
{
    juce::ignoreUnused(row, col);
    return juce::Rectangle<int>();
}

juce::Colour IthacaPluginEditor::VoiceActivityComponent::getStateColour(int state) const
{
    juce::ignoreUnused(state);
    return juce::Colour(0xff000000);
}

int IthacaPluginEditor::VoiceActivityComponent::getMidiNoteFromGrid(int row, int col) const
{
    juce::ignoreUnused(row, col);
    return 0;
}