/**
 * @file InfoPanelComponent.cpp
 * @brief Implementace komponenty pro info labely a live data updates
 */

#include "InfoPanelComponent.h"
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

// Definice konstant (zachovat původní hodnoty)
#ifndef CURRENT_INSTRUMENT
#define CURRENT_INSTRUMENT "Ithaca Grand Piano"
#endif

#ifndef PLUGIN_VERSION  
#define PLUGIN_VERSION "1.0.0"
#endif

// Static member definition
bool InfoPanelComponent::staticInfoSet_ = false;

InfoPanelComponent::InfoPanelComponent(IthacaPluginProcessor& processor)
    : processorRef_(processor), debugMode_(GuiHelpers::isDebugModeEnabled())
{
    GUI_DEBUG("InfoPanelComponent: Constructor starting");
    
    setupAllLabels();
    
    // ZACHOVAT: Původní timer logiku
    startUpdates();
    
    GUI_DEBUG("InfoPanelComponent: Constructor completed - " << getNumChildComponents() << " child components");
}

InfoPanelComponent::~InfoPanelComponent()
{
    stopUpdates();
    GUI_DEBUG("InfoPanelComponent: Destructor called");
}

void InfoPanelComponent::paint(juce::Graphics& g)
{
    // TRANSPARENTNÍ - bez pozadí, nechá hlavní editor vykreslit overlay
    juce::ignoreUnused(g);
    GUI_DEBUG("InfoPanelComponent: Paint method - transparent mode");
}

void InfoPanelComponent::resized()
{
    auto bounds = getLocalBounds();
    
    if (debugMode_) {
        layoutDebugMode(bounds);
    } else {
        layoutBackgroundMode(bounds);
    }
    
    GUI_DEBUG("InfoPanelComponent: Resized with bounds " << bounds.toString().toStdString());
}

void InfoPanelComponent::timerCallback()
{
    // ZACHOVAT: Přesně původní timer logiku včetně fallback bounds
    applyFallbackBounds();
    updateLiveData();
}

void InfoPanelComponent::startUpdates()
{
    if (!isTimerRunning()) {
        startTimer(TIMER_INTERVAL_MS);
        timerCallback(); // První update
        GUI_DEBUG("InfoPanelComponent: Timer updates started");
    }
}

void InfoPanelComponent::stopUpdates()
{
    if (isTimerRunning()) {
        stopTimer();
        GUI_DEBUG("InfoPanelComponent: Timer updates stopped");
    }
}

void InfoPanelComponent::setDebugMode(bool enabled)
{
    if (debugMode_ != enabled) {
        debugMode_ = enabled;
        
        // Přestylovat všechny komponenty
        setupAllLabels();
        resized();
        
        GUI_DEBUG("InfoPanelComponent: Debug mode " << (enabled ? "ENABLED" : "DISABLED"));
    }
}

void InfoPanelComponent::setupAllLabels()
{
    GUI_DEBUG("InfoPanelComponent: Setting up all labels - START");
    
    // Vymazat existující komponenty
    removeAllChildren();
    
    // ZACHOVAT: Všechny původní info labely s původními texty
    activeVoicesLabel = GuiHelpers::createSmallLabel("Active: 0", debugMode_);
    if (activeVoicesLabel) {
        addAndMakeVisible(activeVoicesLabel.get());
        GUI_DEBUG("InfoPanelComponent: activeVoicesLabel created");
    }
    
    sustainingVoicesLabel = GuiHelpers::createSmallLabel("Sustaining: 0", debugMode_);
    if (sustainingVoicesLabel) {
        addAndMakeVisible(sustainingVoicesLabel.get());
        GUI_DEBUG("InfoPanelComponent: sustainingVoicesLabel created");
    }
    
    releasingVoicesLabel = GuiHelpers::createSmallLabel("Releasing: 0", debugMode_);
    if (releasingVoicesLabel) {
        addAndMakeVisible(releasingVoicesLabel.get());
        GUI_DEBUG("InfoPanelComponent: releasingVoicesLabel created");
    }
    
    sampleRateLabel = GuiHelpers::createSmallLabel("Rate: 48000Hz", debugMode_);
    if (sampleRateLabel) {
        addAndMakeVisible(sampleRateLabel.get());
        GUI_DEBUG("InfoPanelComponent: sampleRateLabel created");
    }
    
    instrumentLabel = GuiHelpers::createSmallLabel("Instrument: " + juce::String(CURRENT_INSTRUMENT), debugMode_);
    if (instrumentLabel) {
        addAndMakeVisible(instrumentLabel.get());
        GUI_DEBUG("InfoPanelComponent: instrumentLabel created");
    }
    
    versionLabel = GuiHelpers::createSmallLabel("Version: " + juce::String(PLUGIN_VERSION), debugMode_);
    if (versionLabel) {
        addAndMakeVisible(versionLabel.get());
        GUI_DEBUG("InfoPanelComponent: versionLabel created");
    }
    
    GUI_DEBUG("InfoPanelComponent: All labels setup completed");
}

