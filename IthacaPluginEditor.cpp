/**
 * @file IthacaPluginEditor.cpp
 * @brief Implementace GUI editoru pro IthacaPlugin s obrázekem v pozadí.
 */

#include "IthacaPluginEditor.h"
#include "decorators/BinaryData.h"

//==============================================================================
/**
 * @brief Konstruktor editoru.
 * Inicializuje komponenty GUI: načte obrázek jako pozadí, přidá labely bez fixních bounds.
 * @param p Reference na procesor.
 */
IthacaPluginEditor::IthacaPluginEditor (IthacaPluginProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p), timerCallCounter(0)
{
    // Nastavení velikosti okna
    setSize(400, 600);
    
    // Embedovaný obrázek z BinaryData (načtení a nastavení)
    juce::Image image = juce::ImageCache::getFromMemory(BinaryData::ithacaplayer1_jpg, BinaryData::ithacaplayer1_jpgSize);
    imageComponent.setImage(image);
    imageComponent.setImagePlacement(juce::RectanglePlacement::stretchToFit);       // Roztáhni na celou plochu
    imageComponent.setInterceptsMouseClicks(false, false);                          // Neblokuje interakce s overlay komponentami
    addAndMakeVisible(imageComponent);
    
    // Přidat základní komponenty (bez bounds, ty se nastaví v resized())
    setupMainComponents();
    
    // Debug výpis
    DBG("IthacaPluginEditor constructor with all labels completed successfully");
}

/**
 * @brief Destruktor editoru.
 * Automaticky uvolní komponenty.
 */
IthacaPluginEditor::~IthacaPluginEditor()
{
    DBG("IthacaPluginEditor destructor called");
}

//==============================================================================
/**
 * @brief Maluje pozadí GUI.
 * @param g Grafický kontext.
 */
void IthacaPluginEditor::paint (juce::Graphics& g)
{
    // Žádné plné pozadí - obrázek je podklad
    // Přidat tlumený overlay pro čitelnost controlů
    g.setColour(juce::Colours::black.withAlpha(0.1f));
    g.fillRect(getLocalBounds().removeFromTop(300)); // Overlay na horní část pro labely
    
    // Žádné přímé kreslení textu - necháme na labelech
    
    DBG("Paint method called successfully");
}

/**
 * @brief Resized - nastavuje dynamické pozice komponent v GUI.
 */
void IthacaPluginEditor::resized()
{
    auto bounds = getLocalBounds();
    
    // Obrázek zabírá celou plochu
    imageComponent.setBounds(bounds);
    
    // Controls jsou overlay přes obrázek
    auto controlArea = bounds.removeFromTop(300).reduced(10);
    
    if (engineStatusLabel) engineStatusLabel->setBounds(controlArea.removeFromTop(30));
    if (totalSamplesLabel) totalSamplesLabel->setBounds(controlArea.removeFromTop(18));
    if (sampleRateLabel) sampleRateLabel->setBounds(controlArea.removeFromTop(18));
    if (activeVoicesLabel) activeVoicesLabel->setBounds(controlArea.removeFromTop(18));
    if (sustainingVoicesLabel) sustainingVoicesLabel->setBounds(controlArea.removeFromTop(18));
    if (releasingVoicesLabel) releasingVoicesLabel->setBounds(controlArea.removeFromTop(18));
    if (masterGainLabel) masterGainLabel->setBounds(controlArea.removeFromTop(18));
    if (masterPanLabel) masterPanLabel->setBounds(controlArea.removeFromTop(18));
    
    DBG("Resized method called successfully");
}

/**
 * @brief Callback pro změnu hierarchie rodiče.
 * Spustí timer bezpečně, pokud je komponenta viditelná.
 */
void IthacaPluginEditor::parentHierarchyChanged()
{
    DBG("parentHierarchyChanged called - component showing: " + juce::String(isShowing() ? "true" : "false"));
    
    if (isShowing() && !isTimerRunning()) {
        DBG("Starting safe timer for label updates");
        startTimer(1000); // Začneme s 1 sekunda pro bezpečnost, později zrychlíme
    }
}

//==============================================================================
/**
 * @brief Timer callback pro aktualizaci labelů.
 */
