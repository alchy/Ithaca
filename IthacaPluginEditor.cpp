/**
 * @file IthacaPluginEditor.cpp
 * @brief Implementace GUI editoru s kompaktními slidery pro MIDI parametry
 */

#include "IthacaPluginEditor.h"
#include "decorators/BinaryData.h"
#include <iostream>

// DEBUG: Přepínač pro vypnutí background obrázku
#define BACKGROUND_PICTURE_OFF 0

// Makro pro debug výpisy
#if BACKGROUND_PICTURE_OFF
#define GUI_DEBUG(msg) std::cout << msg << std::endl
#else
#define GUI_DEBUG(msg)
#endif

// Definice konstant
#ifndef CURRENT_INSTRUMENT
#define CURRENT_INSTRUMENT "Ithaca Grand Piano"
#endif

#ifndef PLUGIN_VERSION  
#define PLUGIN_VERSION "1.0.0"
#endif

//==============================================================================
IthacaPluginEditor::IthacaPluginEditor (IthacaPluginProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p), timerCallCounter(0)
{
    GUI_DEBUG("IthacaGUI: Constructor starting with COMPACT SLIDERS");
    
    // Větší okno pro slidery - ale stále kompaktní
    setSize(480, 650);
    
#if !BACKGROUND_PICTURE_OFF
    // Embedded obrázek z BinaryData
    juce::Image image = juce::ImageCache::getFromMemory(BinaryData::ithacaplayer1_jpg, BinaryData::ithacaplayer1_jpgSize);
    imageComponent.setImage(image);
    imageComponent.setImagePlacement(juce::RectanglePlacement::stretchToFit);
    imageComponent.setInterceptsMouseClicks(false, false);
    addAndMakeVisible(imageComponent);
    GUI_DEBUG("IthacaGUI: Background image loaded");
#else
    GUI_DEBUG("IthacaGUI: Background image DISABLED (debug mode)");
#endif
    
    // OPRAVA: Správné pořadí setup metod
    setupMainComponents();      // Vytvoří info labels
    setupCompactSliders();      // Vytvoří slidery
    setupSliderAttachments();   // Propojí slidery s parametry
    
    // DŮLEŽITÉ: Force resized pro správné umístění
    resized();
    
    GUI_DEBUG("IthacaGUI: Constructor completed with sliders - child components: " << getNumChildComponents());
}

IthacaPluginEditor::~IthacaPluginEditor()
{
    stopTimer();
    // Slider attachments se automaticky odstraní
    GUI_DEBUG("IthacaGUI: Destructor called");
}

//==============================================================================
void IthacaPluginEditor::paint (juce::Graphics& g)
{
#if BACKGROUND_PICTURE_OFF
    // DEBUG MODE: Čisté šedé pozadí
    g.fillAll(juce::Colour(0xff808080));
    
    g.setColour(juce::Colour(0xff333333));
    g.setFont(juce::FontOptions(18.0f));
    g.drawText("IthacaCore Sampler - Controls", 10, 10, getWidth() - 20, 30, 
               juce::Justification::centred);
    
    g.setColour(juce::Colour(0xffcccccc));
    g.drawLine(10.0f, 45.0f, static_cast<float>(getWidth() - 10), 45.0f, 1.0f);
    
    GUI_DEBUG("IthacaGUI: Paint method - DEBUG MODE with controls");
#else
    // BACKGROUND MODE: Overlay pro čitelnost
    auto controlArea = getLocalBounds().removeFromTop(350); // Větší oblast pro slidery
    g.setColour(juce::Colours::black.withAlpha(0.6f));
    g.fillRect(controlArea);
    
    // Jemnější border
    g.setColour(juce::Colours::white.withAlpha(0.2f));
    g.drawRect(controlArea, 1);
    
    GUI_DEBUG("IthacaGUI: Paint method - BACKGROUND MODE with controls");
#endif
}