void InfoPanelComponent::updateLiveData()
{
    // ZACHOVAT: Přesně původní logiku aktualizace živých dat
    try {
        if (processorRef_.getVoiceManager()) {
            auto stats = processorRef_.getSamplerStats();
            
            // Voice counters - živé údaje (PŮVODNÍ FORMÁT)
            if (activeVoicesLabel) {
                GuiHelpers::updateLabelText(activeVoicesLabel.get(), 
                    juce::String(GuiConstants::TextConstants::ACTIVE_VOICES_PREFIX) + juce::String(stats.activeVoices));
            }
            
            if (sustainingVoicesLabel) {
                GuiHelpers::updateLabelText(sustainingVoicesLabel.get(), 
                    juce::String(GuiConstants::TextConstants::SUSTAINING_VOICES_PREFIX) + juce::String(stats.sustainingVoices));
            }
            
            if (releasingVoicesLabel) {
                GuiHelpers::updateLabelText(releasingVoicesLabel.get(), 
                    juce::String(GuiConstants::TextConstants::RELEASING_VOICES_PREFIX) + juce::String(stats.releasingVoices));
            }
            
            // Sample rate - živý údaj (PŮVODNÍ FORMÁT)
            if (sampleRateLabel) {
                GuiHelpers::updateLabelText(sampleRateLabel.get(), 
                    juce::String(GuiConstants::TextConstants::SAMPLE_RATE_PREFIX) + juce::String(stats.currentSampleRate) + "Hz");
            }
            
            // Statické informace - nastavit pouze jednou
            if (!staticInfoSet_) {
                if (instrumentLabel) {
                    GuiHelpers::updateLabelText(instrumentLabel.get(), 
                        juce::String(GuiConstants::TextConstants::INSTRUMENT_PREFIX) + juce::String(CURRENT_INSTRUMENT));
                }
                if (versionLabel) {
                    GuiHelpers::updateLabelText(versionLabel.get(), 
                        juce::String(GuiConstants::TextConstants::VERSION_PREFIX) + juce::String(PLUGIN_VERSION));
                }
                staticInfoSet_ = true;
                GUI_DEBUG("InfoPanelComponent: Static info labels set");
            }
            
        } else {
            // Engine není ready - zobrazit fallback hodnoty (PŮVODNÍ FORMÁT)
            if (activeVoicesLabel) {
                GuiHelpers::updateLabelText(activeVoicesLabel.get(), 
                    juce::String(GuiConstants::TextConstants::ACTIVE_VOICES_PREFIX) + GuiConstants::TextConstants::FALLBACK_VALUE);
            }
            if (sustainingVoicesLabel) {
                GuiHelpers::updateLabelText(sustainingVoicesLabel.get(), 
                    juce::String(GuiConstants::TextConstants::SUSTAINING_VOICES_PREFIX) + GuiConstants::TextConstants::FALLBACK_VALUE);
            }
            if (releasingVoicesLabel) {
                GuiHelpers::updateLabelText(releasingVoicesLabel.get(), 
                    juce::String(GuiConstants::TextConstants::RELEASING_VOICES_PREFIX) + GuiConstants::TextConstants::FALLBACK_VALUE);
            }
            if (sampleRateLabel) {
                GuiHelpers::updateLabelText(sampleRateLabel.get(), 
                    juce::String(GuiConstants::TextConstants::SAMPLE_RATE_PREFIX) + GuiConstants::TextConstants::FALLBACK_VALUE);
            }
        }
        
    } catch (...) {
        GUI_DEBUG("InfoPanelComponent: EXCEPTION in updateLiveData");
        
        // Fallback při chybě
        if (activeVoicesLabel) {
            GuiHelpers::updateLabelText(activeVoicesLabel.get(), 
                juce::String(GuiConstants::TextConstants::ACTIVE_VOICES_PREFIX) + GuiConstants::TextConstants::ERROR_VALUE);
        }
        if (sustainingVoicesLabel) {
            GuiHelpers::updateLabelText(sustainingVoicesLabel.get(), 
                juce::String(GuiConstants::TextConstants::SUSTAINING_VOICES_PREFIX) + GuiConstants::TextConstants::ERROR_VALUE);
        }
        if (releasingVoicesLabel) {
            GuiHelpers::updateLabelText(releasingVoicesLabel.get(), 
                juce::String(GuiConstants::TextConstants::RELEASING_VOICES_PREFIX) + GuiConstants::TextConstants::ERROR_VALUE);
        }
        if (sampleRateLabel) {
            GuiHelpers::updateLabelText(sampleRateLabel.get(), 
                juce::String(GuiConstants::TextConstants::SAMPLE_RATE_PREFIX) + GuiConstants::TextConstants::ERROR_VALUE);
        }
    }
}

