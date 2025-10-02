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

# KRITICKÉ: Add JUCE subdirectory PŘED plugin definicí
add_subdirectory(JUCE)

# Add libsndfile for IthacaCore
add_subdirectory(libsndfile)

# KLÍČOVÁ ZMĚNA: juce_add_plugin místo add_executable
juce_add_plugin(IthacaPlayer
    COMPANY_NAME "LordAudio"                   
    IS_SYNTH TRUE                               
    NEEDS_MIDI_INPUT TRUE                       
    NEEDS_MIDI_OUTPUT FALSE                     
    IS_MIDI_EFFECT FALSE                        
    PLUGIN_MANUFACTURER_CODE Lau0               
    PLUGIN_CODE Itca                            
    FORMATS AU VST3 Standalone                  
    PRODUCT_NAME "IthacaPlayer")                

# Generate JUCE header 
juce_generate_juce_header(IthacaPlayer)

# SPRÁVNĚ: target_sources místo add_executable
target_sources(IthacaPlayer

    PRIVATE
        # Configuration
        IthacaConfig.h
        
        # Binary data
        decorators/BinaryData.cpp
        decorators/BinaryData.h
        
        # JUCE - MIDI Control (Centralized)
        MidiCCDefinitions.h

        # JUCE - Plugin Framework (IthacaPluginProcessor)  
        IthacaPluginProcessor.cpp
        IthacaPluginProcessor.h

        # JUCE - Parameter Management (IthacaPluginProcessor)
        ParameterManager.h
        ParameterManager.cpp

        # JUCE - Plugin Framework (IthacaPluginEditor)
        IthacaPluginEditor.cpp  
        IthacaPluginEditor.h

        # JUCE - GUI Helper (IthacaPluginEditor)
        GuiConstants.h
        GuiHelpers.h
        GuiHelpers.cpp
        ParameterAttachmentManager.h
        ParameterAttachmentManager.cpp

        # JUCE - Specialized GUI Components (IthacaPluginEditor)
        SliderPanelComponent.h
        SliderPanelComponent.cpp
        InfoHeaderComponent.h
        InfoHeaderComponent.cpp

        # Asynchronni loader samplu
        AsyncSampleLoader.h
        AsyncSampleLoader.cpp

        # Midi processor
        MidiProcessor.h
        MidiProcessor.cpp
        
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

        # IthacaCore - Tests (optional - can be excluded for release)
        ithaca-core/sampler/tests/test_helpers.cpp
        ithaca-core/sampler/tests/test_helpers.h
        ithaca-core/sampler/tests/tests.cpp
        ithaca-core/sampler/tests/tests.h
)

# Include directories
target_include_directories(IthacaPlayer PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}                             # Root directory - IthacaConfig.h zde
    ${CMAKE_CURRENT_SOURCE_DIR}/ithaca-core                 # IthacaCore modules
    ${CMAKE_CURRENT_SOURCE_DIR}/ithaca-core/sampler         # IthacaCore sampler modules
    ${CMAKE_CURRENT_SOURCE_DIR}/ithaca-core/sampler/tests   # Test framework
)

# JUCE compile definitions
target_compile_definitions(IthacaPlayer
    PUBLIC
        # Standard JUCE settings for audio plugin
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        
        # IthacaCore specific settings
        ENABLE_TESTS=1              # Enable test framework integration
        ITHACA_JUCE_INTEGRATION=1   # Enable JUCE-specific features
        
    PRIVATE
        # Platform-specific definitions
        $<$<PLATFORM_ID:Windows>:WIN32_LEAN_AND_MEAN>
        $<$<PLATFORM_ID:Windows>:_CRT_SECURE_NO_WARNINGS>
)

# Compiler-specific optimizations
if(MSVC)
    target_compile_options(IthacaPlayer PRIVATE
        $<$<CONFIG:Debug>:/Od /Zi /RTC1>      # Debug: no optimization, debug info
        $<$<CONFIG:Release>:/O2 /DNDEBUG>     # Release: optimization, no debug
        /W3                                   # Warning level 3
    )
else()
    target_compile_options(IthacaPlayer PRIVATE
        -Wall -Wextra -pedantic               # All warnings
        $<$<CONFIG:Debug>:-g -O0>             # Debug: debug info, no optimization  
        $<$<CONFIG:Release>:-O3 -DNDEBUG>     # Release: maximum optimization
    )
endif()

# KRITICKÉ: Link libraries - JUCE first, then IthacaCore dependencies
target_link_libraries(IthacaPlayer
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

# Platform-specific settings
if(WIN32)
    # Windows-specific installation
    install(TARGETS IthacaPlayer
        RUNTIME DESTINATION bin
        CONFIGURATIONS Release
    )
    
    # Copy libsndfile DLL on Windows
    install(FILES $<TARGET_FILE:sndfile>
        DESTINATION bin
        CONFIGURATIONS Release
    )
endif()

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

# Debug target information
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "=== Debug Build Information ===")
    message(STATUS "IthacaCore logging: ENABLED")
    message(STATUS "JUCE assertions: ENABLED")
    message(STATUS "Runtime checks: ENABLED")
    message(STATUS "Optimization: DISABLED")
    message(STATUS "Available targets:")
    message(STATUS "  - IthacaPlayer: Build JUCE plugin")
    message(STATUS "  - clean-logs: Remove IthacaCore logs")
    message(STATUS "  - clean-exports: Remove test exports")
    message(STATUS "  - clean-all-ithaca: Clean all IthacaCore data")
    message(STATUS "==============================")
endif()

# Post-build information
add_custom_command(TARGET IthacaPlayer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "=== IthacaCore + JUCE Plugin Build Completed ==="
    COMMAND ${CMAKE_COMMAND} -E echo "Plugin: $<TARGET_FILE:IthacaPlayer>"
    COMMAND ${CMAKE_COMMAND} -E echo "Audio Engine: IthacaCore VoiceManager (128 voices)"
    COMMAND ${CMAKE_COMMAND} -E echo "Formats: AU, VST3, Standalone"
    COMMAND ${CMAKE_COMMAND} -E echo "Sample directory: Check IthacaConfig.h for DEFAULT_SAMPLE_DIR"
    COMMAND ${CMAKE_COMMAND} -E echo "Logs: ./core_logger/core_logger.log"
    COMMAND ${CMAKE_COMMAND} -E echo "================================================="
)
