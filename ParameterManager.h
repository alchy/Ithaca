#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>
#include <functional>

/**
 * @class ParameterManager
 * @brief Specialized class for JUCE parameter management and state persistence
 * 
 * RESPONSIBILITIES:
 * - JUCE ValueTreeState management and lifecycle
 * - Parameter layout creation with modern JUCE API
 * - Thread-safe parameter value access for RT operations
 * - Plugin state save/load operations
 * - Parameter change notifications for GUI updates
 * - Parameter validation and range clamping
 * 
 * DESIGN NOTES:
 * - Uses modern JUCE AudioParameterFloat with ParameterID
 * - Thread-safe parameter access via atomic pointers
 * - Centralized parameter definitions with compile-time constants
 * - RT-safe getter methods for audio thread usage
 * - Clean separation between parameter definition and access
 * 
 * PARAMETER ARCHITECTURE:
 * - Master Gain: 0-127 MIDI range (matches IthacaCore expectations)
 * - Master Pan: -64 to +63 range (center = 0, compatible with MIDI CC)
 * - All parameters use float internally but expose MIDI-compatible ranges
 * - Automatic clamping and validation for all parameter access
 */
class ParameterManager
{
public:
    //==============================================================================
    // Parameter IDs (compile-time constants for consistency)
    static constexpr const char* MASTER_GAIN_ID = "masterGain";
    static constexpr const char* MASTER_PAN_ID = "masterPan";
    
    // Parameter ranges (matching IthacaCore and MIDI standards)
    static constexpr float GAIN_MIN = 0.0f;     // Silent
    static constexpr float GAIN_MAX = 127.0f;   // Maximum MIDI gain
    static constexpr float GAIN_DEFAULT = 100.0f; // ~80% gain as default
    
    static constexpr float PAN_MIN = -64.0f;    // Full left
    static constexpr float PAN_MAX = 63.0f;     // Full right  
    static constexpr float PAN_DEFAULT = 0.0f;  // Center
    
    // Step sizes for parameter automation
    static constexpr float GAIN_STEP = 1.0f;    // Integer steps for MIDI compatibility
    static constexpr float PAN_STEP = 1.0f;     // Integer steps for MIDI compatibility

    //==============================================================================
    // Constructor & Destructor
    
    /**
     * @brief Constructor - initializes JUCE parameter system
     * Creates ValueTreeState with parameter layout and initializes atomic pointers
     */
    explicit ParameterManager();
    
    /**
     * @brief Destructor - default cleanup is sufficient
     */
    ~ParameterManager() = default;

    //==============================================================================
    // JUCE Integration Interface
    
    /**
     * @brief Get JUCE ValueTreeState for GUI binding
     * @return Reference to parameter state for automation and GUI
     * @note Use this for AttachmentBase-derived GUI controls
     */
    juce::AudioProcessorValueTreeState& getValueTreeState() { return parameters_; }
    
    /**
     * @brief Get const JUCE ValueTreeState for read-only access
     * @return Const reference to parameter state
     * @note Use this for read-only operations and state inspection
     */
    const juce::AudioProcessorValueTreeState& getValueTreeState() const { return parameters_; }

    //==============================================================================
    // RT-Safe Parameter Value Access
    
    /**
     * @brief RT-SAFE: Get current master gain as MIDI value (0-127)
     * @return Master gain MIDI value, clamped to valid range
     * @note Safe to call from audio thread - uses atomic load
     */
    uint8_t getMasterGainMIDI() const noexcept;
    
    /**
     * @brief RT-SAFE: Get current master gain as normalized float (0.0-1.0)
     * @return Normalized master gain for linear processing
     * @note Safe to call from audio thread - computed from MIDI value
     */
    float getMasterGainNormalized() const noexcept;
    
    /**
     * @brief RT-SAFE: Get current master pan as MIDI value (0-127, center=64)
     * @return Master pan MIDI value, clamped and offset for MIDI compatibility
     * @note Safe to call from audio thread - uses atomic load with offset
     */
    uint8_t getMasterPanMIDI() const noexcept;
    