void IthacaPluginEditor::resized()
{
    auto bounds = getLocalBounds();
    
#if !BACKGROUND_PICTURE_OFF
    // Background obrázek zabírá celou plochu
    imageComponent.setBounds(bounds);
    
    // === PŮVODNÍ LAYOUT: Sliders vlevo, info vpravo ===
    auto workArea = bounds.reduced(8);
    
    // Levá část: Sliders (2/3 šířky)
    auto slidersArea = workArea.removeFromLeft(320);
    slidersArea.removeFromTop(20);
    
    // Pravá část: Info labels (původní pozice)
    auto infoArea = workArea;
    infoArea.removeFromTop(60);
    
    // === SLIDERS AREA: Dvousloupcový layout ===
    auto leftSliders = slidersArea.removeFromLeft(150);
    auto rightSliders = slidersArea.removeFromLeft(150);
    
    // Levé slidery: Master controls
    if (masterGainLabel) {
        masterGainLabel->setBounds(leftSliders.removeFromTop(16));
        leftSliders.removeFromTop(2);
    }
    if (masterGainSlider) {
        masterGainSlider->setBounds(leftSliders.removeFromTop(60));
        leftSliders.removeFromTop(8);
    }
    
    if (masterPanLabel) {
        masterPanLabel->setBounds(leftSliders.removeFromTop(16));
        leftSliders.removeFromTop(2);
    }
    if (masterPanSlider) {
        masterPanSlider->setBounds(leftSliders.removeFromTop(60));
        leftSliders.removeFromTop(8);
    }
    
    // Pravé slidery: ADSR + LFO (kompaktně)
    if (attackLabel) {
        attackLabel->setBounds(rightSliders.removeFromTop(16));
        rightSliders.removeFromTop(2);
    }
    if (attackSlider) {
        attackSlider->setBounds(rightSliders.removeFromTop(45));
        rightSliders.removeFromTop(4);
    }
    
    if (releaseLabel) {
        releaseLabel->setBounds(rightSliders.removeFromTop(16));
        rightSliders.removeFromTop(2);
    }
    if (releaseSlider) {
        releaseSlider->setBounds(rightSliders.removeFromTop(45));
        rightSliders.removeFromTop(4);
    }
    
    if (sustainLevelLabel) {
        sustainLevelLabel->setBounds(rightSliders.removeFromTop(16));
        rightSliders.removeFromTop(2);
    }
    if (sustainLevelSlider) {
        sustainLevelSlider->setBounds(rightSliders.removeFromTop(45));
        rightSliders.removeFromTop(8);
    }
    
    // LFO menší
    if (lfoPanSpeedLabel) {
        lfoPanSpeedLabel->setBounds(rightSliders.removeFromTop(14));
        rightSliders.removeFromTop(1);
    }
    if (lfoPanSpeedSlider) {
        lfoPanSpeedSlider->setBounds(rightSliders.removeFromTop(35));
        rightSliders.removeFromTop(3);
    }
    
    if (lfoPanDepthLabel) {
        lfoPanDepthLabel->setBounds(rightSliders.removeFromTop(14));
        rightSliders.removeFromTop(1);
    }
    if (lfoPanDepthSlider) {
        lfoPanDepthSlider->setBounds(rightSliders.removeFromTop(35));
    }
    
    // === INFO AREA: Původní pravý sloupec ===
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
    
    // Version dole
    if (versionLabel) {
        auto versionArea = bounds.removeFromBottom(25).removeFromRight(140).reduced(8);
        versionLabel->setBounds(versionArea);
    }
    
#else
    // DEBUG MODE: Původní layout
    auto controlArea = bounds.removeFromRight(150).removeFromTop(450).reduced(8, 20);
    
    controlArea.removeFromTop(30);
    
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
    
    if (versionLabel) {
        auto versionArea = bounds.removeFromBottom(25).removeFromRight(150).reduced(8);
        versionLabel->setBounds(versionArea);
    }
    
    // Slidery vlevo v debug módu
    auto slidersArea = bounds.removeFromLeft(300).reduced(8, 20);
    
    // Jednoduché umístění sliderů vertikálně
    // Master controls
    if (masterGainLabel) {
        masterGainLabel->setBounds(slidersArea.removeFromTop(16));
        slidersArea.removeFromTop(2);
    }
    if (masterGainSlider) {
        masterGainSlider->setBounds(slidersArea.removeFromLeft(80).removeFromTop(100));
        slidersArea.removeFromTop(-100); // Reset height
        slidersArea.removeFromLeft(10); // Space
    }
    
    if (masterPanLabel) {
        masterPanLabel->setBounds(slidersArea.removeFromTop(16));
        slidersArea.removeFromTop(2);
    }
    if (masterPanSlider) {
        masterPanSlider->setBounds(slidersArea.removeFromLeft(80).removeFromTop(100));
    }
#endif
    
    GUI_DEBUG("IthacaGUI: Resized with improved layout preserving info display");
}

