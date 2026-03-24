cmake_minimum_required(VERSION 3.22)

# Project definition - ZMĚNA: jiný název pro JUCE plugin
project(ITHACA_PLAYER_JUCE VERSION 1.1.0)

# Set C++ standard to 17 (required for IthacaCore)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build configuration
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# =============================================================================
# INSTRUMENT INSTANCE CONFIGURATION
# =============================================================================
#
# Multiple Plugin Instances approach:
# Each instrument has its own plugin binary with unique name and config path.
# Example: IthacaPlayer-VintageV, IthacaPlayer-Rhodes, etc.
#
# Benefits:
# - DAW sees each instrument as separate plugin
# - Multiple instruments can be used simultaneously in one session
# - Separate presets and configs per instrument
# - No GUI changes needed
#
# Configuration:
# - INSTRUMENT_NAME: Unique suffix for this build (e.g. "VintageV")
# - Each instrument gets its own config directory
# - Sample bank path is instrument-specific
# =============================================================================

# No instrument-specific builds - unified plugin with runtime sample bank selection

# =============================================================================
# PLUGIN CODE CONFIGURATION
# =============================================================================
#
# VST3/AU requirement: PLUGIN_CODE must be EXACTLY 4 characters
# Each plugin from same manufacturer (Lau0) must have unique code
#
# Naming convention: PLUGIN_CODE also serves as sample bank directory name
# Example: VntV → C:/SoundBanks/IthacaPlayer/VntV/
#
# Format: 4-character alphanumeric code (case-sensitive)
# =============================================================================

# Plugin Code - unified for all sample banks
set(PLUGIN_CODE "Itca")
set(PLUGIN_DESCRIPTION "Ithaca Player - Universal Sample Player")

# Plugin target name - unified
set(PLUGIN_TARGET_NAME "IthacaPlayer")

# Product name displayed in DAW and used for binary filenames (no spaces!)
set(PLUGIN_PRODUCT_NAME "IthacaPlayer")

message(STATUS "=== PLUGIN CONFIGURATION ===")
message(STATUS "Plugin Code:     ${PLUGIN_CODE}")
message(STATUS "Plugin Target:   ${PLUGIN_TARGET_NAME}")
message(STATUS "Product Name:    ${PLUGIN_PRODUCT_NAME}")
message(STATUS "Description:     ${PLUGIN_DESCRIPTION}")
message(STATUS "============================")

# =============================================================================
# SAMPLE BANK PATH CONFIGURATION
# =============================================================================

# Default sample bank path - uses PLUGIN_CODE for consistency
# Structure: C:/SoundBanks/IthacaPlayer/<PLUGIN_CODE>/
#
# Example paths:
#   VintageV  → C:/SoundBanks/IthacaPlayer/VntV/
#   Rhodes    → C:/SoundBanks/IthacaPlayer/Rhds/
#   Wurlitzer → C:/SoundBanks/IthacaPlayer/Wrlz/
#
# Each directory contains:
#   - instrument-definition.json
#   - WAV files (21_1.wav, 21_2.wav, ..., 108_8.wav)
if(WIN32)
    set(SAMPLE_BANK_PATH "C:/SoundBanks/IthacaPlayer/${PLUGIN_CODE}" CACHE STRING "Default sample bank installation path")
elseif(APPLE)
    set(SAMPLE_BANK_PATH "$ENV{HOME}/Library/Application Support/IthacaPlayer/${PLUGIN_CODE}" CACHE STRING "Default sample bank installation path")
else()
    set(SAMPLE_BANK_PATH "$ENV{HOME}/.local/share/IthacaPlayer/${PLUGIN_CODE}" CACHE STRING "Default sample bank installation path")
endif()

message(STATUS "Sample Bank Path: ${SAMPLE_BANK_PATH}")

# =============================================================================
# BUILD ID GENERATION - Automatická generace unikátního BUILD ID
# =============================================================================

# Získat timestamp buildu (lokální čas: YYMMDDHHMMSS)
string(TIMESTAMP BUILD_TIMESTAMP "%y%m%d%H%M%S")

