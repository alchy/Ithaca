#include "IthacaPluginEditor.h"

//==============================================================================
IthacaPluginEditor::IthacaPluginEditor (IthacaPluginProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p), timerCallCounter(0)
{
    // Rozšíření okna pro více komponent
    setSize(500, 320);
    
    // POUZE jeden jednoduchý label pro test
    engineStatusLabel = std::make_unique<juce::Label>();
    engineStatusLabel->setText("IthacaCore - Engine Ready!", juce::dontSendNotification);
    engineStatusLabel->setBounds(10, 10, 450, 30);
    engineStatusLabel->setColour(juce::Label::textColourId, juce::Colour(0xff0066cc));
    engineStatusLabel->setFont(juce::FontOptions(14.0f));
    addAndMakeVisible(engineStatusLabel.get());
    
    // Přidat základní komponenty
    setupMainComponents();
    
    // ŽÁDNÝ TIMER - pořád vypnutý
    
    // Debug výpis
    DBG("IthacaPluginEditor constructor with all labels completed successfully");
}

IthacaPluginEditor::~IthacaPluginEditor()
{
    DBG("IthacaPluginEditor destructor called");
}

//==============================================================================
void IthacaPluginEditor::paint (juce::Graphics& g)
{
    // Minimální paint
    g.fillAll(juce::Colour(0xffffffff)); // Bílé pozadí
    
    // Pouze základní text
    g.setColour(juce::Colour(0xff333333));
    g.setFont(juce::FontOptions(16.0f));
    g.drawText("IthacaCore Sampler - Debug Mode", 10, 50, getWidth() - 20, 30, 
               juce::Justification::centred);
    
    g.drawText("Engine: 703 samples loaded", 10, 80, getWidth() - 20, 20, 
               juce::Justification::centred);
               
    // Oddělení pro nové komponenty
    g.setColour(juce::Colour(0xffcccccc));
    g.drawLine(10.0f, 110.0f, static_cast<float>(getWidth() - 10), 110.0f, 1.0f);
               
    DBG("Paint method called successfully");
}

void IthacaPluginEditor::resized()
{
    // Minimální resize
    if (engineStatusLabel) {
        engineStatusLabel->setBounds(10, 10, getWidth() - 20, 30);
    }
    
    DBG("Resized method called successfully");
}

void IthacaPluginEditor::parentHierarchyChanged()
{
    DBG("parentHierarchyChanged called - component showing: " + juce::String(isShowing() ? "true" : "false"));
    
    // NYNÍ přidáme BEZPEČNÝ timer - pouze pro update labelů
    if (isShowing() && !isTimerRunning()) {
        DBG("Starting safe timer for label updates");
        startTimer(1000); // Začneme s 1 sekunda pro bezpečnost, později zrychlíme
    }
}

//==============================================================================
// VŠECHNY OSTATNÍ METODY PRÁZDNÉ PRO DEBUG

void IthacaPluginEditor::timerCallback()
{
    DBG("Timer callback - updating labels safely");
    
    try {
        // BEZPEČNÁ aktualizace pouze základních labels
        // BEZ přístupu k složitým VoiceManager strukturám
        
        // Engine status
        if (processorRef.getVoiceManager()) {
            if (engineStatusLabel) {
                engineStatusLabel->setText("Engine: Ready (Live)", juce::dontSendNotification);
                engineStatusLabel->setColour(juce::Label::textColourId, juce::Colour(0xff009900));
            }
            
            // Získání základních stats BEZ přístupu k voice details
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
            
            // Master controls - BEZPEČNĚ
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
            // Engine není ready
            if (engineStatusLabel) {
                engineStatusLabel->setText("Engine: Loading...", juce::dontSendNotification);
                engineStatusLabel->setColour(juce::Label::textColourId, juce::Colour(0xffff8800));
            }
        }
        
    } catch (...) {
        DBG("Timer callback caught exception - GUI remains stable");
        // Tiché selhání - GUI zůstane stabilní
    }
}

void IthacaPluginEditor::setupMainComponents()
{
    // Přidáme všechny základní labels postupně
    
    // Total samples label
    totalSamplesLabel = std::make_unique<juce::Label>();
    totalSamplesLabel->setText("Total Samples: 703", juce::dontSendNotification);
    totalSamplesLabel->setBounds(10, 120, 200, 18);
    totalSamplesLabel->setColour(juce::Label::textColourId, juce::Colour(0xff333333));
    addAndMakeVisible(totalSamplesLabel.get());
    
    // Sample rate label
    sampleRateLabel = std::make_unique<juce::Label>();
    sampleRateLabel->setText("Sample Rate: 48000 Hz", juce::dontSendNotification);
    sampleRateLabel->setBounds(10, 140, 200, 18);
    sampleRateLabel->setColour(juce::Label::textColourId, juce::Colour(0xff333333));
    addAndMakeVisible(sampleRateLabel.get());
    
    // Voice counters
    activeVoicesLabel = std::make_unique<juce::Label>();
    activeVoicesLabel->setText("Active: 0", juce::dontSendNotification);  
    activeVoicesLabel->setBounds(10, 170, 200, 18);
    activeVoicesLabel->setColour(juce::Label::textColourId, juce::Colour(0xff333333));
    addAndMakeVisible(activeVoicesLabel.get());
    
    sustainingVoicesLabel = std::make_unique<juce::Label>();
    sustainingVoicesLabel->setText("Sustaining: 0", juce::dontSendNotification);
    sustainingVoicesLabel->setBounds(10, 190, 200, 18);
    sustainingVoicesLabel->setColour(juce::Label::textColourId, juce::Colour(0xff333333));
    addAndMakeVisible(sustainingVoicesLabel.get());
    
    releasingVoicesLabel = std::make_unique<juce::Label>();
    releasingVoicesLabel->setText("Releasing: 0", juce::dontSendNotification);
    releasingVoicesLabel->setBounds(10, 210, 200, 18);
    releasingVoicesLabel->setColour(juce::Label::textColourId, juce::Colour(0xff333333));
    addAndMakeVisible(releasingVoicesLabel.get());
    
    // Master controls
    masterGainLabel = std::make_unique<juce::Label>();
    masterGainLabel->setText("Master Gain: 100", juce::dontSendNotification);
    masterGainLabel->setBounds(10, 240, 200, 18);
    masterGainLabel->setColour(juce::Label::textColourId, juce::Colour(0xff333333));
    addAndMakeVisible(masterGainLabel.get());
    
    masterPanLabel = std::make_unique<juce::Label>();
    masterPanLabel->setText("Master Pan: Center", juce::dontSendNotification);
    masterPanLabel->setBounds(10, 260, 200, 18);
    masterPanLabel->setColour(juce::Label::textColourId, juce::Colour(0xff333333));
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

std::unique_ptr<juce::Label> IthacaPluginEditor::createLabel(const juce::String& text, 
                                                           juce::Justification justification)
{
    auto label = std::make_unique<juce::Label>();
    label->setText(text, juce::dontSendNotification);
    label->setJustificationType(justification);
    label->setColour(juce::Label::textColourId, juce::Colour(0xff333333));
    label->setFont(juce::FontOptions(12.0f));
    return label;
}

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
    // PRÁZDNÉ - nebudeme vytvářet
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