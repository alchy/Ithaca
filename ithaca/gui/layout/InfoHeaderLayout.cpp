/**
 * @file InfoHeaderLayout.cpp
 * @brief Implementation of InfoHeaderComponent layout strategies
 */

#include "ithaca/gui/layout/InfoHeaderLayout.h"
#include "ithaca/gui/helpers/GuiConstants.h"

//==============================================================================
// Public Interface

void InfoHeaderLayout::applyLayout(juce::Rectangle<int> bounds,
                                  const InfoHeaderLabels& labels,
                                  LayoutMode mode)
{
    if (mode == LayoutMode::Background) {
        applyBackgroundMode(bounds, labels);
    } else {
        applyDebugMode(bounds, labels);
    }
}

//==============================================================================
// Private Layout Strategies

void InfoHeaderLayout::applyBackgroundMode(juce::Rectangle<int> bounds,
                                          const InfoHeaderLabels& labels)
{
    // Hierarchical layout:
    // Row 1: Instrument name (large font)
    // Row 2: Version (medium font)
    // Row 3: Sample rate (small font)
    // Row 4: Active | Sustaining (small font, 50/50 split)
    // Row 5: CPU usage (small font, full width)

    if (labels.instrumentNameLabel) {
        labels.instrumentNameLabel->setBounds(
            bounds.removeFromTop(GuiConstants::INFO_TITLE_HEIGHT));
        bounds.removeFromTop(GuiConstants::INFO_ROW_SPACING);
    }

    if (labels.versionLabel) {
        labels.versionLabel->setBounds(
            bounds.removeFromTop(GuiConstants::INFO_VERSION_HEIGHT));
        bounds.removeFromTop(GuiConstants::INFO_ROW_SPACING);
    }

    if (labels.sampleRateLabel) {
        labels.sampleRateLabel->setBounds(
            bounds.removeFromTop(GuiConstants::INFO_SAMPLE_RATE_HEIGHT));
        bounds.removeFromTop(GuiConstants::INFO_ROW_SPACING);
    }

    // Voice stats row - 50/50 split
    auto voiceStatsRow = bounds.removeFromTop(GuiConstants::INFO_VOICE_STATS_HEIGHT);

    if (labels.activeVoicesLabel && labels.sustainingVoicesLabel) {
        int halfWidth = voiceStatsRow.getWidth() / 2;
        labels.activeVoicesLabel->setBounds(voiceStatsRow.removeFromLeft(halfWidth));
        labels.sustainingVoicesLabel->setBounds(voiceStatsRow);
    }

    bounds.removeFromTop(GuiConstants::INFO_ROW_SPACING);

    // CPU usage row (full width)
    if (labels.cpuUsageLabel) {
        labels.cpuUsageLabel->setBounds(
            bounds.removeFromTop(GuiConstants::INFO_VOICE_STATS_HEIGHT));
    }
}

void InfoHeaderLayout::applyDebugMode(juce::Rectangle<int> bounds,
                                     const InfoHeaderLabels& labels)
{
    // Debug mode: compact vertical layout
    const int labelHeight = 18;
    const int spacing = 2;

    if (labels.instrumentNameLabel) {
        labels.instrumentNameLabel->setBounds(bounds.removeFromTop(labelHeight));
        bounds.removeFromTop(spacing);
    }

    if (labels.versionLabel) {
        labels.versionLabel->setBounds(bounds.removeFromTop(labelHeight));
        bounds.removeFromTop(spacing);
    }

    if (labels.sampleRateLabel) {
        labels.sampleRateLabel->setBounds(bounds.removeFromTop(labelHeight));
        bounds.removeFromTop(spacing);
    }

    // Voice stats - 50/50 split
    auto voiceRow = bounds.removeFromTop(labelHeight);
    int halfWidth = voiceRow.getWidth() / 2;

    if (labels.activeVoicesLabel) {
        labels.activeVoicesLabel->setBounds(voiceRow.removeFromLeft(halfWidth));
    }
    if (labels.sustainingVoicesLabel) {
        labels.sustainingVoicesLabel->setBounds(voiceRow);
    }

    bounds.removeFromTop(spacing);

    // CPU usage (full width)
    if (labels.cpuUsageLabel) {
        labels.cpuUsageLabel->setBounds(bounds.removeFromTop(labelHeight));
    }
}