# Získat hostname (počítač kde probíhá build)
cmake_host_system_information(RESULT BUILD_HOST QUERY HOSTNAME)

# Získat Git informace (pokud je projekt v Git repozitáři)
find_package(Git QUIET)
if(GIT_FOUND)
    # Git hash (zkrácený - prvních 7 znaků)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    
    # Git branch
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_BRANCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
else()
    set(GIT_HASH "nogit")
    set(GIT_BRANCH "unknown")
endif()

# Sestavit BUILD ID - vyberte preferovaný formát:

# VARIANTA 1: Timestamp (Hostname) - formát: YYMMDDHHMMSS (Hostname)
set(BUILD_ID "${BUILD_TIMESTAMP} (${BUILD_HOST})")

# VARIANTA 2: Git Hash + Timestamp (pro Git tracking)
# set(BUILD_ID "${GIT_HASH}_${BUILD_TIMESTAMP}")

# VARIANTA 3: Kompletní (maximum info - dlouhé)
# set(BUILD_ID "${GIT_HASH}_${GIT_BRANCH}_${BUILD_TIMESTAMP}_${BUILD_HOST}")

# VARIANTA 4: Pouze Git hash (minimální, vyžaduje Git)
# set(BUILD_ID "${GIT_HASH}")

# Výpis BUILD ID do konzole při konfiguraci
message(STATUS "=== BUILD ID GENERATION ===")
message(STATUS "Timestamp: ${BUILD_TIMESTAMP}")
message(STATUS "Host: ${BUILD_HOST}")
message(STATUS "Git Hash: ${GIT_HASH}")
message(STATUS "Git Branch: ${GIT_BRANCH}")
message(STATUS "Final BUILD_ID: ${BUILD_ID}")
message(STATUS "===========================")

# Vytvořit adresář pro generované soubory
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/generated)

# Vygenerovat header soubor s BUILD ID z template
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/ithaca/config/BuildID.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/generated/BuildID.h
    @ONLY
)

# =============================================================================
# JUCE a knihovny
# =============================================================================

# KRITICKÉ: Add JUCE subdirectory PŘED plugin definicí
add_subdirectory(JUCE)

# Add libsndfile for IthacaCore
add_subdirectory(libsndfile)

# =============================================================================
# JUCE Plugin definice - MULTIPLE INSTRUMENT INSTANCES
# =============================================================================
#
# Plugin je definován s instrument-specific názvem.
# Target name: IthacaPlayer-<InstrumentName> (e.g. IthacaPlayer-VintageV)
# Product name: IthacaPlayer <InstrumentName> (zobrazí se v DAW)
# Plugin Code: 4-char code (e.g. VntV, Rhds, Wrlz)
#
# Každý instrument má:
# - Vlastní plugin binary: IthacaPlayer-VintageV.vst3
# - Vlastní config adresář: %APPDATA%/LordAudio/IthacaPlayer-VintageV/
# - Vlastní sample bank: C:/SoundBanks/IthacaPlayer/VntV/
#
# Plugin Code registry (LordAudio/Lau0):
#   VntV - VintageV Electric Piano
#   Rhds - Rhodes Mark II Electric Piano
#   Wrlz - Wurlitzer 200A Electric Piano
#   Pian - Acoustic Piano
#   Itca - Legacy/fallback code
# =============================================================================

# KLÍČOVÁ ZMĚNA: juce_add_plugin s instrument-specific target name and code
juce_add_plugin(${PLUGIN_TARGET_NAME}
    COMPANY_NAME "LordAudio"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    PLUGIN_MANUFACTURER_CODE Lau0
    PLUGIN_CODE ${PLUGIN_CODE}
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "${PLUGIN_PRODUCT_NAME}")

    # Custom icon (white square with "I" letter placeholder)
    # ICON_BIG "${CMAKE_CURRENT_SOURCE_DIR}/decorators/icon.ico"
    # ICON_SMALL "${CMAKE_CURRENT_SOURCE_DIR}/decorators/icon.icns")                

# Generate JUCE header
juce_generate_juce_header(${PLUGIN_TARGET_NAME})

