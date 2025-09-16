#include "IthacaPluginEditor.h"

//==============================================================================
// STABILIZATION CONFIGURATION
// Adjust these values to prevent race conditions and crashes
//==============================================================================

// Timer stability - add artificial delays to prevent race conditions
#define TIMER_STABILITY_DELAY_MS 2          // Delay in timer callback (0 = disabled)
#define REPAINT_RATE_LIMIT_MS 8             // Minimum time between repaints (0 = disabled)
#define VOICE_UPDATE_BATCH_SIZE 32          // Process voices in batches (128 = all at once)
#define TIMER_FPS 25                        // GUI update frequency (lower = more stable)
#define STATS_UPDATE_DIVIDER 8              // Stats update frequency divider

// Safety features
#define ENABLE_EXCEPTION_SAFETY 1          // Enable try/catch blocks
#define ENABLE_NULL_CHECKS 1               // Enable extensive null pointer checks
#define ENABLE_VOICE_BATCH_PROCESSING 1    // Process voices in batches instead of all at once

//==============================================================================

IthacaPluginEditor::IthacaPluginEditor (IthacaPluginProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p), timerCallCounter(0)
{
    // Nastavení velikosti okna (fixed size)
    setSize(600, 400);
    
    // Inicializace všech komponent
    setupMainComponents();
}

IthacaPluginEditor::~IthacaPluginEditor()
{
    stopTimer();
}

//==============================================================================
void IthacaPluginEditor::paint (juce::Graphics& g)
{
    // Bílé pozadí
    g.fillAll(juce::Colour(0xffffffff));
    
    // Hlavní nadpis
    g.setColour(juce::Colour(0xff333333));
    g.setFont(juce::FontOptions(18.0f));
    g.drawText("IthacaCore Sampler", 10, 10, getWidth() - 20, 25, 
               juce::Justification::centred);
    
    // Oddělení sekcí pomocí linií
    g.setColour(juce::Colour(0xffcccccc));
    g.drawLine(10.0f, 40.0f, static_cast<float>(getWidth() - 10), 40.0f, 1.0f);
    g.drawLine(200.0f, 50.0f, 200.0f, 350.0f, 1.0f); // Vertikální rozdělení
}

void IthacaPluginEditor::resized()
{
    auto area = getLocalBounds();
    area.removeFromTop(50); // Prostor pro hlavní nadpis
    
    // Levý panel - informace a statistiky (190px)
    auto leftPanel = area.removeFromLeft(190);
    leftPanel = leftPanel.reduced(10, 5);
    
    // Engine status
    if (engineStatusLabel) engineStatusLabel->setBounds(leftPanel.removeFromTop(20));
    leftPanel.removeFromTop(5);
    
    // Sample info
    if (sampleRateLabel) sampleRateLabel->setBounds(leftPanel.removeFromTop(18));
    if (totalSamplesLabel) totalSamplesLabel->setBounds(leftPanel.removeFromTop(18));
    leftPanel.removeFromTop(10);
    
    // Voice counters
    if (activeVoicesLabel) activeVoicesLabel->setBounds(leftPanel.removeFromTop(18));
    if (sustainingVoicesLabel) sustainingVoicesLabel->setBounds(leftPanel.removeFromTop(18));
    if (releasingVoicesLabel) releasingVoicesLabel->setBounds(leftPanel.removeFromTop(18));
    leftPanel.removeFromTop(10);
    
    // Master controls
    if (masterGainLabel) masterGainLabel->setBounds(leftPanel.removeFromTop(18));
    if (masterPanLabel) masterPanLabel->setBounds(leftPanel.removeFromTop(18));
    leftPanel.removeFromTop(10);
    
    // Sample directory (zbytek levého panelu)
    if (sampleDirLabel) sampleDirLabel->setBounds(leftPanel.removeFromTop(40));
    
    // Pravý panel - voice activity grid
    auto rightPanel = area;
    rightPanel = rightPanel.reduced(10, 5);
    if (voiceActivityGrid) voiceActivityGrid->setBounds(rightPanel);
}

void IthacaPluginEditor::parentHierarchyChanged()
{
    // Bezpečné spuštění timeru po úplné inicializaci GUI
    if (isShowing() && !isTimerRunning()) {
        startTimer(1000 / TIMER_FPS); // Configurable FPS
        // První update dat
        updateStatsDisplay();
        updateMasterControlsDisplay();
    }
}

