/**
 * @file GuiHelpers.h
 * @brief GUI helper funkce pro vytváření komponent a styling
 * 
 * ============================================================================
 * GUI REFACTORING - HIERARCHICAL LAYOUT
 * ============================================================================
 * 
 * Nové funkce v této verzi:
 * - createTitleLabel() - Velký font pro instrument name (18px bold)
 * - drawRoundedOverlay() - Zaoblené overlay panely s alpha
 * - drawSeparatorLine() - Průhledné separátory mezi slider řádky
 * 
 * Aktualizované funkce:
 * - createSliderLabel() - Střední font (14px) pro slider labels
 * - createInfoLabel() - Střední font (14px) pro version info
 * - createSmallLabel() - Malý font (11px) pro statistiky
 * 
 * Zachováno:
 * - createCompactSlider() - Horizontální slidery
 * - styleSlider() - Debug/Background mode styling
 * - positionHorizontalSliderWithLabel() - Layout helper
 * - layoutTwoColumnSliders() - 50/50 split
 * ============================================================================
 */

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "ithaca/gui/helpers/GuiConstants.h"

class GuiHelpers {
public:
    
    // =========================================================================
    // SLIDER CREATION
    // =========================================================================
    
    /**
     * @brief Vytvoří kompaktní horizontální slider
     * @param min Minimální hodnota
     * @param max Maximální hodnota
     * @param defaultVal Výchozí hodnota
     * @param interval Krok změny
     * @return Unique pointer na Slider
     */
    static std::unique_ptr<juce::Slider> createCompactSlider(
        double min, double max, double defaultVal, double interval = 1.0);
    
    /**
     * @brief Aplikuje styling na slider podle debug mode
     * @param slider Reference na slider k ostylování
     * @param debugMode true = debug colors, false = overlay colors
     */
    static void styleSlider(juce::Slider& slider, bool debugMode);
    
    // =========================================================================
    // LABEL CREATION - Hierarchie fontů
    // =========================================================================
    
    /**
     * @brief Vytvoří title label (18px, bold) pro instrument name
     * @param text Text labelu
     * @param debugMode Debug mode flag
     * @return Unique pointer na Label
     * @note NOVÉ - používá GuiConstants::TITLE_FONT_SIZE
     */
    static std::unique_ptr<juce::Label> createTitleLabel(
        const juce::String& text, bool debugMode = false);
    
    /**
     * @brief Vytvoří slider label (14px, bold) pro slider názvy
     * @param text Text labelu
     * @param debugMode Debug mode flag
     * @return Unique pointer na Label
     * @note Používá GuiConstants::INFO_LABEL_FONT_SIZE
     */
    static std::unique_ptr<juce::Label> createSliderLabel(
        const juce::String& text, bool debugMode = false);
    
    /**
     * @brief Vytvoří info label (14px) pro version a static info
     * @param text Text labelu
     * @param debugMode Debug mode flag
     * @return Unique pointer na Label
     * @note Používá GuiConstants::INFO_LABEL_FONT_SIZE
     */
    static std::unique_ptr<juce::Label> createInfoLabel(
        const juce::String& text, bool debugMode = false);
    
    /**
     * @brief Vytvoří small label (11px) pro statistiky a detail info
     * @param text Text labelu
     * @param debugMode Debug mode flag
     * @return Unique pointer na Label
     * @note Používá GuiConstants::SMALL_LABEL_FONT_SIZE
     */
    static std::unique_ptr<juce::Label> createSmallLabel(
        const juce::String& text, bool debugMode = false);
    
    // =========================================================================
    // LAYOUT HELPERS
    // =========================================================================
    
    /**
     * @brief Umístí horizontální slider s labelem nad ním
     * @param area Reference na area (bude změněna - removeFromTop)
     * @param label Pointer na label (nullable)
     * @param slider Pointer na slider (nullable)
     * @note Label nad sliderem s GuiConstants::SLIDER_LABEL_SPACING mezerou
     */
    static void positionHorizontalSliderWithLabel(
        juce::Rectangle<int>& area, 
        juce::Label* label, 
        juce::Slider* slider);
    
    /**
     * @brief Rozdělí area na dva sloupce (50/50 split)
     * @param totalArea Celková oblast k rozdělení
     * @param leftColumn Output: Levý sloupec
     * @param rightColumn Output: Pravý sloupec
     * @return Zbývající prostor (pokud existuje)
     * @note Používá GuiConstants::COLUMN_SPACING mezi sloupci
     */
    static juce::Rectangle<int> layoutTwoColumnSliders(
        juce::Rectangle<int> totalArea, 
        juce::Rectangle<int>& leftColumn, 
        juce::Rectangle<int>& rightColumn);
    
    // =========================================================================
    // OVERLAY RENDERING - NOVÉ pro hierarchický layout
    // =========================================================================
    
    /**
     * @brief Vykreslí zaoblené overlay pozadí s průhledností
     * @param g Graphics context
     * @param area Oblast pro overlay
     * @param alpha Průhlednost (0.0-1.0)
     * @param cornerRadius Poloměr zaoblení rohů v pixelech
     * @note NOVÉ - používá se pro info header (80%) a slider panel (60%)
     */
    static void drawRoundedOverlay(juce::Graphics& g, 
                                   juce::Rectangle<int> area, 
                                   float alpha, 
                                   float cornerRadius);
    
    /**
     * @brief Vykreslí průhlednou separační čáru mezi slider řádky
     * @param g Graphics context
     * @param x1 Začátek X
     * @param y1 Začátek Y
     * @param x2 Konec X
     * @param y2 Konec Y
     * @param baseColor Základní barva čáry (aplikuje se GuiConstants::SEPARATOR_ALPHA)
     * @note NOVÉ - používá se mezi řádky sliderů
     */
    static void drawSeparatorLine(juce::Graphics& g, 
                                  int x1, int y1, int x2, int y2,
                                  juce::Colour baseColor = juce::Colours::white);
    
    // =========================================================================
    // LEGACY FUNCTIONS - Zachováno pro kompatibilitu
    // =========================================================================
    
    /**
     * @brief Aplikuje overlay pro control area (legacy)
     * @param g Graphics context
     * @param area Oblast pro overlay
     * @note Legacy: nyní používá drawRoundedOverlay()
     */
    static void applyControlAreaOverlay(juce::Graphics& g, 
                                       juce::Rectangle<int> area);
    
    /**
     * @brief Aplikuje debug background (ZACHOVÁNO)
     * @param g Graphics context
     * @param area Oblast pro background
     */
    static void applyDebugBackground(juce::Graphics& g, 
                                    juce::Rectangle<int> area);
    
    // =========================================================================
    // UTILITY FUNCTIONS
    // =========================================================================
    
    /**
     * @brief Zjistí, zda je zapnutý debug mode
     * @return true pokud BACKGROUND_PICTURE_OFF != 0
     */
    static bool isDebugModeEnabled();
    
    /**
     * @brief Aktualizuje text labelu (pouze pokud se změnil)
     * @param label Pointer na label
     * @param newText Nový text
     */
    static void updateLabelText(juce::Label* label, const juce::String& newText);

private:
    // Disable instantiation
    GuiHelpers() = delete;
};