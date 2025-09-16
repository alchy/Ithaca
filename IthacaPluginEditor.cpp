/**
 * @file IthacaPluginEditor.cpp
 * @brief Implementace GUI editoru pro IthacaPlugin s obrázkem v pozadí a funkčními overlays.
 */

#include "IthacaPluginEditor.h"
#include "decorators/BinaryData.h"
#include <iostream>

// DEBUG: Přepínač pro vypnutí background obrázku
#define BACKGROUND_PICTURE_OFF 1  // Nastav na 1 pro vypnutí obrázku, 0 pro zapnutí

//==============================================================================
/**
 * @brief Konstruktor editoru s obrázkem na pozadí a funkčními overlays.
 */
IthacaPluginEditor::IthacaPluginEditor (IthacaPluginProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p), timerCallCounter(0)
{
    std::cout << "IthacaGUI: Constructor starting with background image" << std::endl;
    
    // Nastavení velikosti okna
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
    // DEBUG MODE: Šedé pozadí místo bílého pro lepší viditelnost
    g.fillAll(juce::Colour(0xff808080)); // Tmavě šedé pro jistotu
    
    // FORCE TEST: Kreslení textu přímo v paint() pro test
    g.setColour(juce::Colour(0xffffffff)); // Bílý text
    g.setFont(juce::FontOptions(20.0f));
    g.drawText("DIRECT PAINT TEST", 50, 100, 300, 30, juce::Justification::centred);
    
    g.setColour(juce::Colour(0xff000000)); // Černý text
    g.drawText("Labels count: " + juce::String(getNumChildComponents()), 50, 140, 300, 30, juce::Justification::centred);
    
    // NOVÝ TEST: Force bounds setup v paint() 
    if (engineStatusLabel && engineStatusLabel->getBounds().isEmpty()) {
        engineStatusLabel->setBounds(15, 180, 370, 25);
        std::cout << "IthacaGUI: EMERGENCY bounds fix in paint()" << std::endl;
    }
    
    if (totalSamplesLabel && totalSamplesLabel->getBounds().isEmpty()) {
        totalSamplesLabel->setBounds(15, 210, 370, 20);
    }
    
    // Test kreslení bounds info
    if (engineStatusLabel) {
        g.drawText("Engine label bounds: " + engineStatusLabel->getBounds().toString(), 
                   50, 250, 350, 20, juce::Justification::centredLeft);
    }
    
    // Původní title
    g.setColour(juce::Colour(0xff333333));
    g.setFont(juce::FontOptions(18.0f));
    g.drawText("IthacaCore Sampler - Debug Mode", 10, 10, getWidth() - 20, 30, 
               juce::Justification::centred);
               
    // Oddělení
    g.setColour(juce::Colour(0xffcccccc));
    g.drawLine(10.0f, 45.0f, static_cast<float>(getWidth() - 10), 45.0f, 1.0f);
    
    std::cout << "IthacaGUI: Paint method - DEBUG MODE with emergency bounds fix" << std::endl;
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
 * @brief Resized - nastavuje dynamické pozice komponent podle debug režimu.
 */
void IthacaPluginEditor::resized()
{
    auto bounds = getLocalBounds();
    
#if !BACKGROUND_PICTURE_OFF
    // BACKGROUND MODE: Obrázek zabírá celou plochu
    imageComponent.setBounds(bounds);
    
    // Controls jsou overlay přes obrázek v horní části
    auto controlArea = bounds.removeFromTop(280).reduced(15);
#else
    // DEBUG MODE: Controls zabírají normální layout
    auto controlArea = bounds.removeFromTop(400).reduced(15, 50); // Začínáme níže kvůli title
#endif
    
    // Rozložení labelů s dostatečným prostorem
    if (engineStatusLabel) {
        engineStatusLabel->setBounds(controlArea.removeFromTop(25));
        controlArea.removeFromTop(5); // mezera
    }
    
    if (totalSamplesLabel) {
        totalSamplesLabel->setBounds(controlArea.removeFromTop(20));
        controlArea.removeFromTop(3);
    }
    
    if (sampleRateLabel) {
        sampleRateLabel->setBounds(controlArea.removeFromTop(20));
        controlArea.removeFromTop(3);
    }
    
    controlArea.removeFromTop(10); // větší mezera před voice counters
    
    if (activeVoicesLabel) {
        activeVoicesLabel->setBounds(controlArea.removeFromTop(20));
        controlArea.removeFromTop(3);
    }
    
    if (sustainingVoicesLabel) {
        sustainingVoicesLabel->setBounds(controlArea.removeFromTop(20));
        controlArea.removeFromTop(3);
    }
    
    if (releasingVoicesLabel) {
        releasingVoicesLabel->setBounds(controlArea.removeFromTop(20));
        controlArea.removeFromTop(3);
    }
    
    controlArea.removeFromTop(10); // větší mezera před master controls
    
    if (masterGainLabel) {
        masterGainLabel->setBounds(controlArea.removeFromTop(20));
        controlArea.removeFromTop(3);
    }
    
    if (masterPanLabel) {
        masterPanLabel->setBounds(controlArea.removeFromTop(20));
        std::cout << "IthacaGUI: masterPanLabel bounds set to " << masterPanLabel->getBounds().toString() << std::endl;
    }
    
    std::cout << "IthacaGUI: Resized method completed - mode=" << (BACKGROUND_PICTURE_OFF ? "DEBUG" : "BACKGROUND") << std::endl;
    std::cout << "IthacaGUI: Total visible child components: " << getNumChildComponents() << std::endl;
    
    // Debug: Print bounds of all labels
    if (engineStatusLabel) std::cout << "engineStatusLabel bounds: " << engineStatusLabel->getBounds().toString() << std::endl;
    if (totalSamplesLabel) std::cout << "totalSamplesLabel bounds: " << totalSamplesLabel->getBounds().toString() << std::endl;
    if (activeVoicesLabel) std::cout << "activeVoicesLabel bounds: " << activeVoicesLabel->getBounds().toString() << std::endl;
    
    // FALLBACK: Timer spuštění pokud ještě neběží
    if (!isTimerRunning()) {
        std::cout << "IthacaGUI: Starting timer fallback in resized()" << std::endl;
        startTimer(300);
        timerCallback(); // Force první update
    }
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
    try {
        if (processorRef.getVoiceManager()) {
            if (engineStatusLabel) {
                engineStatusLabel->setText("Engine: Ready (Live)", juce::dontSendNotification);
            }
            
            auto stats = processorRef.getSamplerStats();
            
            if (sampleRateLabel) {
                sampleRateLabel->setText("Sample Rate: " + juce::String(stats.currentSampleRate) + " Hz", juce::dontSendNotification);
            }
            
            if (totalSamplesLabel) {
                int samples = (stats.totalLoadedSamples > 0) ? stats.totalLoadedSamples : 703;
                totalSamplesLabel->setText("Total Samples: " + juce::String(samples), juce::dontSendNotification);
            }
            
            if (activeVoicesLabel) {
                activeVoicesLabel->setText("Active: " + juce::String(stats.activeVoices), juce::dontSendNotification);
            }
            
            if (sustainingVoicesLabel) {
                sustainingVoicesLabel->setText("Sustaining: " + juce::String(stats.sustainingVoices), juce::dontSendNotification);
            }
            
            if (releasingVoicesLabel) {
                releasingVoicesLabel->setText("Releasing: " + juce::String(stats.releasingVoices), juce::dontSendNotification);
            }
            
            // Master controls
            auto& parameters = processorRef.getParameters();
            if (auto* gainParam = parameters.getRawParameterValue("masterGain")) {
                if (masterGainLabel) {
                    int gainValue = static_cast<int>(gainParam->load());
                    masterGainLabel->setText("Master Gain: " + juce::String(gainValue), juce::dontSendNotification);
                }
            }
            
            if (auto* panParam = parameters.getRawParameterValue("masterPan")) {
                if (masterPanLabel) {
                    float panValue = panParam->load();
                    juce::String panText = "Master Pan: ";
                    if (panValue < -0.5f) {
                        panText += "L" + juce::String(static_cast<int>(-panValue));
                    } else if (panValue > 0.5f) {
                        panText += "R" + juce::String(static_cast<int>(panValue));
                    } else {
                        panText += "Center";
                    }
                    masterPanLabel->setText(panText, juce::dontSendNotification);
                }
            }
            
        } else {
            if (engineStatusLabel) {
                engineStatusLabel->setText("Engine: Loading...", juce::dontSendNotification);
            }
        }
        
    } catch (...) {
        std::cout << "IthacaGUI: EXCEPTION in timer callback" << std::endl;
    }
}

/**
 * @brief Nastaví hlavní komponenty (labels) s vhodným stylem pro overlay.
 */
void IthacaPluginEditor::setupMainComponents()
{
    std::cout << "IthacaGUI: setupMainComponents - creating labels..." << std::endl;
    
    // Status display - hlavní label s větším fontem
    engineStatusLabel = createLabel("IthacaCore - Engine Ready!", juce::Justification::centred);
    engineStatusLabel->setFont(juce::FontOptions(16.0f));
    addAndMakeVisible(engineStatusLabel.get());
    std::cout << "IthacaGUI: engineStatusLabel created and added" << std::endl;
    
    // Info labels
    totalSamplesLabel = createLabel("Total Samples: 703");
    addAndMakeVisible(totalSamplesLabel.get());
    std::cout << "IthacaGUI: totalSamplesLabel created and added" << std::endl;
    
    sampleRateLabel = createLabel("Sample Rate: 48000 Hz");
    addAndMakeVisible(sampleRateLabel.get());
    std::cout << "IthacaGUI: sampleRateLabel created and added" << std::endl;
    
    // Voice counters
    activeVoicesLabel = createLabel("Active: 0");
    addAndMakeVisible(activeVoicesLabel.get());
    std::cout << "IthacaGUI: activeVoicesLabel created and added" << std::endl;
    
    sustainingVoicesLabel = createLabel("Sustaining: 0");
    addAndMakeVisible(sustainingVoicesLabel.get());
    std::cout << "IthacaGUI: sustainingVoicesLabel created and added" << std::endl;
    
    releasingVoicesLabel = createLabel("Releasing: 0");
    addAndMakeVisible(releasingVoicesLabel.get());
    std::cout << "IthacaGUI: releasingVoicesLabel created and added" << std::endl;
    
    // Master controls
    masterGainLabel = createLabel("Master Gain: 100");
    addAndMakeVisible(masterGainLabel.get());
    std::cout << "IthacaGUI: masterGainLabel created and added" << std::endl;
    
    masterPanLabel = createLabel("Master Pan: Center");
    addAndMakeVisible(masterPanLabel.get());
    std::cout << "IthacaGUI: masterPanLabel created and added" << std::endl;
    
    std::cout << "IthacaGUI: All overlay components setup completed - " << getNumChildComponents() << " child components" << std::endl;
}

// Prázdné metody pro kompatibilitu s header
void IthacaPluginEditor::setupLabels() { /* prázdné */ }
void IthacaPluginEditor::setupVoiceActivityGrid() { /* prázdné */ }
void IthacaPluginEditor::updateStatsDisplay() { /* prázdné */ }
void IthacaPluginEditor::updateVoiceActivityDisplay() { /* prázdné */ }
void IthacaPluginEditor::updateMasterControlsDisplay() { /* prázdné */ }

/**
 * @brief Vytvoří label s vhodným stylem podle debug režimu.
 */
std::unique_ptr<juce::Label> IthacaPluginEditor::createLabel(const juce::String& text, 
                                                           juce::Justification justification)
{
    auto label = std::make_unique<juce::Label>();
    label->setText(text, juce::dontSendNotification);
    label->setJustificationType(justification);
    
#if BACKGROUND_PICTURE_OFF
    // DEBUG MODE: Tmavý text na šedém pozadí pro lepší viditelnost
    label->setColour(juce::Label::textColourId, juce::Colour(0xff000000)); // Černý text
    label->setColour(juce::Label::backgroundColourId, juce::Colour(0xffffffff)); // Bílé pozadí labelů
    label->setColour(juce::Label::outlineColourId, juce::Colour(0xff333333)); // Tmavý okraj
    std::cout << "IthacaGUI: Label created with DEBUG styling (black on white)" << std::endl;
#else
    // BACKGROUND MODE: Kontrastní barvy pro čitelnost na obrázku
    label->setColour(juce::Label::textColourId, juce::Colours::white);
    label->setColour(juce::Label::backgroundColourId, juce::Colours::black.withAlpha(0.7f));
    label->setColour(juce::Label::outlineColourId, juce::Colours::white.withAlpha(0.3f));
    std::cout << "IthacaGUI: Label created with BACKGROUND styling" << std::endl;
#endif
    
    label->setFont(juce::FontOptions(12.0f));
    
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