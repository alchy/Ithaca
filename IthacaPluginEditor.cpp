#include "IthacaPluginEditor.h"
#include "IthacaPluginProcessor.h"

//==============================================================================
// CONFIG
#define TIMER_STABILITY_DELAY_MS 2
#define REPAINT_RATE_LIMIT_MS 8
#define VOICE_UPDATE_BATCH_SIZE 32
#define TIMER_FPS 25
#define STATS_UPDATE_DIVIDER 8

#define ENABLE_EXCEPTION_SAFETY 1
#define ENABLE_NULL_CHECKS 1
#define ENABLE_VOICE_BATCH_PROCESSING 1

//==============================================================================
// Konstruktor / Destruktor
IthacaPluginEditor::IthacaPluginEditor (IthacaPluginProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p), timerCallCounter (0)
{
    setSize (600, 400);
    setupMainComponents();
}

IthacaPluginEditor::~IthacaPluginEditor()
{
    stopTimer();
}

//==============================================================================
// PAINT / LAYOUT
void IthacaPluginEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (BG_COLOR));
    g.setColour (juce::Colour (TEXT_COLOR));
    g.setFont (juce::FontOptions (18.0f));
    g.drawText ("IthacaCore Sampler", 10, 10, getWidth() - 20, 25,
                juce::Justification::centred);

    g.setColour (juce::Colour (BORDER_COLOR));
    g.drawLine (10.0f, 40.0f, (float) (getWidth() - 10), 40.0f, 1.0f);
    g.drawLine (200.0f, 50.0f, 200.0f, 350.0f, 1.0f);
}

void IthacaPluginEditor::resized()
{
    auto area = getLocalBounds();
    area.removeFromTop (50);

    auto leftPanel = area.removeFromLeft (190).reduced (10, 5);
    if (engineStatusLabel) engineStatusLabel->setBounds (leftPanel.removeFromTop (20));
    leftPanel.removeFromTop (5);

    if (sampleRateLabel) sampleRateLabel->setBounds (leftPanel.removeFromTop (18));
    if (totalSamplesLabel) totalSamplesLabel->setBounds (leftPanel.removeFromTop (18));
    leftPanel.removeFromTop (10);

    if (activeVoicesLabel) activeVoicesLabel->setBounds (leftPanel.removeFromTop (18));
    if (sustainingVoicesLabel) sustainingVoicesLabel->setBounds (leftPanel.removeFromTop (18));
    if (releasingVoicesLabel) releasingVoicesLabel->setBounds (leftPanel.removeFromTop (18));
    leftPanel.removeFromTop (10);

    if (masterGainLabel) masterGainLabel->setBounds (leftPanel.removeFromTop (18));
    if (masterPanLabel) masterPanLabel->setBounds (leftPanel.removeFromTop (18));
    leftPanel.removeFromTop (10);

    if (sampleDirLabel) sampleDirLabel->setBounds (leftPanel.removeFromTop (40));

    auto rightPanel = area.reduced (10, 5);
    if (voiceActivityGrid) voiceActivityGrid->setBounds (rightPanel);
}

void IthacaPluginEditor::parentHierarchyChanged()
{
    if (isShowing() && !isTimerRunning())
    {
        startTimer (1000 / TIMER_FPS);
        updateStatsDisplay();
        updateMasterControlsDisplay();
    }
}

//==============================================================================
// TIMER
void IthacaPluginEditor::timerCallback()
{
#if TIMER_STABILITY_DELAY_MS > 0
    juce::Thread::sleep (TIMER_STABILITY_DELAY_MS);
#endif

#if ENABLE_EXCEPTION_SAFETY
    try {
#endif
        timerCallCounter++;
        updateVoiceActivityDisplay();

        if (timerCallCounter >= STATS_UPDATE_DIVIDER)
        {
            updateStatsDisplay();
            updateMasterControlsDisplay();
            timerCallCounter = 0;
        }

#if ENABLE_EXCEPTION_SAFETY
    } catch (...) {}
#endif
}

//==============================================================================
// INITIALIZATION
void IthacaPluginEditor::setupMainComponents()
{
    setupLabels();
    setupVoiceActivityGrid();
    updateStatsDisplay();
    updateMasterControlsDisplay();
}

void IthacaPluginEditor::setupLabels()
{
    engineStatusLabel = createLabel ("Engine: Initializing...");
    engineStatusLabel->setFont (juce::FontOptions (14.0f));
    engineStatusLabel->setColour (juce::Label::textColourId, juce::Colour (ACCENT_COLOR));
    addAndMakeVisible (engineStatusLabel.get());

    sampleRateLabel = createLabel ("Sample Rate: --");
    totalSamplesLabel = createLabel ("Total Samples: --");
    addAndMakeVisible (sampleRateLabel.get());
    addAndMakeVisible (totalSamplesLabel.get());

    activeVoicesLabel = createLabel ("Active: 0");
    sustainingVoicesLabel = createLabel ("Sustaining: 0");
    releasingVoicesLabel = createLabel ("Releasing: 0");
    addAndMakeVisible (activeVoicesLabel.get());
    addAndMakeVisible (sustainingVoicesLabel.get());
    addAndMakeVisible (releasingVoicesLabel.get());

    masterGainLabel = createLabel ("Master Gain: 100");
    masterPanLabel = createLabel ("Master Pan: Center");
    addAndMakeVisible (masterGainLabel.get());
    addAndMakeVisible (masterPanLabel.get());

    sampleDirLabel = createLabel ("Sample Dir:\nLoading...", juce::Justification::topLeft);
    sampleDirLabel->setFont (juce::FontOptions (11.0f));
    addAndMakeVisible (sampleDirLabel.get());
}