void InfoPanelComponent::applyFallbackBounds()
{
    // ZACHOVAT: Přesně původní fallback bounds logiku
    static bool boundsFixed = false;
    if (!boundsFixed) {
        GUI_DEBUG("InfoPanelComponent: Fixing bounds via fallback mode");
        
        // Fallback bounds pro případ, že resized() nefunguje správně
        if (activeVoicesLabel && activeVoicesLabel->getBounds().isEmpty()) {
            activeVoicesLabel->setBounds(340, 80, 130, 18);
        }
        if (sustainingVoicesLabel && sustainingVoicesLabel->getBounds().isEmpty()) {
            sustainingVoicesLabel->setBounds(340, 102, 130, 18);
        }
        if (releasingVoicesLabel && releasingVoicesLabel->getBounds().isEmpty()) {
            releasingVoicesLabel->setBounds(340, 124, 130, 18);
        }
        if (sampleRateLabel && sampleRateLabel->getBounds().isEmpty()) {
            sampleRateLabel->setBounds(340, 150, 130, 18);
        }
        if (instrumentLabel && instrumentLabel->getBounds().isEmpty()) {
            instrumentLabel->setBounds(340, 172, 130, 18);
        }
        if (versionLabel && versionLabel->getBounds().isEmpty()) {
            versionLabel->setBounds(340, 600, 130, 18);
        }
        
        boundsFixed = true;
        GUI_DEBUG("InfoPanelComponent: All bounds fixed with fallback positions");
    }
}

void InfoPanelComponent::layoutBackgroundMode(juce::Rectangle<int> bounds)
{
    // ZACHOVAT: Původní layout pozice pro background mode
    auto infoArea = bounds;
    
    if (activeVoicesLabel) {
        activeVoicesLabel->setBounds(infoArea.removeFromTop(18));
        infoArea.removeFromTop(4);
    }
    if (sustainingVoicesLabel) {
        sustainingVoicesLabel->setBounds(infoArea.removeFromTop(18));
        infoArea.removeFromTop(4);
    }
    if (releasingVoicesLabel) {
        releasingVoicesLabel->setBounds(infoArea.removeFromTop(18));
        infoArea.removeFromTop(8);
    }
    if (sampleRateLabel) {
        sampleRateLabel->setBounds(infoArea.removeFromTop(18));
        infoArea.removeFromTop(4);
    }
    if (instrumentLabel) {
        instrumentLabel->setBounds(infoArea.removeFromTop(18));
        infoArea.removeFromTop(20);
    }
    
    // Version label speciální umístění - bude umístěn hlavním editorem
    
    GUI_DEBUG("InfoPanelComponent: Background mode layout completed");
}

void InfoPanelComponent::layoutDebugMode(juce::Rectangle<int> bounds)
{
    // DEBUG MODE: Původní layout pro debug
    auto controlArea = bounds.reduced(GuiConstants::COMPONENT_PADDING);
    
    controlArea.removeFromTop(30); // Top spacing
    
    if (activeVoicesLabel) {
        activeVoicesLabel->setBounds(controlArea.removeFromTop(18));
        controlArea.removeFromTop(4);
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
    
    // Version label - speciální umístění v debug módu
    if (versionLabel) {
        auto versionArea = bounds.removeFromBottom(25).reduced(8);
        versionLabel->setBounds(versionArea);
    }
    
    GUI_DEBUG("InfoPanelComponent: Debug mode layout completed");
}