# SPRÁVNĚ: target_sources místo add_executable
target_sources(${PLUGIN_TARGET_NAME}

    PRIVATE
        # =====================================================================
        # ITHACA PLUGIN - Structured Source Files
        # =====================================================================

        # Configuration
        ithaca/config/IthacaConfig.h
        ithaca/config/AppConstants.h

        # Audio Processing & Sample Loading
        ithaca/audio/IthacaPluginProcessor.h
        ithaca/audio/IthacaPluginProcessor.cpp
        ithaca/audio/AsyncSampleLoader.h
        ithaca/audio/AsyncSampleLoader.cpp
        ithaca/audio/InstrumentMetadata.h
        ithaca/audio/InstrumentMetadata.cpp
        ithaca/audio/PerformanceMonitor.h
        ithaca/audio/PerformanceMonitor.cpp
        ithaca/audio/PluginStateManager.h
        ithaca/audio/PluginStateManager.cpp
        ithaca/audio/SampleBankPathManager.h
        ithaca/audio/SampleBankPathManager.cpp

        # Parameters
        ithaca/parameters/ParameterManager.h
        ithaca/parameters/ParameterManager.cpp
        ithaca/parameters/ParameterAttachmentManager.h
        ithaca/parameters/ParameterAttachmentManager.cpp

        # MIDI
        ithaca/midi/MidiHelpers.h
        ithaca/midi/MidiProcessor.h
        ithaca/midi/MidiProcessor.cpp
        ithaca/midi/MidiLearnManager.h
        ithaca/midi/MidiLearnManager.cpp

        # GUI - Main Editor
        ithaca/gui/IthacaPluginEditor.h
        ithaca/gui/IthacaPluginEditor.cpp

        # GUI - Components
        ithaca/gui/components/InfoHeaderComponent.h
        ithaca/gui/components/InfoHeaderComponent.cpp
        ithaca/gui/components/SliderPanelComponent.h
        ithaca/gui/components/SliderPanelComponent.cpp
        ithaca/gui/components/SampleBankSelectorComponent.h
        ithaca/gui/components/SampleBankSelectorComponent.cpp
        ithaca/gui/components/MidiLearnSlider.h

        # GUI - Factories
        ithaca/gui/factories/SliderFactory.h
        ithaca/gui/factories/SliderFactory.cpp
        ithaca/gui/factories/InfoHeaderLabelFactory.h
        ithaca/gui/factories/InfoHeaderLabelFactory.cpp

        # GUI - Layout
        ithaca/gui/layout/InfoHeaderLayout.h
        ithaca/gui/layout/InfoHeaderLayout.cpp

        # GUI - Helpers
        ithaca/gui/helpers/GuiHelpers.h
        ithaca/gui/helpers/GuiHelpers.cpp
        
        # IthacaCore - Core Logger module
        ithaca-core/sampler/core_logger.cpp
        ithaca-core/sampler/core_logger.h
        
        # IthacaCore - SamplerIO module  
        ithaca-core/sampler/sampler_io.cpp
        
        # IthacaCore - Sampler coordinator module
        ithaca-core/sampler/sampler.cpp
        ithaca-core/sampler/sampler.h
        
        # IthacaCore - InstrumentLoader module
        ithaca-core/sampler/instrument_loader.cpp
        ithaca-core/sampler/instrument_loader.h

        # IthacaCore - Sine Wave Generator (fallback audio)
        ithaca-core/sampler/sine_wave_generator.cpp
        ithaca-core/sampler/sine_wave_generator.h

        # IthacaCore - Wav exporter
        ithaca-core/sampler/wav_file_exporter.cpp
        ithaca-core/sampler/wav_file_exporter.h

        # IthacaCore - Voice module
        ithaca-core/sampler/voice.cpp
        ithaca-core/sampler/voice_processing.cpp
        ithaca-core/sampler/voice.h
        
        # IthacaCore - VoiceManager module
        ithaca-core/sampler/voice_manager.cpp
        ithaca-core/sampler/voice_manager.h

        # IthacaCore - Envelopes (ADSR/ASR)
        ithaca-core/sampler/envelopes/envelope.cpp
        ithaca-core/sampler/envelopes/envelope.h
        ithaca-core/sampler/envelopes/envelope_static_data.cpp
        ithaca-core/sampler/envelopes/envelope_static_data.h

        # IthacaCore - Pan
        ithaca-core/sampler/pan.cpp
        ithaca-core/sampler/pan.h
        ithaca-core/sampler/lfopan.cpp
        ithaca-core/sampler/lfopan.h

        # IthacaCore - DSP Effects
        ithaca-core/dsp/dsp_effect.h
        ithaca-core/dsp/dsp_chain.h
        ithaca-core/dsp/dsp_chain.cpp
        ithaca-core/dsp/bbe/bbe_processor.h
        ithaca-core/dsp/bbe/bbe_processor.cpp
        ithaca-core/dsp/bbe/biquad_filter.h
        ithaca-core/dsp/bbe/biquad_filter.cpp
        ithaca-core/dsp/bbe/harmonic_enhancer.h
        ithaca-core/dsp/bbe/harmonic_enhancer.cpp
        ithaca-core/dsp/limiter/limiter.h
        ithaca-core/dsp/limiter/limiter.cpp

        # IthacaCore - Tests (optional - can be excluded for release)
        ithaca-core/sampler/tests/test_helpers.cpp
        ithaca-core/sampler/tests/test_helpers.h
        ithaca-core/sampler/tests/tests.cpp
        ithaca-core/sampler/tests/tests.h
)

