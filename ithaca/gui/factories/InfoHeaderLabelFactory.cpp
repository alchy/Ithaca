/**
 * @file InfoHeaderLabelFactory.cpp
 * @brief Implementation of InfoHeader label factory
 */

#include "ithaca/gui/factories/InfoHeaderLabelFactory.h"
#include "ithaca/audio/IthacaPluginProcessor.h"
#include "ithaca/gui/helpers/GuiHelpers.h"
#include "BuildID.h"

//==============================================================================
// Public Interface

InfoHeaderLabelBundle InfoHeaderLabelFactory::createAllLabels(
    IthacaPluginProcessor& processor,
    bool debugMode)
{
    InfoHeaderLabelBundle bundle;

    // Instrument name - LARGE FONT (18px, bold)
    auto instrumentName = getInstrumentName(processor);
    bundle.instrumentNameLabel = GuiHelpers::createTitleLabel(instrumentName, debugMode);

    // Build ID - MEDIUM FONT (14px)
    bundle.versionLabel = GuiHelpers::createInfoLabel(
        BuildInfo::getBuildString(),
        debugMode);

    // Sample rate - SMALL FONT (11px)
    bundle.sampleRateLabel = GuiHelpers::createSmallLabel(
        "Sample Rate: 48000 Hz",
        debugMode);

    // Active voices - SMALL FONT (11px)
    bundle.activeVoicesLabel = GuiHelpers::createSmallLabel(
        "Active: 0",
        debugMode);

    // Sustaining voices - SMALL FONT (11px)
    bundle.sustainingVoicesLabel = GuiHelpers::createSmallLabel(
        "Sustaining: 0",
        debugMode);

    // CPU usage - SMALL FONT (11px, color-coded)
    bundle.cpuUsageLabel = GuiHelpers::createSmallLabel(
        "CPU: 0% | Dropouts: 0",
        debugMode);

    return bundle;
}

void InfoHeaderLabelFactory::addToComponent(juce::Component& parent,
                                           InfoHeaderLabelBundle& bundle)
{
    if (bundle.instrumentNameLabel) {
        parent.addAndMakeVisible(bundle.instrumentNameLabel.get());
    }
    if (bundle.versionLabel) {
        parent.addAndMakeVisible(bundle.versionLabel.get());
    }
    if (bundle.sampleRateLabel) {
        parent.addAndMakeVisible(bundle.sampleRateLabel.get());
    }
    if (bundle.activeVoicesLabel) {
        parent.addAndMakeVisible(bundle.activeVoicesLabel.get());
    }
    if (bundle.sustainingVoicesLabel) {
        parent.addAndMakeVisible(bundle.sustainingVoicesLabel.get());
    }
    if (bundle.cpuUsageLabel) {
        parent.addAndMakeVisible(bundle.cpuUsageLabel.get());
    }
}

//==============================================================================
// Private Helpers

juce::String InfoHeaderLabelFactory::getInstrumentName(IthacaPluginProcessor& processor)
{
    auto instrumentName = processor.getInstrumentName();
    if (instrumentName.isEmpty()) {
        return "Loading...";
    }
    return instrumentName;
}