void IthacaPluginEditor::parentHierarchyChanged()
{
    GUI_DEBUG("IthacaGUI: parentHierarchyChanged - starting timer");
    
    if (isShowing() && !isTimerRunning()) {
        startTimer(300);
        timerCallback(); // První update
    }
}

//==============================================================================
void IthacaPluginEditor::sliderValueChanged(juce::Slider* slider)
{
    // Slider attachments automaticky synchronizují s parametry
    // Můžeme přidat custom akce zde pokud potřebujeme
    
    if (slider == lfoPanSpeedSlider.get() || slider == lfoPanDepthSlider.get()) {
        // LFO parametry - můžeme přidat visual feedback
        GUI_DEBUG("IthacaGUI: LFO parameter changed");
    }
    
    // Pro debug můžeme logovat změny
    GUI_DEBUG("IthacaGUI: Slider value changed: " << slider->getValue());
}

//==============================================================================
void IthacaPluginEditor::timerCallback()
{
    // PŮVODNÍ LOGIKA: Ensure all labels have proper bounds (fallback fixing)
    static bool boundsFixed = false;
    if (!boundsFixed) {
        GUI_DEBUG("IthacaGUI: Fixing bounds via timer callback - fallback mode");
        
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
        GUI_DEBUG("IthacaGUI: All bounds fixed with fallback positions");
    }
    
    // AKTUALIZACE ŽIVÝCH DAT (podle původního kódu)
    try {
        if (processorRef.getVoiceManager()) {
            auto stats = processorRef.getSamplerStats();
            
            // Voice counters - živé údaje (PŮVODNÍ FORMÁT)
            if (activeVoicesLabel) {
                updateLabelText(activeVoicesLabel.get(), "Active: " + juce::String(stats.activeVoices));
            }
            
            if (sustainingVoicesLabel) {
                updateLabelText(sustainingVoicesLabel.get(), "Sustaining: " + juce::String(stats.sustainingVoices));
            }
            
            if (releasingVoicesLabel) {
                updateLabelText(releasingVoicesLabel.get(), "Releasing: " + juce::String(stats.releasingVoices));
            }
            
            // Sample rate - živý údaj (PŮVODNÍ FORMÁT)
            if (sampleRateLabel) {
                updateLabelText(sampleRateLabel.get(), "Rate: " + juce::String(stats.currentSampleRate) + "Hz");
            }
            
            // Statické informace - nastavit pouze jednou
            static bool staticInfoSet = false;
            if (!staticInfoSet) {
                if (instrumentLabel) {
                    updateLabelText(instrumentLabel.get(), "Instrument: " + juce::String(CURRENT_INSTRUMENT));
                }
                if (versionLabel) {
                    updateLabelText(versionLabel.get(), "Version: " + juce::String(PLUGIN_VERSION));
                }
                staticInfoSet = true;
                GUI_DEBUG("IthacaGUI: Static info labels set");
            }
            
        } else {
            // Engine není ready - zobrazit fallback hodnoty (PŮVODNÍ FORMÁT)
            if (activeVoicesLabel) updateLabelText(activeVoicesLabel.get(), "Active: --");
            if (sustainingVoicesLabel) updateLabelText(sustainingVoicesLabel.get(), "Sustaining: --");
            if (releasingVoicesLabel) updateLabelText(releasingVoicesLabel.get(), "Releasing: --");
            if (sampleRateLabel) updateLabelText(sampleRateLabel.get(), "Rate: --");
        }
        
    } catch (...) {
        GUI_DEBUG("IthacaGUI: EXCEPTION in timer callback");
        
        // Fallback při chybě
        if (activeVoicesLabel) updateLabelText(activeVoicesLabel.get(), "Active: ERR");
        if (sustainingVoicesLabel) updateLabelText(sustainingVoicesLabel.get(), "Sustaining: ERR");
        if (releasingVoicesLabel) updateLabelText(releasingVoicesLabel.get(), "Releasing: ERR");
        if (sampleRateLabel) updateLabelText(sampleRateLabel.get(), "Rate: ERR");
    }
}