void IthacaPluginEditor::setupVoiceActivityGrid()
{
    voiceActivityGrid = std::make_unique<VoiceActivityComponent>();
    addAndMakeVisible (voiceActivityGrid.get());
}

//==============================================================================
// UPDATE METHODS
void IthacaPluginEditor::updateStatsDisplay()
{
    if (!processorRef.getVoiceManager())
    {
        if (engineStatusLabel)
            updateLabelText (engineStatusLabel.get(), "Engine: Loading...");
        return;
    }

    auto stats = processorRef.getSamplerStats();
    updateLabelText (engineStatusLabel.get(), "Engine: Ready");
    updateLabelText (sampleRateLabel.get(), "Sample Rate: " + juce::String (stats.currentSampleRate) + " Hz");
    updateLabelText (totalSamplesLabel.get(), "Total Samples: " + juce::String (stats.totalLoadedSamples));
}

void IthacaPluginEditor::updateVoiceActivityDisplay()
{
    if (voiceActivityGrid)
        voiceActivityGrid->updateVoiceStates (0, 0, 0); // Zde můžeš přidat reálná data
}

void IthacaPluginEditor::updateMasterControlsDisplay()
{
    auto& parameters = processorRef.getParameters();

    if (auto* gainParam = parameters.getRawParameterValue ("masterGain"))
        updateLabelText (masterGainLabel.get(), "Master Gain: " + juce::String ((int)gainParam->load()));

    if (auto* panParam = parameters.getRawParameterValue ("masterPan"))
    {
        float panValue = panParam->load();
        juce::String text = "Master Pan: ";
        if (panValue < -0.5f) text += "L" + juce::String ((int)-panValue);
        else if (panValue > 0.5f) text += "R" + juce::String ((int)panValue);
        else text += "Center";
        updateLabelText (masterPanLabel.get(), text);
    }
}

//==============================================================================
// HELPERS
std::unique_ptr<juce::Label> IthacaPluginEditor::createLabel (const juce::String& text,
                                                              juce::Justification justification)
{
    auto label = std::make_unique<juce::Label>();
    label->setText (text, juce::dontSendNotification);
    label->setJustificationType (justification);
    label->setColour (juce::Label::textColourId, juce::Colour (TEXT_COLOR));
    label->setFont (juce::FontOptions (12.0f));
    return label;
}

void IthacaPluginEditor::updateLabelText (juce::Label* label, const juce::String& newText)
{
    if (label && label->getText() != newText)
        label->setText (newText, juce::dontSendNotification);
}

//==============================================================================
// VoiceActivityComponent IMPLEMENTATION
IthacaPluginEditor::VoiceActivityComponent::VoiceActivityComponent()
{
    voiceStates.fill (0);
}

void IthacaPluginEditor::VoiceActivityComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xfff8f8f8));
    for (int row = 0; row < GRID_ROWS; ++row)
    {
        for (int col = 0; col < GRID_COLS; ++col)
        {
            auto bounds = getCellBounds (row, col);
            g.setColour (getStateColour (voiceStates[getMidiNoteFromGrid(row, col)]));
            g.fillRect (bounds);
            g.setColour (juce::Colour (BORDER_COLOR));
            g.drawRect (bounds, 1);
        }
    }
}

void IthacaPluginEditor::VoiceActivityComponent::resized() {}

void IthacaPluginEditor::VoiceActivityComponent::updateVoiceStates (int, int, int)
{
    repaint();
}

void IthacaPluginEditor::VoiceActivityComponent::setVoiceState (uint8_t midiNote, bool isActive, int voiceState)
{
    if (midiNote < 128)
    {
        voiceStates[midiNote] = isActive ? voiceState : 0;
        repaint();
    }
}

juce::Rectangle<int> IthacaPluginEditor::VoiceActivityComponent::getCellBounds (int row, int col) const
{
    int x = CELL_PADDING + col * (CELL_SIZE + CELL_PADDING);
    int y = 30 + CELL_PADDING + row * (CELL_SIZE + CELL_PADDING);
    return { x, y, CELL_SIZE, CELL_SIZE };
}

juce::Colour IthacaPluginEditor::VoiceActivityComponent::getStateColour (int state) const
{
    switch (state)
    {
        case 1: return juce::Colour (0xff0066ff);
        case 2: return juce::Colour (0xff00cc66);
        case 3: return juce::Colour (0xffff6600);
        default: return juce::Colour (0xffeeeeee);
    }
}

int IthacaPluginEditor::VoiceActivityComponent::getMidiNoteFromGrid (int row, int col) const
{
    return row * GRID_COLS + col;
}
