/**
 * @file InfoHeaderComponent.cpp
 * @brief Implementace info header komponenty
 * 
 * ============================================================================
 * GUI REFACTORING - INFO HEADER IMPLEMENTATION
 * ============================================================================
 */

#include "ithaca/gui/components/InfoHeaderComponent.h"
#include "ithaca/config/AppConstants.h"
#include "BuildID.h"
#include <iostream>

#define BACKGROUND_PICTURE_OFF 0

#if BACKGROUND_PICTURE_OFF
#define GUI_DEBUG(msg) std::cout << msg << std::endl
#else
#define GUI_DEBUG(msg)
#endif

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
                                      Constants::Gui::Overlay::INFO_ALPHA,
                                      Constants::Gui::Overlay::CORNER_RADIUS);
    }
    // Debug mode: průhledné pozadí
}

void InfoHeaderComponent::resized()
{
    auto bounds = getLocalBounds().reduced(Constants::Gui::Layout::SECTION_PADDING);

    auto layoutMode = debugMode_ ? InfoHeaderLayout::LayoutMode::Debug
                                  : InfoHeaderLayout::LayoutMode::Background;

    InfoHeaderLayout::applyLayout(bounds, getLabelsForLayout(), layoutMode);
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

    // Create all labels using factory
    labelBundle_ = InfoHeaderLabelFactory::createAllLabels(processorRef_, debugMode_);

    // Add to component
    InfoHeaderLabelFactory::addToComponent(*this, labelBundle_);

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
        if (labelBundle_.instrumentNameLabel) {
            labelBundle_.instrumentNameLabel->setText(Constants::Gui::Text::LOADING_TEXT,
                                        juce::dontSendNotification);
        }

        if (labelBundle_.activeVoicesLabel) {
            labelBundle_.activeVoicesLabel->setText("Active: --", juce::dontSendNotification);
        }
        if (labelBundle_.sustainingVoicesLabel) {
            labelBundle_.sustainingVoicesLabel->setText("Sustaining: --", juce::dontSendNotification);
        }

        return;
    }

    if (processorRef_.hasLoadingError()) {
        // Loading failed
        if (labelBundle_.instrumentNameLabel) {
            labelBundle_.instrumentNameLabel->setText(Constants::Gui::Text::ERROR_TEXT,
                                        juce::dontSendNotification);
        }
        return;
    }
    
    // ========================================================================
    // Loading completed - show normal info
    // ========================================================================

    // Update instrument name (always restore after loading)
    if (labelBundle_.instrumentNameLabel) {
        // FIXED: Vždy obnovit název nástroje po dokončení loadingu
        // (resetuje "Loading samples..." zpět na název nástroje z JSON)
        auto currentText = labelBundle_.instrumentNameLabel->getText();
        if (currentText == Constants::Gui::Text::LOADING_TEXT ||
            currentText == Constants::Gui::Text::ERROR_TEXT ||
            currentText.isEmpty()) {
            auto instrumentName = processorRef_.getInstrumentName();
            if (!instrumentName.isEmpty()) {
                labelBundle_.instrumentNameLabel->setText(instrumentName, juce::dontSendNotification);
                GUI_DEBUG("InfoHeaderComponent: Restored instrument name after loading");
            }
        }
    }
    
    // ========================================================================
    // Update live statistics
    // ========================================================================
    
    if (vm) {
        auto stats = processorRef_.getSamplerStats();
        
        // Active voices
        if (labelBundle_.activeVoicesLabel) {
            labelBundle_.activeVoicesLabel->setText(
                juce::String(Constants::Gui::Text::ACTIVE_VOICES_PREFIX) +
                juce::String(stats.activeVoices),
                juce::dontSendNotification);
        }

        // Sustaining voices
        if (labelBundle_.sustainingVoicesLabel) {
            labelBundle_.sustainingVoicesLabel->setText(
                juce::String(Constants::Gui::Text::SUSTAINING_VOICES_PREFIX) +
                juce::String(stats.sustainingVoices),
                juce::dontSendNotification);
        }

        // Sample rate (if not set)
        if (labelBundle_.sampleRateLabel && stats.currentSampleRate > 0) {
            labelBundle_.sampleRateLabel->setText(
                juce::String(Constants::Gui::Text::SAMPLE_RATE_PREFIX) +
                juce::String(stats.currentSampleRate) + " Hz",
                juce::dontSendNotification);
        }

        // CPU Usage with color-coded indication
        if (labelBundle_.cpuUsageLabel) {
            juce::String cpuText = "CPU: " +
                juce::String(stats.cpuUsagePercent, 1) + "% | Dropouts: " +
                juce::String(stats.dropoutCount);

            labelBundle_.cpuUsageLabel->setText(cpuText, juce::dontSendNotification);

            // Color-coded CPU status
            juce::Colour cpuColor;
            if (stats.dropoutCount > 0 || stats.cpuUsagePercent > 80.0) {
                cpuColor = juce::Colours::red;        // Critical (RED)
            } else if (stats.cpuUsagePercent > 50.0) {
                cpuColor = juce::Colours::orange;     // Warning (ORANGE)
            } else {
                cpuColor = juce::Colours::lightgreen; // OK (GREEN)
            }

            labelBundle_.cpuUsageLabel->setColour(juce::Label::textColourId, cpuColor);
        }

    } else {
        // No VoiceManager yet
        if (labelBundle_.activeVoicesLabel) {
            labelBundle_.activeVoicesLabel->setText("Active: --", juce::dontSendNotification);
        }
        if (labelBundle_.sustainingVoicesLabel) {
            labelBundle_.sustainingVoicesLabel->setText("Sustaining: --", juce::dontSendNotification);
        }
    }
}

// ============================================================================
// Private Methods - Helpers
// ============================================================================

InfoHeaderLabels InfoHeaderComponent::getLabelsForLayout() const
{
    InfoHeaderLabels labels;

    labels.instrumentNameLabel = labelBundle_.instrumentNameLabel.get();
    labels.versionLabel = labelBundle_.versionLabel.get();
    labels.sampleRateLabel = labelBundle_.sampleRateLabel.get();
    labels.activeVoicesLabel = labelBundle_.activeVoicesLabel.get();
    labels.sustainingVoicesLabel = labelBundle_.sustainingVoicesLabel.get();
    labels.cpuUsageLabel = labelBundle_.cpuUsageLabel.get();

    return labels;
}