# =============================================================================
# Include directories
# =============================================================================

target_include_directories(${PLUGIN_TARGET_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}                             # Root directory - IthacaConfig.h zde
    ${CMAKE_CURRENT_SOURCE_DIR}/ithaca-core                 # IthacaCore modules
    ${CMAKE_CURRENT_SOURCE_DIR}/ithaca-core/sampler         # IthacaCore sampler modules
    ${CMAKE_CURRENT_SOURCE_DIR}/ithaca-core/sampler/tests   # Test framework
    ${CMAKE_CURRENT_SOURCE_DIR}/json/include                # nlohmann/json library
    ${CMAKE_CURRENT_BINARY_DIR}/generated                   # PŘIDÁNO: Pro BuildID.h
)

# =============================================================================
# Compile definitions
# =============================================================================

# JUCE compile definitions
target_compile_definitions(${PLUGIN_TARGET_NAME}
    PUBLIC
        # Standard JUCE settings for audio plugin
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0

        # IthacaCore specific settings
        ENABLE_TESTS=1              # Enable test framework integration
        ITHACA_JUCE_INTEGRATION=1   # Enable JUCE-specific features

        # MULTIPLE INSTRUMENT INSTANCES support
        # Define plugin target name for config directory lookup
        # Example: "IthacaPlayer-VintageV"
        ITHACA_PLUGIN_TARGET_NAME="${PLUGIN_TARGET_NAME}"

        # Define plugin code for config filename generation
        # Example: "VntV" → samplebank_config-VntV.json
        ITHACA_PLUGIN_CODE=${PLUGIN_CODE}

        # Resource paths for development
        ITHACA_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
        ITHACA_BINARY_DIR="${CMAKE_CURRENT_BINARY_DIR}"

    PRIVATE
        # Platform-specific definitions
        $<$<PLATFORM_ID:Windows>:WIN32_LEAN_AND_MEAN>
        $<$<PLATFORM_ID:Windows>:_CRT_SECURE_NO_WARNINGS>
)

# =============================================================================
# Compiler options
# =============================================================================

# Compiler-specific optimizations
if(MSVC)
    target_compile_options(${PLUGIN_TARGET_NAME} PRIVATE
        $<$<CONFIG:Debug>:/Od /Zi /RTC1>      # Debug: no optimization, debug info
        $<$<CONFIG:Release>:/O2 /DNDEBUG>     # Release: optimization, no debug
        /W3                                   # Warning level 3
    )
else()
    target_compile_options(${PLUGIN_TARGET_NAME} PRIVATE
        -Wall -Wextra -pedantic               # All warnings
        $<$<CONFIG:Debug>:-g -O0>             # Debug: debug info, no optimization  
        $<$<CONFIG:Release>:-O3 -DNDEBUG>     # Release: maximum optimization
    )
