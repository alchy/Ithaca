#include "mixer.h"
#include <algorithm>
#include <cmath>

/**
 * @brief Konstruktor s výchozími parametry optimalizovanými pro hudební aplikace
 */
Mixer::Mixer()
    : lowEnergyThreshold_(0.1f),       // Ochrana při nízké energii
      saturationThreshold_(0.9f),      // Soft saturation hranice
      gainReductionStrength_(1.0f),    // Plná síla gain redukce
      lastTotalEnergy_(0.0f),
      lastAdaptiveGain_(1.0f),
      lastActiveVoicesCount_(0) {
}

/**
 * @brief RT-SAFE: Hlavní mixovací metoda s energy-based gain sharing
 * 
 * Tato metoda kombinuje audio z více voices do stereo výstupu.
 * Pro každý sample:
 * - Spočítá celkovou energii a zároveň mixuje signály v jednom loopu přes voices (optimalizace výkonu).
 * - Aplikuje adaptivní gain a soft saturation.
 * 
 * @param voices Vector s VoiceData strukturami (audio + energy data)
 * @param outputLeft Výstupní buffer pro levý kanál 
 * @param outputRight Výstupní buffer pro pravý kanál
 * @param numSamples Počet samples k zpracování
 * @return true pokud byl vyproducen audio output, false pro silence
 */
bool Mixer::mixVoices(const std::vector<VoiceData>& voices, 
                      float* outputLeft, float* outputRight, int numSamples) noexcept {
    
    // Validace parametrů
    if (!outputLeft || !outputRight || numSamples <= 0) {
        return false;
    }
    
    // Vyčištění output bufferů
    std::fill(outputLeft, outputLeft + numSamples, 0.0f);
    std::fill(outputRight, outputRight + numSamples, 0.0f);
    
    // Počítání aktivních voices
    int activeCount = 0;
    for (const auto& voice : voices) {
        if (voice.isActive && voice.audioLeft && voice.audioRight && voice.energyBuffer) {
            activeCount++;
        }
    }
    
    // Early exit pokud nejsou žádné aktivní voices
    if (activeCount == 0) {
        lastActiveVoicesCount_ = 0;
        lastTotalEnergy_ = 0.0f;
        lastAdaptiveGain_ = 1.0f;
        return false;
    }
    
    lastActiveVoicesCount_ = activeCount;
    bool anyOutput = false;
    
    // HLAVNÍ MIXOVACÍ LOOP - sample po sample processing
    for (int sampleIdx = 0; sampleIdx < numSamples; ++sampleIdx) {
        
        // Kombinovaná fáze: Výpočet celkové energie a mixing signálů v jednom loopu
        float totalEnergy = 0.0f;
        float mixedLeft = 0.0f;
        float mixedRight = 0.0f;
        
        for (const auto& voice : voices) {
            if (voice.isActive && voice.audioLeft && voice.audioRight && voice.energyBuffer) {
                totalEnergy += voice.energyBuffer[sampleIdx];
                mixedLeft += voice.audioLeft[sampleIdx];
                mixedRight += voice.audioRight[sampleIdx];
            }
        }
        
        // Výpočet adaptivního gain na základě energie
        const float adaptiveGain = calculateAdaptiveGain(totalEnergy);
        
        // Aplikace adaptivního gain a soft saturation
        mixedLeft *= adaptiveGain;
        mixedRight *= adaptiveGain;
        
        outputLeft[sampleIdx] = applySoftSaturation(mixedLeft);
        outputRight[sampleIdx] = applySoftSaturation(mixedRight);
        
        // Detekce output aktivity
        if (std::abs(outputLeft[sampleIdx]) > 0.0001f || std::abs(outputRight[sampleIdx]) > 0.0001f) {
            anyOutput = true;
        }
    }
    
    // Update monitoring hodnot (poslední sample v bloku)
    if (numSamples > 0) {
        float lastSampleEnergy = 0.0f;
        for (const auto& voice : voices) {
            if (voice.isActive && voice.energyBuffer) {
                lastSampleEnergy += voice.energyBuffer[numSamples - 1];
            }
        }
        lastTotalEnergy_ = lastSampleEnergy;
        lastAdaptiveGain_ = calculateAdaptiveGain(lastSampleEnergy);
    }
    
    return anyOutput;
}

/**
 * @brief RT-SAFE: Energy-based adaptivní gain algoritmus
 * 
 * Implementuje gain-sharing princip s odmocninovou redukcí:
 * - Nízká energie: žádná redukce (zachování dynamics)
 * - Vysoká energie: progresivní redukce (prevence clipping)
 * 
 * @param totalEnergy Celková energie všech voices v daném sample
 * @return Adaptivní gain hodnota (0.0-1.0)
 */
float Mixer::calculateAdaptiveGain(float totalEnergy) const noexcept {
    // Ochrana při nízké energii - žádná zbytečná redukce
    if (totalEnergy <= lowEnergyThreshold_) {
        return 1.0f;
    }
    
    // Energy-based gain redukce s odmocninovou funkcí
    // Čím vyšší energie, tím více redukce, ale ne lineárně
    float baseGain = 1.0f / std::sqrt(totalEnergy);
    
    // Aplikace konfigurovatelné síly redukce
    float adaptiveGain = baseGain;
    if (gainReductionStrength_ != 1.0f) {
        // Interpolace mezi 1.0 (žádná redukce) a baseGain (plná redukce)
        adaptiveGain = 1.0f + gainReductionStrength_ * (baseGain - 1.0f);
    }
    
    // Clamp do rozumných mezí
    adaptiveGain = std::max(0.1f, std::min(1.0f, adaptiveGain));
    
    return adaptiveGain;
}

/**
 * @brief RT-SAFE: Soft saturation pro zabránění hard clipping
 * 
 * Používá tanh-based soft saturation která zachovává hudební charakter
 * signálu i při vyšších úrovních.
 * 
 * @param value Input hodnota
 * @return Saturated hodnota
 */
float Mixer::applySoftSaturation(float value) const noexcept {
    const float absValue = std::abs(value);
    
    // Pokud je hodnota pod prahem, vrať beze změny
    if (absValue <= saturationThreshold_) {
        return value;
    }
    
    // Soft saturation pouze pro hodnoty nad prahem
    const float sign = (value >= 0.0f) ? 1.0f : -1.0f;
    const float excess = absValue - saturationThreshold_;
    
    // Tanh saturation pro smooth compression
    const float saturatedExcess = std::tanh(excess * 2.0f) * 0.1f;
    const float result = sign * (saturationThreshold_ + saturatedExcess);
    
    // Final clamp jako bezpečnostní opatření
    return std::max(-1.0f, std::min(1.0f, result));
}