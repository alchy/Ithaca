/**
 * @file InfoPanelComponent.cpp (Refactored with Loading Status)
 * @brief Info panel with async loading status display
 */

#include "InfoPanelComponent.h"
#include "GuiConstants.h"
#include <iostream>

// Debug macro (matches IthacaPluginEditor)
#define BACKGROUND_PICTURE_OFF 0
#define CURRENT_INSTRUMENT "Ithaca Grand Piano"
#define PLUGIN_VERSION "1.0.0"

#if BACKGROUND_PICTURE_OFF
#define GUI_DEBUG(msg) std::cout << msg << std::endl
#else
#define GUI_DEBUG(msg)
#endif

// Static initialization
bool InfoPanelComponent::staticInfoSet_ = false;

//==============================================================================
// Constructor / Destructor

InfoPanelComponent::InfoPanelComponent(IthacaPluginProcessor& processor)
    : processorRef_(processor), debugMode_(false)
{
    GUI_DEBUG("InfoPanelComponent: Constructor starting");
    
    // Setup all labels
    setupAllLabels();
    
    // Don't start timer yet - wait for parentHierarchyChanged
    GUI_DEBUG("InfoPanelComponent: Constructor completed");
}

InfoPanelComponent::~InfoPanelComponent()
{
    stopTimer();
    GUI_DEBUG("InfoPanelComponent: Destructor - timer stopped");
}

//==============================================================================
// Component Overrides

void InfoPanelComponent::paint(juce::Graphics& g)
{
    // Transparent - no background painting
    juce::ignoreUnused(g);
}

void InfoPanelComponent::resized()
{
    auto bounds = getLocalBounds();
    
    // Different layouts based on debug mode
    if (debugMode_) {
        layoutDebugMode(bounds);
    } else {
        layoutBackgroundMode(bounds);
    }
}

//==============================================================================
// Timer Override

void InfoPanelComponent::timerCallback()
{
    // Update live data from processor
    updateLiveData();
}

//==============================================================================
// Public Control Methods

void InfoPanelComponent::startUpdates()
{
    if (!isTimerRunning()) {
        startTimer(TIMER_INTERVAL_MS);
        GUI_DEBUG("InfoPanelComponent: Timer started (" << TIMER_INTERVAL_MS << "ms interval)");
    }
}

void InfoPanelComponent::stopUpdates()
{
    if (isTimerRunning()) {
        stopTimer();
        GUI_DEBUG("InfoPanelComponent: Timer stopped");
    }
}

void InfoPanelComponent::setDebugMode(bool enabled)
{
    debugMode_ = enabled;
    
    // Recreate labels with new debug mode
    setupAllLabels();
    resized();
    
    GUI_DEBUG("InfoPanelComponent: Debug mode " << (enabled ? "ENABLED" : "DISABLED"));
}

//==============================================================================
// Private Methods - Setup