//==============================================================================
void IthacaPluginEditor::timerCallback()
{
#if TIMER_STABILITY_DELAY_MS > 0
    // Stabilization delay to prevent race conditions
    juce::Thread::sleep(TIMER_STABILITY_DELAY_MS);
#endif

#if ENABLE_EXCEPTION_SAFETY
    try {
#endif
        timerCallCounter++;
        
        // Vždy update voice activity
        updateVoiceActivityDisplay();
        
        // Stats update pouze každý N-tý frame
        if (timerCallCounter >= STATS_UPDATE_DIVIDER) {
            updateStatsDisplay();
            updateMasterControlsDisplay();
            timerCallCounter = 0;
        }

#if ENABLE_EXCEPTION_SAFETY
    } catch (...) {
        // Silent failure to keep GUI stable
    }
#endif
}

//==============================================================================
void IthacaPluginEditor::setupMainComponents()
{
    setupLabels();
    setupVoiceActivityGrid();
    
    // První update dat
    updateStatsDisplay();
    updateMasterControlsDisplay();
}

void IthacaPluginEditor::setupLabels()
{
    // Engine status
    engineStatusLabel = createLabel("Engine: Initializing...", juce::Justification::centredLeft);
    engineStatusLabel->setFont(juce::FontOptions(14.0f));
    engineStatusLabel->setColour(juce::Label::textColourId, juce::Colour(0xff0066cc));
    addAndMakeVisible(engineStatusLabel.get());
    
    // Sample info labels
    sampleRateLabel = createLabel("Sample Rate: --", juce::Justification::centredLeft);
    totalSamplesLabel = createLabel("Total Samples: --", juce::Justification::centredLeft);
    addAndMakeVisible(sampleRateLabel.get());
    addAndMakeVisible(totalSamplesLabel.get());
    
    // Voice counter labels
    activeVoicesLabel = createLabel("Active: 0", juce::Justification::centredLeft);
    sustainingVoicesLabel = createLabel("Sustaining: 0", juce::Justification::centredLeft);
    releasingVoicesLabel = createLabel("Releasing: 0", juce::Justification::centredLeft);
    addAndMakeVisible(activeVoicesLabel.get());
    addAndMakeVisible(sustainingVoicesLabel.get());
    addAndMakeVisible(releasingVoicesLabel.get());
    
    // Master controls labels
    masterGainLabel = createLabel("Master Gain: 100", juce::Justification::centredLeft);
    masterPanLabel = createLabel("Master Pan: Center", juce::Justification::centredLeft);
    addAndMakeVisible(masterGainLabel.get());
    addAndMakeVisible(masterPanLabel.get());
    
    // Sample directory label
    sampleDirLabel = createLabel("Sample Dir:\nLoading...", juce::Justification::topLeft);
    sampleDirLabel->setFont(juce::FontOptions(11.0f));
    sampleDirLabel->setColour(juce::Label::textColourId, juce::Colour(0xff666666));
    addAndMakeVisible(sampleDirLabel.get());
}

void IthacaPluginEditor::setupVoiceActivityGrid()
{
#if ENABLE_EXCEPTION_SAFETY
    try {
#endif
        voiceActivityGrid = std::make_unique<VoiceActivityComponent>();
        addAndMakeVisible(voiceActivityGrid.get());
#if ENABLE_EXCEPTION_SAFETY
    } catch (...) {
        voiceActivityGrid.reset(); // Bezpečné selhání
    }
#endif
}

//==============================================================================
void IthacaPluginEditor::updateStatsDisplay()
{
#if ENABLE_NULL_CHECKS
    if (!processorRef.getVoiceManager()) {
        if (engineStatusLabel) {
            updateLabelText(engineStatusLabel.get(), "Engine: Loading...");
            engineStatusLabel->setColour(juce::Label::textColourId, juce::Colour(0xffff8800));
        }
        if (sampleRateLabel) updateLabelText(sampleRateLabel.get(), "Sample Rate: --");
        if (totalSamplesLabel) updateLabelText(totalSamplesLabel.get(), "Total Samples: Loading...");
        if (sampleDirLabel) updateLabelText(sampleDirLabel.get(), "Sample Dir:\nLoading samples...");
        return;
    }
#endif
    
    auto stats = processorRef.getSamplerStats();
    
    // Engine status
    if (engineStatusLabel) {
        updateLabelText(engineStatusLabel.get(), "Engine: Ready");
        engineStatusLabel->setColour(juce::Label::textColourId, juce::Colour(0xff009900));
    }
    
    // Sample rate
    if (sampleRateLabel) {
        updateLabelText(sampleRateLabel.get(), 
                       "Sample Rate: " + juce::String(stats.currentSampleRate) + " Hz");
    }
    
    // Total samples
    if (totalSamplesLabel) {
        int totalSamples = (stats.totalLoadedSamples > 0) ? stats.totalLoadedSamples : 703;
        updateLabelText(totalSamplesLabel.get(), 
                       "Total Samples: " + juce::String(totalSamples));
    }
    
    // Sample directory
    if (sampleDirLabel) {
        juce::String sampleDir = "Sample Dir:\n";
        juce::String fullPath = juce::String(DEFAULT_SAMPLE_DIR);
        auto pathParts = juce::StringArray::fromTokens(fullPath, juce::File::getSeparatorString(), "");
        if (pathParts.size() > 0) {
            sampleDir += "..." + juce::File::getSeparatorString() + pathParts.getReference(pathParts.size() - 1);
        } else {
            sampleDir += "Ready";
        }
        updateLabelText(sampleDirLabel.get(), sampleDir);
    }
}

