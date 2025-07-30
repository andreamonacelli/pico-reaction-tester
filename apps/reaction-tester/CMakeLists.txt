# Defining the executable
add_executable(reaction-tester
    main.c
)

# Link against Pico, FreeRTOS, microros
target_link_libraries(reaction-tester
    pico_stdlib
    FreeRTOS-Kernel FreeRTOS-Kernel-Heap4
    micro_ros
)

# Configure Pico IO settings
pico_enable_stdio_usb(reaction-tester 1)
pico_enable_stdio_uart(reaction-tester 0)

# Compile definitions for Pico config
target_compile_definitions(reaction-tester PRIVATE
    PICO_UART_ENABLE_CRLF_SUPPORT=0
    PICO_STDIO_ENABLE_CRLF_SUPPORT=0
    PICO_STDIO_DEFAULT_CRLF=0
)

# Set UF2 output directory
set_target_properties(reaction-tester PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Generate UF2 and binary outputs
pico_add_extra_outputs(reaction-tester)