//==============================================================================
void IthacaPluginEditor::setupMainComponents()
{
    GUI_DEBUG("IthacaGUI: Setting up main components...");
    
    // Info labels (malý font pro kompaktnost)
    activeVoicesLabel = createSmallLabel("Active: 0");
    addAndMakeVisible(activeVoicesLabel.get());
    
    sustainingVoicesLabel = createSmallLabel("Sustaining: 0");
    addAndMakeVisible(sustainingVoicesLabel.get());
    
    releasingVoicesLabel = createSmallLabel("Releasing: 0");
    addAndMakeVisible(releasingVoicesLabel.get());
    
    sampleRateLabel = createSmallLabel("Rate: 48000Hz");
    addAndMakeVisible(sampleRateLabel.get());
    
    instrumentLabel = createSmallLabel("Instrument: " + juce::String(CURRENT_INSTRUMENT));
    addAndMakeVisible(instrumentLabel.get());
    
    versionLabel = createSmallLabel("Version: " + juce::String(PLUGIN_VERSION));
    addAndMakeVisible(versionLabel.get());
    
    GUI_DEBUG("IthacaGUI: Main components setup completed");
}

void IthacaPluginEditor::setupCompactSliders()
{
    GUI_DEBUG("IthacaGUI: Setting up compact sliders - START");
    
    // === MASTER CONTROLS ===
    masterGainLabel = createSliderLabel("Master Gain");
    if (masterGainLabel) {
        addAndMakeVisible(masterGainLabel.get());
        GUI_DEBUG("IthacaGUI: masterGainLabel created and added");
    }
    
    masterGainSlider = createCompactSlider(0.0, 127.0, 100.0, 1.0, "");
    if (masterGainSlider) {
        masterGainSlider->addListener(this);
        addAndMakeVisible(masterGainSlider.get());
        GUI_DEBUG("IthacaGUI: masterGainSlider created and added");
    }
    
    masterPanLabel = createSliderLabel("Master Pan");
    if (masterPanLabel) {
        addAndMakeVisible(masterPanLabel.get());
        GUI_DEBUG("IthacaGUI: masterPanLabel created and added");
    }
    
    masterPanSlider = createCompactSlider(0.0, 127.0, 64.0, 1.0, "");
    if (masterPanSlider) {
        masterPanSlider->addListener(this);
        addAndMakeVisible(masterPanSlider.get());
        GUI_DEBUG("IthacaGUI: masterPanSlider created and added");
    }
    
    // === ADSR ENVELOPE ===
    attackLabel = createSliderLabel("Attack");
    if (attackLabel) {
        addAndMakeVisible(attackLabel.get());
        GUI_DEBUG("IthacaGUI: attackLabel created and added");
    }
    
    attackSlider = createCompactSlider(0.0, 127.0, 0.0, 1.0, "");
    if (attackSlider) {
        attackSlider->addListener(this);
        addAndMakeVisible(attackSlider.get());
        GUI_DEBUG("IthacaGUI: attackSlider created and added");
    }
    
    releaseLabel = createSliderLabel("Release");
    if (releaseLabel) {
        addAndMakeVisible(releaseLabel.get());
        GUI_DEBUG("IthacaGUI: releaseLabel created and added");
    }
    
    releaseSlider = createCompactSlider(0.0, 127.0, 4.0, 1.0, "");
    if (releaseSlider) {
        releaseSlider->addListener(this);
        addAndMakeVisible(releaseSlider.get());
        GUI_DEBUG("IthacaGUI: releaseSlider created and added");
    }
    
    sustainLevelLabel = createSliderLabel("Sustain");
    if (sustainLevelLabel) {
        addAndMakeVisible(sustainLevelLabel.get());
        GUI_DEBUG("IthacaGUI: sustainLevelLabel created and added");
    }
    
    sustainLevelSlider = createCompactSlider(0.0, 127.0, 127.0, 1.0, "");
    if (sustainLevelSlider) {
        sustainLevelSlider->addListener(this);
        addAndMakeVisible(sustainLevelSlider.get());
        GUI_DEBUG("IthacaGUI: sustainLevelSlider created and added");
    }
    
    // === LFO PANNING ===
    lfoPanSpeedLabel = createSliderLabel("LFO Speed");
    if (lfoPanSpeedLabel) {
        addAndMakeVisible(lfoPanSpeedLabel.get());
        GUI_DEBUG("IthacaGUI: lfoPanSpeedLabel created and added");
    }
    
    lfoPanSpeedSlider = createCompactSlider(0.0, 127.0, 0.0, 1.0, "");
    if (lfoPanSpeedSlider) {
        lfoPanSpeedSlider->addListener(this);
        addAndMakeVisible(lfoPanSpeedSlider.get());
        GUI_DEBUG("IthacaGUI: lfoPanSpeedSlider created and added");
    }
    
    lfoPanDepthLabel = createSliderLabel("LFO Depth");
    if (lfoPanDepthLabel) {
        addAndMakeVisible(lfoPanDepthLabel.get());
        GUI_DEBUG("IthacaGUI: lfoPanDepthLabel created and added");
    }
    
    lfoPanDepthSlider = createCompactSlider(0.0, 127.0, 0.0, 1.0, "");
    if (lfoPanDepthSlider) {
        lfoPanDepthSlider->addListener(this);
        addAndMakeVisible(lfoPanDepthSlider.get());
        GUI_DEBUG("IthacaGUI: lfoPanDepthSlider created and added");
    }
    
    GUI_DEBUG("IthacaGUI: Compact sliders setup completed - total child components: " << getNumChildComponents());
}