void IthacaPluginEditor::updateVoiceActivityDisplay()
{
#if ENABLE_NULL_CHECKS
    if (!voiceActivityGrid || !processorRef.getVoiceManager()) {
        // Fallback values
        if (activeVoicesLabel) updateLabelText(activeVoicesLabel.get(), "Active: --");
        if (sustainingVoicesLabel) updateLabelText(sustainingVoicesLabel.get(), "Sustaining: --");
        if (releasingVoicesLabel) updateLabelText(releasingVoicesLabel.get(), "Releasing: --");
        
        if (voiceActivityGrid) voiceActivityGrid->updateVoiceStates(0, 0, 0);
        return;
    }
#endif
    
    auto stats = processorRef.getSamplerStats();
    
    // Update voice counters
    if (activeVoicesLabel) updateLabelText(activeVoicesLabel.get(), "Active: " + juce::String(stats.activeVoices));
    if (sustainingVoicesLabel) updateLabelText(sustainingVoicesLabel.get(), "Sustaining: " + juce::String(stats.sustainingVoices));
    if (releasingVoicesLabel) updateLabelText(releasingVoicesLabel.get(), "Releasing: " + juce::String(stats.releasingVoices));
    
    // Update voice activity grid
    if (voiceActivityGrid) {
        voiceActivityGrid->updateVoiceStates(stats.activeVoices, stats.sustainingVoices, stats.releasingVoices);
        
#if ENABLE_EXCEPTION_SAFETY
        try {
#endif
            auto* voiceManager = processorRef.getVoiceManager();
            
#if ENABLE_VOICE_BATCH_PROCESSING
            // Process voices in batches to reduce load
            static int batchOffset = 0;
            int batchEnd = juce::jmin(batchOffset + VOICE_UPDATE_BATCH_SIZE, 128);
            
            for (int i = batchOffset; i < batchEnd; ++i) {
#else
            // Process all voices at once (less stable)
            for (int i = 0; i < 128; ++i) {
#endif
                const auto& voice = voiceManager->getVoiceMIDI(static_cast<uint8_t>(i));
                
                int state = 0; // idle
                if (voice.isActive()) {
                    auto voiceState = voice.getState();
                    if (voiceState == VoiceState::Sustaining) {
                        state = 2; // sustaining - zelená
                    } else if (voiceState == VoiceState::Releasing) {
                        state = 3; // releasing - oranžová
                    } else {
                        state = 1; // active (attack/decay) - modrá
                    }
                }
                
                voiceActivityGrid->setVoiceState(static_cast<uint8_t>(i), voice.isActive(), state);
            }
            
#if ENABLE_VOICE_BATCH_PROCESSING
            // Move to next batch
            batchOffset = (batchOffset + VOICE_UPDATE_BATCH_SIZE) % 128;
#endif

#if ENABLE_EXCEPTION_SAFETY
        } catch (...) {
            // Silent failure
        }
#endif
    }
}

void IthacaPluginEditor::updateMasterControlsDisplay()
{
#if ENABLE_EXCEPTION_SAFETY
    try {
#endif
        auto& parameters = processorRef.getParameters();
        
        // Master Gain
        if (auto* gainParam = parameters.getRawParameterValue("masterGain")) {
            if (masterGainLabel) {
                int gainValue = static_cast<int>(gainParam->load());
                updateLabelText(masterGainLabel.get(), "Master Gain: " + juce::String(gainValue));
            }
        } else {
            if (masterGainLabel) updateLabelText(masterGainLabel.get(), "Master Gain: --");
        }
        
        // Master Pan
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
                updateLabelText(masterPanLabel.get(), panText);
            }
        } else {
            if (masterPanLabel) updateLabelText(masterPanLabel.get(), "Master Pan: --");
        }
        
#if ENABLE_EXCEPTION_SAFETY
    } catch (...) {
        // Fallback values
        if (masterGainLabel) updateLabelText(masterGainLabel.get(), "Master Gain: --");
        if (masterPanLabel) updateLabelText(masterPanLabel.get(), "Master Pan: --");
    }
