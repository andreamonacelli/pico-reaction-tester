cmake_minimum_required(VERSION 3.12)

# Import Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# Initialize the SDK
pico_sdk_init()

# Project Definition
project(reaction-tester C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Pull in FreeRTOS kernel path and import
set(ENV{FREERTOS_KERNEL_PATH} "${CMAKE_SOURCE_DIR}/FreeRTOS-Kernel")
include(FreeRTOS_Kernel_import.cmake)
add_compile_definitions(PLATFORM_NAME_FREERTOS)

# Include FreeRTOS header directories.
include_directories(
    ${CMAKE_SOURCE_DIR}  # This ensures FreeRTOSConfig.h is found
    ${CMAKE_SOURCE_DIR}/libmicroros/include
)

# Set compile options
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -finline-functions -O2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -finline-functions -O2")

# Link to microROS
link_directories(libmicroros)

# Adding modules
add_subdirectory(apps/reaction-tester)