void IthacaPluginEditor::setupSliderAttachments()
{
    GUI_DEBUG("IthacaGUI: Setting up slider attachments - START");
    
    auto& params = processorRef.getParameters();
    GUI_DEBUG("IthacaGUI: Got parameters reference");
    
    // Kontrola, zda existují všechny požadované parametry
    if (!params.getParameter("masterGain")) {
        GUI_DEBUG("IthacaGUI: ERROR - masterGain parameter not found!");
        return;
    }
    if (!params.getParameter("masterPan")) {
        GUI_DEBUG("IthacaGUI: ERROR - masterPan parameter not found!");
        return;
    }
    
    // Master controls
    if (masterGainSlider) {
        sliderAttachments.push_back(
            std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                params, "masterGain", *masterGainSlider));
        GUI_DEBUG("IthacaGUI: masterGain attachment created");
    } else {
        GUI_DEBUG("IthacaGUI: ERROR - masterGainSlider is null!");
    }
    
    if (masterPanSlider) {
        sliderAttachments.push_back(
            std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                params, "masterPan", *masterPanSlider));
        GUI_DEBUG("IthacaGUI: masterPan attachment created");
    } else {
        GUI_DEBUG("IthacaGUI: ERROR - masterPanSlider is null!");
    }
    
    // ADSR
    if (attackSlider && params.getParameter("attack")) {
        sliderAttachments.push_back(
            std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                params, "attack", *attackSlider));
        GUI_DEBUG("IthacaGUI: attack attachment created");
    }
    
    if (releaseSlider && params.getParameter("release")) {
        sliderAttachments.push_back(
            std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                params, "release", *releaseSlider));
        GUI_DEBUG("IthacaGUI: release attachment created");
    }
    
    if (sustainLevelSlider && params.getParameter("sustainLevel")) {
        sliderAttachments.push_back(
            std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                params, "sustainLevel", *sustainLevelSlider));
        GUI_DEBUG("IthacaGUI: sustainLevel attachment created");
    }
    
    // LFO Panning
    if (lfoPanSpeedSlider && params.getParameter("lfoPanSpeed")) {
        sliderAttachments.push_back(
            std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                params, "lfoPanSpeed", *lfoPanSpeedSlider));
        GUI_DEBUG("IthacaGUI: lfoPanSpeed attachment created");
    }
    
    if (lfoPanDepthSlider && params.getParameter("lfoPanDepth")) {
        sliderAttachments.push_back(
            std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                params, "lfoPanDepth", *lfoPanDepthSlider));
        GUI_DEBUG("IthacaGUI: lfoPanDepth attachment created");
    }
    
    GUI_DEBUG("IthacaGUI: Slider attachments completed - " << sliderAttachments.size() << " attachments created");
}