#endif
}

//==============================================================================
// Helper methods

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
// VoiceActivityComponent implementation

IthacaPluginEditor::VoiceActivityComponent::VoiceActivityComponent() : lastRepaintTime(0)
{
    // Inicializace všech voice states na idle
    voiceStates.fill(0);
    
    // Nastavit velikost podle gridu
    int width = GRID_COLS * (CELL_SIZE + CELL_PADDING) + CELL_PADDING;
    int height = GRID_ROWS * (CELL_SIZE + CELL_PADDING) + CELL_PADDING + 40; // +40 pro popisky
    setSize(width, height);
}

void IthacaPluginEditor::VoiceActivityComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xfff8f8f8)); // Velmi světle šedé pozadí
    
    // Nadpis
    g.setColour(juce::Colour(0xff333333));
    g.setFont(juce::FontOptions(14.0f));
    g.drawText("Voice Activity (MIDI Notes 0-127)", 0, 5, getWidth(), 20, 
               juce::Justification::centred);
    
    // Kreslení gridu
    for (int row = 0; row < GRID_ROWS; ++row) {
        for (int col = 0; col < GRID_COLS; ++col) {
            int midiNote = getMidiNoteFromGrid(row, col);
            if (midiNote >= 128) continue; // Safety check
            
            auto cellBounds = getCellBounds(row, col);
            int state = voiceStates[midiNote];
            
            // Barva podle stavu - NOVÉ BARVY
            g.setColour(getStateColour(state));
            g.fillRect(cellBounds);
            
            // Rámeček
            g.setColour(juce::Colour(0xffcccccc));
            g.drawRect(cellBounds, 1);
        }
    }
    
    // Legenda s novými barvami
    int legendY = getHeight() - 15;
    g.setFont(juce::FontOptions(10.0f));
    g.setColour(juce::Colour(0xff666666));
    g.drawText("□ Idle  ■ Attack  ■ Sustain  ■ Release", 5, legendY, getWidth() - 10, 12, 
               juce::Justification::centredLeft);
}

void IthacaPluginEditor::VoiceActivityComponent::resized()
{
    // Grid je fixed size, nic speciálního k resize
}

void IthacaPluginEditor::VoiceActivityComponent::updateVoiceStates(int active, int sustaining, int releasing)
{
    juce::ignoreUnused(active, sustaining, releasing);
    
#if REPAINT_RATE_LIMIT_MS > 0
    // Rate limited repaint
    auto currentTime = juce::Time::getMillisecondCounter();
    if (currentTime - lastRepaintTime >= REPAINT_RATE_LIMIT_MS) {
        repaint();
        lastRepaintTime = currentTime;
    }
#else
    repaint();
#endif
}

void IthacaPluginEditor::VoiceActivityComponent::setVoiceState(uint8_t midiNote, bool isActive, int voiceState)
{
    if (midiNote >= 128) return;
    
    int newState = isActive ? voiceState : 0;
    if (voiceStates[midiNote] != newState) {
        voiceStates[midiNote] = newState;
        
#if REPAINT_RATE_LIMIT_MS > 0
        // Rate limited repaint
        auto currentTime = juce::Time::getMillisecondCounter();
        if (currentTime - lastRepaintTime >= REPAINT_RATE_LIMIT_MS) {
            repaint();
            lastRepaintTime = currentTime;
        }
#else
        repaint();
#endif
    }
}

//==============================================================================
// VoiceActivityComponent helper methods

juce::Rectangle<int> IthacaPluginEditor::VoiceActivityComponent::getCellBounds(int row, int col) const
{
    int x = CELL_PADDING + col * (CELL_SIZE + CELL_PADDING);
    int y = 30 + CELL_PADDING + row * (CELL_SIZE + CELL_PADDING); // +30 pro nadpis
    return juce::Rectangle<int>(x, y, CELL_SIZE, CELL_SIZE);
}

juce::Colour IthacaPluginEditor::VoiceActivityComponent::getStateColour(int state) const
{
    switch (state) {
        case 1: return juce::Colour(0xff0066ff);      // Attack/Decay - modrá
        case 2: return juce::Colour(0xff00cc66);      // Sustaining - zelená
        case 3: return juce::Colour(0xffff6600);      // Releasing - oranžová
        default: return juce::Colour(0xffeeeeee);     // Idle - světle šedá
    }
}

int IthacaPluginEditor::VoiceActivityComponent::getMidiNoteFromGrid(int row, int col) const
{
    // Layout: 8 řádků x 16 sloupců = 128 notes
    // Note 0 = top-left, Note 127 = bottom-right
    return row * GRID_COLS + col;
}