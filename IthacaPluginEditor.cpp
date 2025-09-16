#include "IthacaPluginEditor.h"

//==============================================================================
IthacaPluginEditor::IthacaPluginEditor (IthacaPluginProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    // MINIMÁLNÍ konstruktor pro debugging hang problému
    
    // Základní velikost okna
    setSize(400, 300);
    
    // DOČASNĚ VYPNUTO - postupně přidávat komponenty:
    // setupParameterControls();
    // setupMonitoringComponents();
    // setupSampleManagement();
    
    // DOČASNĚ VYPNUTO - timer může způsobovat hang:
    // startTimer(33);
    
    // Pouze základní label pro test
    testLabel = std::make_unique<juce::Label>();
    testLabel->setText("IthacaCore Plugin - Engine Ready!", juce::dontSendNotification);
    testLabel->setBounds(10, 10, getWidth() - 20, 30);
    addAndMakeVisible(testLabel.get());
}

IthacaPluginEditor::~IthacaPluginEditor()
{
    // Zastavit timer pokud běží
    stopTimer();
}

//==============================================================================
void IthacaPluginEditor::paint (juce::Graphics& g)
{
    // Minimální paint pro test
    g.fillAll(juce::Colour(0xff2a2a2a));
    
    g.setColour(juce::Colours::white);
    g.setFont(16.0f);
    g.drawText("IthacaCore Sampler", 10, 50, getWidth() - 20, 30, 
               juce::Justification::centred);
    
    g.setColour(juce::Colours::lightgreen);
    g.setFont(12.0f);
    g.drawText("Audio Engine: 703 samples loaded", 10, 80, getWidth() - 20, 20, 
               juce::Justification::centred);
}

void IthacaPluginEditor::resized()
{
    // Minimální resize
    if (testLabel) {
        testLabel->setBounds(10, 10, getWidth() - 20, 30);
    }
}

// DOČASNĚ VYPNUTÉ metody pro postupné debugging:

void IthacaPluginEditor::timerCallback()
{
    // PRÁZDNÉ - timer vypnut
}

void IthacaPluginEditor::setupParameterControls()
{
    // PRÁZDNÉ - postupně přidat zpět
}

void IthacaPluginEditor::setupMonitoringComponents() 
{
    // PRÁZDNÉ - postupně přidat zpět
}

void IthacaPluginEditor::setupSampleManagement()
{
    // PRÁZDNÉ - postupně přidat zpět
}

void IthacaPluginEditor::browseSamplesButtonClicked()
{
    // Prázdná implementace
}

void IthacaPluginEditor::updateStatsDisplay()
{
    // Prázdná implementace
}

void IthacaPluginEditor::updateVoiceActivityDisplay()
{
    // Prázdná implementace
}

//==============================================================================
// VoiceActivityComponent - MINIMÁLNÍ implementace

IthacaPluginEditor::VoiceActivityComponent::VoiceActivityComponent()
{
    setSize(200, 100);
}

void IthacaPluginEditor::VoiceActivityComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff333333));
    g.setColour(juce::Colours::white);
    g.drawText("Voice Activity", getLocalBounds(), juce::Justification::centred);
}

void IthacaPluginEditor::VoiceActivityComponent::updateVoiceStates(int active, int sustaining, int releasing)
{
    // Prázdná implementace
    juce::ignoreUnused(active, sustaining, releasing);
}