//==============================================================================
std::unique_ptr<juce::Slider> IthacaPluginEditor::createCompactSlider(double min, double max, double defaultVal, 
                                                                     double interval, const juce::String& suffix)
{
    auto slider = std::make_unique<juce::Slider>(juce::Slider::LinearVertical, juce::Slider::TextBoxBelow);
    
    slider->setRange(min, max, interval);
    slider->setValue(defaultVal);
    slider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 40, 16);
    slider->setTextValueSuffix(suffix);
    
    // Compact styling
#if BACKGROUND_PICTURE_OFF
    // Debug mode: jasné barvy na šedém pozadí
    slider->setColour(juce::Slider::trackColourId, juce::Colour(0xff333333));
    slider->setColour(juce::Slider::thumbColourId, juce::Colour(0xff0066cc));
    slider->setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xff000000));
    slider->setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xffffffff));
#else
    // Background mode: kontrastní barvy pro overlay
    slider->setColour(juce::Slider::trackColourId, juce::Colour(SLIDER_TRACK_COLOR));
    slider->setColour(juce::Slider::thumbColourId, juce::Colour(SLIDER_THUMB_COLOR));
    slider->setColour(juce::Slider::textBoxTextColourId, juce::Colour(SLIDER_TEXT_COLOR));
    slider->setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::black.withAlpha(0.7f));
    slider->setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::white.withAlpha(0.3f));
#endif
    
    return slider;
}

std::unique_ptr<juce::Label> IthacaPluginEditor::createSliderLabel(const juce::String& text)
{
    auto label = std::make_unique<juce::Label>();
    label->setText(text, juce::dontSendNotification);
    label->setJustificationType(juce::Justification::centred);
    label->setFont(juce::FontOptions(11.0f, juce::Font::bold));
    
#if BACKGROUND_PICTURE_OFF
    label->setColour(juce::Label::textColourId, juce::Colour(0xff000000));
    label->setColour(juce::Label::backgroundColourId, juce::Colours::transparentWhite);
#else
    label->setColour(juce::Label::textColourId, juce::Colours::white);
    label->setColour(juce::Label::backgroundColourId, juce::Colours::black.withAlpha(0.6f));
    label->setColour(juce::Label::outlineColourId, juce::Colours::white.withAlpha(0.3f));
#endif
    
    return label;
}

//==============================================================================
// Existující helper metody (beze změn)

