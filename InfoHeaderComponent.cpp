/**
 * @file InfoHeaderComponent.cpp
 * @brief Implementace info header komponenty
 * 
 * ============================================================================
 * GUI REFACTORING - INFO HEADER IMPLEMENTATION
 * ============================================================================
 */

#include "InfoHeaderComponent.h"
#include "GuiConstants.h"
#include "BuildID.h"
#include <iostream>

#define BACKGROUND_PICTURE_OFF 0
#define CURRENT_INSTRUMENT "VintageV Electric Piano"


#if BACKGROUND_PICTURE_OFF
#define GUI_DEBUG(msg) std::cout << msg << std::endl
#else
#define GUI_DEBUG(msg)
#endif

// Static initialization
bool InfoHeaderComponent::staticInfoSet_ = false;

// ============================================================================
// Constructor / Destructor
// ============================================================================

InfoHeaderComponent::InfoHeaderComponent(IthacaPluginProcessor& processor)
    : processorRef_(processor), debugMode_(false)
{
    GUI_DEBUG("InfoHeaderComponent: Constructor starting");
    
    setupAllLabels();
    
    GUI_DEBUG("InfoHeaderComponent: Constructor completed");
}

InfoHeaderComponent::~InfoHeaderComponent()
{
    stopTimer();
    GUI_DEBUG("InfoHeaderComponent: Destructor - timer stopped");
}

// ============================================================================
// Component Overrides
// ============================================================================

void InfoHeaderComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    
    if (!debugMode_) {
        // Zaoblené overlay pozadí (80% alpha, 6px radius)
        GuiHelpers::drawRoundedOverlay(g, bounds, 
                                      GuiConstants::INFO_OVERLAY_ALPHA,
                                      GuiConstants::PANEL_CORNER_RADIUS);
    }
    // Debug mode: průhledné pozadí
}

void InfoHeaderComponent::resized()
{
    auto bounds = getLocalBounds().reduced(GuiConstants::SECTION_PADDING);
    
    if (debugMode_) {
        layoutDebugMode(bounds);
    } else {
        layoutBackgroundMode(bounds);
    }
}

// ============================================================================
// Timer Override
// ============================================================================

void InfoHeaderComponent::timerCallback()
{
    updateLiveData();
}

// ============================================================================
// Public Control Methods
// ============================================================================

void InfoHeaderComponent::startUpdates()
{
    if (!isTimerRunning()) {
        startTimer(TIMER_INTERVAL_MS);
        GUI_DEBUG("InfoHeaderComponent: Timer started (" << TIMER_INTERVAL_MS << "ms)");
    }
}

void InfoHeaderComponent::stopUpdates()
{
    if (isTimerRunning()) {
        stopTimer();
        GUI_DEBUG("InfoHeaderComponent: Timer stopped");
    }
}

void InfoHeaderComponent::setDebugMode(bool enabled)
{
    debugMode_ = enabled;
    setupAllLabels();
    resized();
    GUI_DEBUG("InfoHeaderComponent: Debug mode " << (enabled ? "ENABLED" : "DISABLED"));
}

// ============================================================================
// Private Methods - Setup
// ============================================================================

void InfoHeaderComponent::setupAllLabels()
{
    GUI_DEBUG("InfoHeaderComponent: Setting up labels");
    
    removeAllChildren();
    
    // Instrument name - VELKÝ FONT (18px, bold)
    instrumentNameLabel = GuiHelpers::createTitleLabel(CURRENT_INSTRUMENT, debugMode_);
    if (instrumentNameLabel) {
        addAndMakeVisible(instrumentNameLabel.get());
    }
    
    // Version - STŘEDNÍ FONT (14px)
    versionLabel = GuiHelpers::createInfoLabel(
        "Version: " + juce::String(PLUGIN_VERSION), debugMode_);
    if (versionLabel) {
        addAndMakeVisible(versionLabel.get());
    }
    
    // Sample rate - MALÝ FONT (11px)
    sampleRateLabel = GuiHelpers::createSmallLabel("Sample Rate: 48000 Hz", debugMode_);
    if (sampleRateLabel) {
        addAndMakeVisible(sampleRateLabel.get());
    }
    
    // Active voices - MALÝ FONT (11px)
    activeVoicesLabel = GuiHelpers::createSmallLabel("Active: 0", debugMode_);
    if (activeVoicesLabel) {
        addAndMakeVisible(activeVoicesLabel.get());
    }
    
    // Sustaining voices - MALÝ FONT (11px)
    sustainingVoicesLabel = GuiHelpers::createSmallLabel("Sustaining: 0", debugMode_);
    if (sustainingVoicesLabel) {
        addAndMakeVisible(sustainingVoicesLabel.get());
    }
    
    GUI_DEBUG("InfoHeaderComponent: All labels created");
}

// ============================================================================
// Private Methods - Data Update
// ============================================================================