void InfoPanelComponent::setupAllLabels()
{
    GUI_DEBUG("InfoPanelComponent: Setting up all labels - START");
    
    // Remove existing components
    removeAllChildren();
    
    // Create all info labels with current debug mode
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
    
    // IMPORTANT: Instrument label now shows loading status
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

//==============================================================================
// Private Methods - Data Update

void InfoPanelComponent::updateLiveData()
{
    auto* vm = processorRef_.getVoiceManager();
    
    // ========================================================================
    // CRITICAL: Check async loading status first
    // ========================================================================
    
    if (processorRef_.isLoadingInProgress()) {
        // Currently loading - show loading status
        if (instrumentLabel) {
            instrumentLabel->setText("Loading samples...", juce::dontSendNotification);
        }
        
        // Keep other stats at zero during loading
        if (activeVoicesLabel) {
            activeVoicesLabel->setText("Active: --", juce::dontSendNotification);
        }
        if (sustainingVoicesLabel) {
            sustainingVoicesLabel->setText("Sustaining: --", juce::dontSendNotification);
        }
        if (releasingVoicesLabel) {
            releasingVoicesLabel->setText("Releasing: --", juce::dontSendNotification);
        }
        
        GUI_DEBUG("InfoPanelComponent: Showing loading status");
        return;
    }
    
    if (processorRef_.hasLoadingError()) {
        // Loading failed - show error
        if (instrumentLabel) {
            instrumentLabel->setText("Sample load error", juce::dontSendNotification);
        }
        
        GUI_DEBUG("InfoPanelComponent: Showing error status");
        return;
    }
    
    // ========================================================================
    // Loading completed - show normal info
    // ========================================================================
    
    // Update instrument label to show instrument name
    if (instrumentLabel && !staticInfoSet_) {
        instrumentLabel->setText("Instrument: " + juce::String(CURRENT_INSTRUMENT), 
                                juce::dontSendNotification);
        staticInfoSet_ = true;
        GUI_DEBUG("InfoPanelComponent: Static info set - instrument name displayed");
    }
    
    // ========================================================================
    // Update live voice statistics
    // ========================================================================
    
    if (vm) {
        // Get stats from processor
        auto stats = processorRef_.getSamplerStats();
        
        // Update active voices
        if (activeVoicesLabel) {
            activeVoicesLabel->setText(
                juce::String(GuiConstants::TextConstants::ACTIVE_VOICES_PREFIX) + juce::String(stats.activeVoices),
                juce::dontSendNotification
            );
        }
        
        // Update sustaining voices
        if (sustainingVoicesLabel) {
            sustainingVoicesLabel->setText(
                juce::String(GuiConstants::TextConstants::SUSTAINING_VOICES_PREFIX) + juce::String(stats.sustainingVoices),
                juce::dontSendNotification
            );
        }
        
        // Update releasing voices
        if (releasingVoicesLabel) {
            releasingVoicesLabel->setText(
                juce::String(GuiConstants::TextConstants::RELEASING_VOICES_PREFIX) + juce::String(stats.releasingVoices),
                juce::dontSendNotification
            );
        }
        
        // Update sample rate (if not already set)
        if (sampleRateLabel && stats.currentSampleRate > 0) {
            sampleRateLabel->setText(
                juce::String(GuiConstants::TextConstants::SAMPLE_RATE_PREFIX) + 
                juce::String(stats.currentSampleRate) + "Hz",
                juce::dontSendNotification
            );
        }
        
    } else {
        // No VoiceManager yet - show fallback values
        if (activeVoicesLabel) {
            activeVoicesLabel->setText(
                juce::String(GuiConstants::TextConstants::ACTIVE_VOICES_PREFIX) + 
                GuiConstants::TextConstants::FALLBACK_VALUE,
                juce::dontSendNotification
            );
        }
        
        if (sustainingVoicesLabel) {
            sustainingVoicesLabel->setText(
                juce::String(GuiConstants::TextConstants::SUSTAINING_VOICES_PREFIX) + 
                GuiConstants::TextConstants::FALLBACK_VALUE,
                juce::dontSendNotification
            );
        }
        
        if (releasingVoicesLabel) {
            releasingVoicesLabel->setText(
                juce::String(GuiConstants::TextConstants::RELEASING_VOICES_PREFIX) + 
                GuiConstants::TextConstants::FALLBACK_VALUE,
                juce::dontSendNotification
            );
        }
    }
}

//==============================================================================
// Private Methods - Layout

void InfoPanelComponent::layoutBackgroundMode(juce::Rectangle<int> bounds)
{
    // BACKGROUND MODE: Original vertical stacking layout
    // (This matches the original IthacaPluginEditor layout)
    
    const int labelHeight = 20;
    const int spacing = 4;
    
    if (activeVoicesLabel) {
        activeVoicesLabel->setBounds(bounds.removeFromTop(labelHeight));
        bounds.removeFromTop(spacing);
    }
    
    if (sustainingVoicesLabel) {
        sustainingVoicesLabel->setBounds(bounds.removeFromTop(labelHeight));
        bounds.removeFromTop(spacing);
    }
    
    if (releasingVoicesLabel) {
        releasingVoicesLabel->setBounds(bounds.removeFromTop(labelHeight));
        bounds.removeFromTop(spacing);
    }
    
    if (sampleRateLabel) {
        sampleRateLabel->setBounds(bounds.removeFromTop(labelHeight));
        bounds.removeFromTop(spacing);
    }
    
    // Instrument label (shows loading status or instrument name)
    if (instrumentLabel) {
        instrumentLabel->setBounds(bounds.removeFromTop(labelHeight));
        bounds.removeFromTop(spacing);
    }
    
    // Version label at bottom
    if (versionLabel) {
        versionLabel->setBounds(bounds.removeFromTop(labelHeight));
    }
    
    GUI_DEBUG("InfoPanelComponent: Background mode layout applied");
}

void InfoPanelComponent::layoutDebugMode(juce::Rectangle<int> bounds)
{
    // DEBUG MODE: Compact vertical stacking
    
    const int labelHeight = 18;
    const int spacing = 2;
    
    if (activeVoicesLabel) {
        activeVoicesLabel->setBounds(bounds.removeFromTop(labelHeight));
        bounds.removeFromTop(spacing);
    }
    
    if (sustainingVoicesLabel) {
        sustainingVoicesLabel->setBounds(bounds.removeFromTop(labelHeight));
        bounds.removeFromTop(spacing);
    }
    
    if (releasingVoicesLabel) {
        releasingVoicesLabel->setBounds(bounds.removeFromTop(labelHeight));
        bounds.removeFromTop(spacing);
    }
    
    if (sampleRateLabel) {
        sampleRateLabel->setBounds(bounds.removeFromTop(labelHeight));
        bounds.removeFromTop(spacing);
    }
    
    if (instrumentLabel) {
        instrumentLabel->setBounds(bounds.removeFromTop(labelHeight));
        bounds.removeFromTop(spacing);
    }
    
    if (versionLabel) {
        versionLabel->setBounds(bounds.removeFromTop(labelHeight));
    }
    
    GUI_DEBUG("InfoPanelComponent: Debug mode layout applied");
}

void InfoPanelComponent::applyFallbackBounds()
{
    // Fallback layout if resized() wasn't called properly
    // (Original implementation preserved)
    
    const int x = 10;
    int y = 10;
    const int width = getWidth() - 20;
    const int height = 20;
    const int spacing = 25;
    
    if (activeVoicesLabel) {
        activeVoicesLabel->setBounds(x, y, width, height);
        y += spacing;
    }
    
    if (sustainingVoicesLabel) {
        sustainingVoicesLabel->setBounds(x, y, width, height);
        y += spacing;
    }
    
    if (releasingVoicesLabel) {
        releasingVoicesLabel->setBounds(x, y, width, height);
        y += spacing;
    }
    
    if (sampleRateLabel) {
        sampleRateLabel->setBounds(x, y, width, height);
        y += spacing;
    }
    
    if (instrumentLabel) {
        instrumentLabel->setBounds(x, y, width, height);
        y += spacing;
    }
    
    if (versionLabel) {
        versionLabel->setBounds(x, y, width, height);
    }
    
    GUI_DEBUG("InfoPanelComponent: Fallback bounds applied");
}