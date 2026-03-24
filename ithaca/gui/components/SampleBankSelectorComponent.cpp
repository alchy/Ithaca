/**
 * @file SampleBankSelectorComponent.cpp
 * @brief Implementation of Sample Bank Selector Component
 */

#include "SampleBankSelectorComponent.h"

SampleBankSelectorComponent::SampleBankSelectorComponent(IthacaPluginProcessor& processor)
    : processorRef_(processor) {
    setupComponents();
    startTimerHz(2); // 2 Hz = 500ms interval (slower update rate)
}

SampleBankSelectorComponent::~SampleBankSelectorComponent() {
    stopTimer();
}

void SampleBankSelectorComponent::setupComponents() {
    // Sample bank label
    sampleBankLabel_.setJustificationType(juce::Justification::centred);
    sampleBankLabel_.setColour(juce::Label::textColourId, juce::Colours::white);
    sampleBankLabel_.setFont(juce::FontOptions(14.0f, juce::Font::plain));
    addAndMakeVisible(sampleBankLabel_);

    // Load button
    loadButton_.setButtonText("Load Sample Bank...");
    loadButton_.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF2C3E50));
    loadButton_.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    loadButton_.onClick = [this]() { loadButtonClicked(); };
    addAndMakeVisible(loadButton_);

    // Initial status update
    updateStatus();
}

void SampleBankSelectorComponent::paint(juce::Graphics& g) {
    // Rounded overlay background (80% alpha, 6px radius)
    g.setColour(juce::Colour(0x20, 0x20, 0x20).withAlpha(0.8f));
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 6.0f);

    // Border
    g.setColour(juce::Colour(0xFF3A3A3A));
    g.drawRoundedRectangle(getLocalBounds().toFloat(), 6.0f, 1.0f);
}

void SampleBankSelectorComponent::resized() {
    auto area = getLocalBounds().reduced(10);

    // Sample bank label at top
    sampleBankLabel_.setBounds(area.removeFromTop(25));

    area.removeFromTop(5); // Spacing

    // Load button
    loadButton_.setBounds(area.removeFromTop(30));
}

void SampleBankSelectorComponent::timerCallback() {
    updateStatus();
}

void SampleBankSelectorComponent::updateStatus() {
    // Get current sample bank path from processor
    juce::String currentPath = processorRef_.getLoadedSampleBankPath();

    // Update label with current state
    if (currentPath.isEmpty()) {
        sampleBankLabel_.setText("Sample Bank: Sine Wave Test Tone",
                                juce::dontSendNotification);
    } else {
        juce::String bankName = getSampleBankNameFromPath(currentPath);
        sampleBankLabel_.setText("Sample Bank: " + bankName,
                                juce::dontSendNotification);
    }
}

void SampleBankSelectorComponent::loadButtonClicked() {
    // Create file chooser for directory selection
    fileChooser_ = std::make_unique<juce::FileChooser>(
        "Select Sample Bank Directory",
        juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
        "",
        true // Use native dialog
    );

    auto folderChooserFlags = juce::FileBrowserComponent::openMode |
                              juce::FileBrowserComponent::canSelectDirectories;

    fileChooser_->launchAsync(folderChooserFlags, [this](const juce::FileChooser& chooser) {
        auto results = chooser.getResults();

        if (results.isEmpty()) {
            // User cancelled
            return;
        }

        auto selectedDirectory = results.getFirst();

        if (!selectedDirectory.isDirectory()) {
            // Not a directory - show error
            juce::AlertWindow::showMessageBoxAsync(
                juce::AlertWindow::WarningIcon,
                "Invalid Selection",
                "Please select a directory containing sample bank files.",
                "OK"
            );
            return;
        }

        // Load sample bank from selected directory
        processorRef_.loadSampleBankFromDirectory(selectedDirectory.getFullPathName());

        // Update status immediately
        updateStatus();
    });
}

juce::String SampleBankSelectorComponent::getSampleBankNameFromPath(const juce::String& path) const {
    if (path.isEmpty()) {
        return "None";
    }

    // Get last component of path (directory name)
    juce::File file(path);
    return file.getFileName();
}

void SampleBankSelectorComponent::startUpdates() {
    startTimerHz(2); // 2 Hz = 500ms interval
}

void SampleBankSelectorComponent::stopUpdates() {
    stopTimer();
}