void IthacaPluginEditor::timerCallback()
{
    DBG("Timer callback - updating labels safely");
    
    try {
        if (processorRef.getVoiceManager()) {
            if (engineStatusLabel) {
                engineStatusLabel->setText("Engine: Ready (Live)", juce::dontSendNotification);
                engineStatusLabel->setColour(juce::Label::textColourId, juce::Colours::white);
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
                engineStatusLabel->setColour(juce::Label::textColourId, juce::Colours::white);
            }
        }
        
    } catch (...) {
        DBG("Timer callback caught exception - GUI remains stable");
    }
}

/**
 * @brief Nastaví hlavní komponenty (labely) bez bounds.
 */
void IthacaPluginEditor::setupMainComponents()
{
    // Status display
    engineStatusLabel = createLabel("IthacaCore - Engine Ready!");
    addAndMakeVisible(engineStatusLabel.get());
    
    // Total samples label
    totalSamplesLabel = createLabel("Total Samples: 703");
    addAndMakeVisible(totalSamplesLabel.get());
    
    // Sample rate label
    sampleRateLabel = createLabel("Sample Rate: 48000 Hz");
    addAndMakeVisible(sampleRateLabel.get());
    
    // Voice counters
    activeVoicesLabel = createLabel("Active: 0");
    addAndMakeVisible(activeVoicesLabel.get());
    
    sustainingVoicesLabel = createLabel("Sustaining: 0");
    addAndMakeVisible(sustainingVoicesLabel.get());
    
    releasingVoicesLabel = createLabel("Releasing: 0");
    addAndMakeVisible(releasingVoicesLabel.get());
    
    // Master controls
    masterGainLabel = createLabel("Master Gain: 100");
    addAndMakeVisible(masterGainLabel.get());
    
    masterPanLabel = createLabel("Master Pan: Center");
    addAndMakeVisible(masterPanLabel.get());
    
    DBG("All basic components setup completed");
}

void IthacaPluginEditor::setupLabels()
{
    // PRÁZDNÉ
}

void IthacaPluginEditor::setupVoiceActivityGrid()
{
    // PRÁZDNÉ
}

void IthacaPluginEditor::updateStatsDisplay()
{
    // PRÁZDNÉ
}

void IthacaPluginEditor::updateVoiceActivityDisplay()
{
    // PRÁZDNÉ
}

void IthacaPluginEditor::updateMasterControlsDisplay()
{
    // PRÁZDNÉ
}

/**
 * @brief Vytvoří label s defaultními barvami pro overlay.
 * @param text Text labelu.
 * @param justification Zarovnání textu.
 * @return Ukazatel na label.
 */
std::unique_ptr<juce::Label> IthacaPluginEditor::createLabel(const juce::String& text, 
                                                           juce::Justification justification)
{
    auto label = std::make_unique<juce::Label>();
    label->setText(text, juce::dontSendNotification);
    label->setJustificationType(justification);
    label->setColour(juce::Label::textColourId, juce::Colours::white);                          // Bílý text pro čitelnost
    label->setColour(juce::Label::backgroundColourId, juce::Colours::black.withAlpha(0.8f));    // Vyšší alpha pro viditelnost
    label->setFont(juce::FontOptions(12.0f));
    return label;
}

/**
 * @brief Aktualizuje text labelu, pokud se liší.
 * @param label Ukazatel na label.
 * @param newText Nový text.
 */
void IthacaPluginEditor::updateLabelText(juce::Label* label, const juce::String& newText)
{
    if (label && label->getText() != newText) {
        label->setText(newText, juce::dontSendNotification);
    }
}

//==============================================================================
// VoiceActivityComponent - PRÁZDNÁ implementace pro debug

IthacaPluginEditor::VoiceActivityComponent::VoiceActivityComponent()
{
    // PRÁZDNÉ
}

void IthacaPluginEditor::VoiceActivityComponent::paint(juce::Graphics& g)
{
    // PRÁZDNÉ
}

void IthacaPluginEditor::VoiceActivityComponent::resized()
{
    // PRÁZDNÉ
}

void IthacaPluginEditor::VoiceActivityComponent::updateVoiceStates(int active, int sustaining, int releasing)
{
    // PRÁZDNÉ
    juce::ignoreUnused(active, sustaining, releasing);
}

void IthacaPluginEditor::VoiceActivityComponent::setVoiceState(uint8_t midiNote, bool isActive, int voiceState)
{
    // PRÁZDNÉ
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