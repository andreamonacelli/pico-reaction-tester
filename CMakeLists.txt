cmake_minimum_required(VERSION 3.12)

# Import Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)


# Pull in FreeRTOS kernel path and import
set(ENV{FREERTOS_KERNEL_PATH} "${CMAKE_SOURCE_DIR}/FreeRTOS-Kernel")
include(FreeRTOS_Kernel_import.cmake)
add_compile_definitions(PLATFORM_NAME_FREERTOS)

# Project Definition
project(pico-practice C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if (PICO_SDK_VERSION_STRING VERSION_LESS "2.1.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 2.1.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# Include FreeRTOS header directories.
include_directories(
    ${CMAKE_SOURCE_DIR}  # This ensures FreeRTOSConfig.h is found
    ${CMAKE_SOURCE_DIR}/libmicroros/include
)

# Initialize the SDK
pico_sdk_init()

# Set compile options
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffunction-sections -fdata-sections")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffunction-sections -fdata-sections")

# Adding microROS
#link_directories(libmicroros)

# Adding modules
add_subdirectory(apps/reaction-tester)