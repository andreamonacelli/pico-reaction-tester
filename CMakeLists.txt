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
)

# If you want debug output from USB (pass -DPICO_STDIO_USB=1) this ensures you don't lose any debug output while USB is set up
if (NOT DEFINED PICO_STDIO_USB_CONNECT_WAIT_TIMEOUT_MS)
    set(PICO_STDIO_USB_CONNECT_WAIT_TIMEOUT_MS 3000)
endif()

# Initialize the SDK
pico_sdk_init()

# Set compile options
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffunction-sections -fdata-sections")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffunction-sections -fdata-sections")

# Adding modules
add_subdirectory(my-blink)

# ------------------- NOT NEEDED ------------------------

#function(add_subdirectory_exclude_platforms NAME)
#    if (ARGN)
#        if (PICO_PLATFORM IN_LIST ARGN)
#            message("Skipping ${NAME} example which is unsupported on this platform")
#            return()
#        endif()
#        foreach(PATTERN IN LISTS ARGN)
#            string(REGEX MATCH "${PATTERN}" MATCHED "${PICO_PLATFORM}")
#            if (MATCHED)
#                message("Skipping ${NAME} example which is unsupported on this platform")
#                return()
#            endif()
#        endforeach()
#    endif()
#    add_subdirectory(${NAME})
#endfunction()

#add_subdirectory_exclude_platforms(my-blink)

# add_compile_options(-Wall
#        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
#        -Wno-unused-function # we have some for the docs that aren't called
#        )
# if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
#    add_compile_options(-Wno-maybe-uninitialized)
# endif()