void InfoHeaderComponent::updateLiveData()
{
    auto* vm = processorRef_.getVoiceManager();
    
    // ========================================================================
    // CRITICAL: Check async loading status first
    // ========================================================================
    
    if (processorRef_.isLoadingInProgress()) {
        // Currently loading
        if (instrumentNameLabel) {
            instrumentNameLabel->setText(GuiConstants::TextConstants::LOADING_TEXT, 
                                        juce::dontSendNotification);
        }
        
        if (activeVoicesLabel) {
            activeVoicesLabel->setText("Active: --", juce::dontSendNotification);
        }
        if (sustainingVoicesLabel) {
            sustainingVoicesLabel->setText("Sustaining: --", juce::dontSendNotification);
        }
        
        return;
    }
    
    if (processorRef_.hasLoadingError()) {
        // Loading failed
        if (instrumentNameLabel) {
            instrumentNameLabel->setText(GuiConstants::TextConstants::ERROR_TEXT, 
                                        juce::dontSendNotification);
        }
        return;
    }
    
    // ========================================================================
    // Loading completed - show normal info
    // ========================================================================
    
    // Update instrument name (once)
    if (instrumentNameLabel && !staticInfoSet_) {
        instrumentNameLabel->setText(CURRENT_INSTRUMENT, juce::dontSendNotification);
        staticInfoSet_ = true;
    }
    
    // ========================================================================
    // Update live statistics
    // ========================================================================
    
    if (vm) {
        auto stats = processorRef_.getSamplerStats();
        
        // Active voices
        if (activeVoicesLabel) {
            activeVoicesLabel->setText(
                juce::String(GuiConstants::TextConstants::ACTIVE_VOICES_PREFIX) + 
                juce::String(stats.activeVoices),
                juce::dontSendNotification);
        }
        
        // Sustaining voices
        if (sustainingVoicesLabel) {
            sustainingVoicesLabel->setText(
                juce::String(GuiConstants::TextConstants::SUSTAINING_VOICES_PREFIX) + 
                juce::String(stats.sustainingVoices),
                juce::dontSendNotification);
        }
        
        // Sample rate (if not set)
        if (sampleRateLabel && stats.currentSampleRate > 0) {
            sampleRateLabel->setText(
                juce::String(GuiConstants::TextConstants::SAMPLE_RATE_PREFIX) + 
                juce::String(stats.currentSampleRate) + " Hz",
                juce::dontSendNotification);
        }
        
    } else {
        // No VoiceManager yet
        if (activeVoicesLabel) {
            activeVoicesLabel->setText("Active: --", juce::dontSendNotification);
        }
        if (sustainingVoicesLabel) {
            sustainingVoicesLabel->setText("Sustaining: --", juce::dontSendNotification);
        }
    }
}

// ============================================================================
// Private Methods - Layout
// ============================================================================

void InfoHeaderComponent::layoutBackgroundMode(juce::Rectangle<int> bounds)
{
    // Hierarchický layout:
    // Row 1: Instrument name (velký font)
    // Row 2: Version (střední font)
    // Row 3: Sample rate (malý font)
    // Row 4: Active | Sustaining (malý font, 50/50)
    
    if (instrumentNameLabel) {
        instrumentNameLabel->setBounds(
            bounds.removeFromTop(GuiConstants::INFO_TITLE_HEIGHT));
        bounds.removeFromTop(GuiConstants::INFO_ROW_SPACING);
    }
    
    if (versionLabel) {
        versionLabel->setBounds(
            bounds.removeFromTop(GuiConstants::INFO_VERSION_HEIGHT));
        bounds.removeFromTop(GuiConstants::INFO_ROW_SPACING);
    }
    
    if (sampleRateLabel) {
        sampleRateLabel->setBounds(
            bounds.removeFromTop(GuiConstants::INFO_SAMPLE_RATE_HEIGHT));
        bounds.removeFromTop(GuiConstants::INFO_ROW_SPACING);
    }
    
    // Voice stats row - 50/50 split
    auto voiceStatsRow = bounds.removeFromTop(GuiConstants::INFO_VOICE_STATS_HEIGHT);
    
    if (activeVoicesLabel && sustainingVoicesLabel) {
        int halfWidth = voiceStatsRow.getWidth() / 2;
        activeVoicesLabel->setBounds(voiceStatsRow.removeFromLeft(halfWidth));
        sustainingVoicesLabel->setBounds(voiceStatsRow);
    }
    
    GUI_DEBUG("InfoHeaderComponent: Background mode layout applied");
}

void InfoHeaderComponent::layoutDebugMode(juce::Rectangle<int> bounds)
{
    // Debug mode: kompaktní vertikální layout
    const int labelHeight = 18;
    const int spacing = 2;
    
    if (instrumentNameLabel) {
        instrumentNameLabel->setBounds(bounds.removeFromTop(labelHeight));
        bounds.removeFromTop(spacing);
    }
    
    if (versionLabel) {
        versionLabel->setBounds(bounds.removeFromTop(labelHeight));
        bounds.removeFromTop(spacing);
    }
    
    if (sampleRateLabel) {
        sampleRateLabel->setBounds(bounds.removeFromTop(labelHeight));
        bounds.removeFromTop(spacing);
    }
    
    // Voice stats - 50/50
    auto voiceRow = bounds.removeFromTop(labelHeight);
    int halfWidth = voiceRow.getWidth() / 2;
    
    if (activeVoicesLabel) {
        activeVoicesLabel->setBounds(voiceRow.removeFromLeft(halfWidth));
    }
    if (sustainingVoicesLabel) {
        sustainingVoicesLabel->setBounds(voiceRow);
    }
    
    GUI_DEBUG("InfoHeaderComponent: Debug mode layout applied");
}