endif()

# =============================================================================
# Link libraries
# =============================================================================

# KRITICKÉ: Link libraries - JUCE first, then IthacaCore dependencies
target_link_libraries(${PLUGIN_TARGET_NAME}
    PRIVATE
        # JUCE modules - MUSÍ BÝT PRVNÍ
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_gui_basics
        
        # IthacaCore dependencies
        sndfile                         # libsndfile for WAV operations
        
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# =============================================================================
# Install decorators (background.jpg) to user roaming directory
# =============================================================================

# POST_BUILD: Install decorators to user's roaming directory
# This ensures decorators are in the same location as samplebank config and core_logger
add_custom_command(TARGET ${PLUGIN_TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_IMAGE=${CMAKE_CURRENT_SOURCE_DIR}/decorators/background.jpg
        -DPLUGIN_NAME=${PLUGIN_TARGET_NAME}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/InstallDecorators.cmake
    COMMENT "Installing decorators for ${PLUGIN_TARGET_NAME} to user's roaming profile"
    VERBATIM
)

# Also keep copies in Standalone executable location for development fallback
# (Development builds will check user roaming first, then fall back to these)
add_custom_command(TARGET ${PLUGIN_TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        ${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_TARGET_NAME}_artefacts/Debug/Standalone/decorators
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/decorators/background.jpg
        ${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_TARGET_NAME}_artefacts/Debug/Standalone/decorators/background.jpg
    COMMENT "Copying background.jpg to Standalone Debug location (development fallback)"
)

add_custom_command(TARGET ${PLUGIN_TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        ${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_TARGET_NAME}_artefacts/Release/Standalone/decorators
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/decorators/background.jpg
        ${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_TARGET_NAME}_artefacts/Release/Standalone/decorators/background.jpg
    COMMENT "Copying background.jpg to Standalone Release location (development fallback)"
)

# =============================================================================
# Platform-specific settings
# =============================================================================

if(WIN32)
    # Windows-specific installation
    install(TARGETS ${PLUGIN_TARGET_NAME}
        RUNTIME DESTINATION bin
        CONFIGURATIONS Release
    )
    
    # Copy libsndfile DLL on Windows
    install(FILES $<TARGET_FILE:sndfile>
        DESTINATION bin
        CONFIGURATIONS Release
    )
    
    # Install background image
    install(FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/decorators/background.jpg
        DESTINATION bin/decorators
        CONFIGURATIONS Release
    )
endif()

# =============================================================================
# Build information output
# =============================================================================

# Output information
message(STATUS "=== IthacaCore + JUCE Plugin Build Configuration ===")
message(STATUS "Project: ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "Description: IthacaCore Professional Sampler with JUCE Integration")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Plugin formats: AU VST3 Standalone")
message(STATUS "Audio engine: IthacaCore VoiceManager")
message(STATUS "GUI framework: JUCE")
message(STATUS "Audio I/O: libsndfile")
message(STATUS "Testing: ${ENABLE_TESTS}")
message(STATUS "====================================================")

# =============================================================================
# Development targets
# =============================================================================

# Development targets for IthacaCore compatibility
add_custom_target(clean-logs
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_SOURCE_DIR}/core_logger
    COMMENT "Cleaning IthacaCore log files"
)

add_custom_target(clean-exports
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_SOURCE_DIR}/exports
    COMMENT "Cleaning IthacaCore test exports"
)

add_custom_target(clean-all-ithaca
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_SOURCE_DIR}/core_logger
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_SOURCE_DIR}/exports
    COMMENT "Cleaning all IthacaCore logs and exports"
)

# =============================================================================
# Debug build information
# =============================================================================

# Debug target information
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "=== Debug Build Information ===")
    message(STATUS "IthacaCore logging: ENABLED")
    message(STATUS "JUCE assertions: ENABLED")
    message(STATUS "Runtime checks: ENABLED")
    message(STATUS "Optimization: DISABLED")
    message(STATUS "BUILD_ID: ${BUILD_ID}")
    message(STATUS "Available targets:")
    message(STATUS "  - IthacaPlayer: Build JUCE plugin")
    message(STATUS "  - clean-logs: Remove IthacaCore logs")
    message(STATUS "  - clean-exports: Remove test exports")
    message(STATUS "  - clean-all-ithaca: Clean all IthacaCore data")
    message(STATUS "==============================")