    /**
     * @brief RT-SAFE: Get current master pan as normalized float (-1.0 to 1.0)
     * @return Normalized master pan (-1.0=left, 0.0=center, 1.0=right)
     * @note Safe to call from audio thread - computed from internal value
     */
    float getMasterPanNormalized() const noexcept;

    //==============================================================================
    // Parameter Validation and Utilities
    
    /**
     * @brief Check if master gain parameter is available
     * @return true if parameter is properly initialized
     */
    bool isGainParameterValid() const noexcept { return masterGainParam_ != nullptr; }
    
    /**
     * @brief Check if master pan parameter is available  
     * @return true if parameter is properly initialized
     */
    bool isPanParameterValid() const noexcept { return masterPanParam_ != nullptr; }
    
    /**
     * @brief Get current parameter count
     * @return Number of parameters in this manager
     */
    int getParameterCount() const noexcept { return 2; } // gain + pan

    //==============================================================================
    // State Management
    
    /**
     * @brief Save parameter state to binary data
     * @param destData Target memory block for serialized state
     * @note Non-RT operation - use from GUI thread or state save callbacks
     */
    void saveState(juce::MemoryBlock& destData);
    
    /**
     * @brief Load parameter state from binary data
     * @param data Source data pointer
     * @param sizeInBytes Size of source data in bytes
     * @note Non-RT operation - use from GUI thread or state load callbacks
     */
    void loadState(const void* data, int sizeInBytes);
    
    /**
     * @brief Reset all parameters to default values
     * @note Non-RT operation - triggers parameter change notifications
     */
    void resetToDefaults();

    //==============================================================================
    // Parameter Change Notifications
    
    /**
     * @brief Set callback for parameter changes (useful for GUI updates)
     * @param callback Function to call when parameters change
     * @note Callback is called from the message thread, not audio thread
     */
    void setParameterChangeCallback(std::function<void(const juce::String&)> callback);
    
    /**
     * @brief Clear parameter change callback
     */
    void clearParameterChangeCallback() { parameterChangeCallback_ = nullptr; }

private:
    //==============================================================================
    // JUCE Parameter System
    
    /**
     * @brief Main JUCE parameter state container
     * Manages all parameters, automation, and state persistence
     */
    juce::AudioProcessorValueTreeState parameters_;
    
    // Thread-safe parameter pointers (set once during construction)
    std::atomic<float>* masterGainParam_;   ///< Atomic pointer to gain parameter
    std::atomic<float>* masterPanParam_;    ///< Atomic pointer to pan parameter
    
    // Parameter change notification system
    std::function<void(const juce::String&)> parameterChangeCallback_;

    //==============================================================================
    // Private Initialization Methods
    
    /**
     * @brief Create parameter layout for JUCE ValueTreeState
     * @return Complete parameter layout with all plugin parameters
     * @note Called once during construction to set up parameter structure
     */
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    /**
     * @brief Initialize atomic parameter pointers after layout creation
     * @note Called after parameter layout is created and ValueTreeState is constructed
     */
    void initializeParameterPointers();

    //==============================================================================
    // Private Utility Methods
    
    /**
     * @brief Clamp MIDI value to valid range (0-127)
     * @param value Input floating-point value
     * @return Clamped value as uint8_t in MIDI range
     * @note Static method for consistent clamping behavior
     */
    static uint8_t clampMIDIValue(float value) noexcept;
    
    /**
     * @brief Clamp pan value to valid range (-64 to +63)
     * @param value Input floating-point pan value
     * @return Clamped pan value
     * @note Static method for consistent pan range enforcement
     */
    static float clampPanValue(float value) noexcept;
    
    /**
     * @brief Convert internal pan value to MIDI format
     * @param internalPan Pan value in internal range (-64 to +63)
     * @return MIDI pan value (0-127, center=64)
     */
    static uint8_t panToMIDI(float internalPan) noexcept;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ParameterManager)
};