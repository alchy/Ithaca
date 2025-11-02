# CMake script to install decorators (background.jpg) to user's roaming profile
# This script is executed as a POST_BUILD step
#
# MULTIPLE INSTRUMENT INSTANCES support:
# Each instrument instance gets its own decorators directory
# Example: IthacaPlayer-VintageV → %APPDATA%/LordAudio/IthacaPlayer-VintageV/decorators/

cmake_minimum_required(VERSION 3.22)

set(COMPANY_NAME "LordAudio")

# PLUGIN_NAME is passed from CMakeLists.txt via -DPLUGIN_NAME=...
# It includes the instrument suffix (e.g., "IthacaPlayer-VintageV")
if(NOT DEFINED PLUGIN_NAME)
    message(FATAL_ERROR "PLUGIN_NAME must be defined via -DPLUGIN_NAME=<name>")
endif()

# SOURCE_IMAGE is passed from CMakeLists.txt via -DSOURCE_IMAGE=...
if(NOT DEFINED SOURCE_IMAGE)
    message(FATAL_ERROR "SOURCE_IMAGE must be defined via -DSOURCE_IMAGE=<path>")
endif()

message(STATUS "=== Installing Decorators ==")
message(STATUS "Plugin Name: ${PLUGIN_NAME}")
message(STATUS "Source Image: ${SOURCE_IMAGE}")

# Determine platform-specific roaming directory
if(WIN32)
    # Windows: %APPDATA%\\LordAudio\\IthacaPlayer-VintageV
    file(TO_CMAKE_PATH "$ENV{APPDATA}" ROAMING_DIR)
    set(PLUGIN_DATA_DIR "${ROAMING_DIR}/${COMPANY_NAME}/${PLUGIN_NAME}")

elseif(APPLE)
    # macOS: ~/Library/Application Support/LordAudio/IthacaPlayer-VintageV
    file(TO_CMAKE_PATH "$ENV{HOME}/Library/Application Support" ROAMING_DIR)
    set(PLUGIN_DATA_DIR "${ROAMING_DIR}/${COMPANY_NAME}/${PLUGIN_NAME}")

else()
    # Linux: ~/.local/share/LordAudio/IthacaPlayer-VintageV
    file(TO_CMAKE_PATH "$ENV{HOME}/.local/share" ROAMING_DIR)
    set(PLUGIN_DATA_DIR "${ROAMING_DIR}/${COMPANY_NAME}/${PLUGIN_NAME}")
endif()

set(DECORATORS_DIR "${PLUGIN_DATA_DIR}/decorators")

message(STATUS "Target Directory: ${DECORATORS_DIR}")

# Create decorators directory if it doesn't exist
file(MAKE_DIRECTORY "${DECORATORS_DIR}")

# Copy background.jpg to decorators directory (always overwrite to update with new builds)
set(TARGET_FILE "${DECORATORS_DIR}/background.jpg")
message(STATUS "Installing background.jpg to: ${TARGET_FILE}")

file(COPY "${SOURCE_IMAGE}" DESTINATION "${DECORATORS_DIR}")
message(STATUS "Decorators installed successfully")