endif()

# =============================================================================
# Post-build information
# =============================================================================

# Post-build information s BUILD ID
add_custom_command(TARGET ${PLUGIN_TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "=== IthacaCore + JUCE Plugin Build Completed ==="
    COMMAND ${CMAKE_COMMAND} -E echo "Plugin: $<TARGET_FILE:${PLUGIN_TARGET_NAME}>"
    COMMAND ${CMAKE_COMMAND} -E echo "Build ID: ${BUILD_ID}"
    COMMAND ${CMAKE_COMMAND} -E echo "Build Time: ${BUILD_TIMESTAMP}"
    COMMAND ${CMAKE_COMMAND} -E echo "Build Host: ${BUILD_HOST}"
    COMMAND ${CMAKE_COMMAND} -E echo "Git Hash: ${GIT_HASH}"
    COMMAND ${CMAKE_COMMAND} -E echo "Git Branch: ${GIT_BRANCH}"
    COMMAND ${CMAKE_COMMAND} -E echo "Audio Engine: IthacaCore VoiceManager (128 voices)"
    COMMAND ${CMAKE_COMMAND} -E echo "Formats: AU, VST3, Standalone"
    COMMAND ${CMAKE_COMMAND} -E echo "Sample directory: Check IthacaConfig.h for DEFAULT_SAMPLE_DIR"
    COMMAND ${CMAKE_COMMAND} -E echo "Logs: ./core_logger/core_logger.log"
    COMMAND ${CMAKE_COMMAND} -E echo "================================================="
)# =============================================================================
# BUILD ID GENERATION - Přidat před juce_add_plugin
# =============================================================================

# Získat timestamp buildu (lokální čas: YYMMDDHHMMSS)
string(TIMESTAMP BUILD_TIMESTAMP "%y%m%d%H%M%S")

# Získat hostname (počítač kde probíhá build)
cmake_host_system_information(RESULT BUILD_HOST QUERY HOSTNAME)

# Získat Git hash (pokud je projekt v Git repozitáři)
find_package(Git QUIET)
if(GIT_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_BRANCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
else()
    set(GIT_HASH "nogit")
    set(GIT_BRANCH "unknown")
endif()

# Sestavit BUILD ID (vyberte preferovaný formát)

# VARIANTA 1: Timestamp (Hostname) - formát: YYMMDDHHMMSS (Hostname)
set(BUILD_ID "${BUILD_TIMESTAMP} (${BUILD_HOST})")

# VARIANTA 2: Git Hash + Timestamp (pro Git tracking)
# set(BUILD_ID "${GIT_HASH}_${BUILD_TIMESTAMP}")

# VARIANTA 3: Kompletní (maximum info)
# set(BUILD_ID "${GIT_HASH}_${GIT_BRANCH}_${BUILD_TIMESTAMP}_${BUILD_HOST}")

# VARIANTA 4: Pouze Git hash (minimální, pro Git workflow)
# set(BUILD_ID "${GIT_HASH}")

# Výpis BUILD ID do konzole
message(STATUS "=== BUILD ID GENERATION ===")
message(STATUS "Timestamp: ${BUILD_TIMESTAMP}")
message(STATUS "Host: ${BUILD_HOST}")
message(STATUS "Git Hash: ${GIT_HASH}")
message(STATUS "Git Branch: ${GIT_BRANCH}")
message(STATUS "Final BUILD_ID: ${BUILD_ID}")
message(STATUS "===========================")

# Přidat do include paths
target_include_directories(${PLUGIN_TARGET_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/ithaca-core
    ${CMAKE_CURRENT_SOURCE_DIR}/ithaca-core/sampler
    ${CMAKE_CURRENT_SOURCE_DIR}/ithaca-core/sampler/tests
    ${CMAKE_CURRENT_BINARY_DIR}/generated  # PŘIDÁNO: Pro BuildID.h
)