std::unique_ptr<juce::Label> IthacaPluginEditor::createSmallLabel(const juce::String& text, 
                                                                juce::Justification justification)
{
    auto label = std::make_unique<juce::Label>();
    label->setText(text, juce::dontSendNotification);
    label->setJustificationType(justification);
    
#if BACKGROUND_PICTURE_OFF
    label->setColour(juce::Label::textColourId, juce::Colour(0xff000000));
    label->setColour(juce::Label::backgroundColourId, juce::Colour(0xffffffff));
    label->setColour(juce::Label::outlineColourId, juce::Colour(0xff333333));
#else
    label->setColour(juce::Label::textColourId, juce::Colours::white);
    label->setColour(juce::Label::backgroundColourId, juce::Colours::black.withAlpha(0.75f));
    label->setColour(juce::Label::outlineColourId, juce::Colours::white.withAlpha(0.4f));
#endif
    
    label->setFont(juce::FontOptions(10.0f));
    
    return label;
}

std::unique_ptr<juce::Label> IthacaPluginEditor::createLabel(const juce::String& text, 
                                                           juce::Justification justification)
{
    auto label = std::make_unique<juce::Label>();
    label->setText(text, juce::dontSendNotification);
    label->setJustificationType(justification);
    
#if BACKGROUND_PICTURE_OFF
    label->setColour(juce::Label::textColourId, juce::Colour(0xff000000));
    label->setColour(juce::Label::backgroundColourId, juce::Colour(0xffffffff));
    label->setColour(juce::Label::outlineColourId, juce::Colour(0xff333333));
#else
    label->setColour(juce::Label::textColourId, juce::Colours::white);
    label->setColour(juce::Label::backgroundColourId, juce::Colours::black.withAlpha(0.85f));
    label->setColour(juce::Label::outlineColourId, juce::Colours::white.withAlpha(0.6f));
#endif
    
    label->setFont(juce::FontOptions(14.0f));
    
    return label;
}

void IthacaPluginEditor::updateLabelText(juce::Label* label, const juce::String& newText)
{
    if (label && label->getText() != newText) {
        label->setText(newText, juce::dontSendNotification);
    }
}

//==============================================================================
// Prázdné implementace metod pro kompatibilitu s header

void IthacaPluginEditor::setupLabels() { /* prázdné */ }
void IthacaPluginEditor::setupVoiceActivityGrid() { /* prázdné */ }
void IthacaPluginEditor::updateStatsDisplay() { /* prázdné */ }
void IthacaPluginEditor::updateVoiceActivityDisplay() { /* prázdné */ }
void IthacaPluginEditor::updateMasterControlsDisplay() { /* prázdné */ }
void IthacaPluginEditor::updateSliderValues() { /* prázdné - slider attachments se starají o sync */ }

//==============================================================================
// VoiceActivityComponent - prázdné implementace pro kompatibilitu s header

IthacaPluginEditor::VoiceActivityComponent::VoiceActivityComponent()
{
    voiceStates.fill(0);
}

void IthacaPluginEditor::VoiceActivityComponent::paint(juce::Graphics& g)
{
    juce::ignoreUnused(g);
}

void IthacaPluginEditor::VoiceActivityComponent::resized()
{
    // prázdné
}

void IthacaPluginEditor::VoiceActivityComponent::updateVoiceStates(int active, int sustaining, int releasing)
{
    juce::ignoreUnused(active, sustaining, releasing);
}

void IthacaPluginEditor::VoiceActivityComponent::setVoiceState(uint8_t midiNote, bool isActive, int voiceState)
{
    juce::ignoreUnused(midiNote, isActive, voiceState);
}

juce::Rectangle<int> IthacaPluginEditor::VoiceActivityComponent::getCellBounds(int row, int col) const
{
    juce::ignoreUnused(row, col);
    return juce::Rectangle<int>();
}

juce::Colour IthacaPluginEditor::VoiceActivityComponent::getStateColour(int state) const
{
    juce::ignoreUnused(state);
    return juce::Colour(0xff000000);
}

int IthacaPluginEditor::VoiceActivityComponent::getMidiNoteFromGrid(int row, int col) const
{
    juce::ignoreUnused(row, col);
    return 0;
}