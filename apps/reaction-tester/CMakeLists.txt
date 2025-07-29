# Defining the executable
add_executable(reaction-tester
    main.c
)

# Link against Pico, FreeRTOS, microros
target_link_libraries(reaction-tester
    pico_stdlib
    pico_async_context_freertos
    microros
    FreeRTOS-Kernel FreeRTOS-Kernel-Heap4
)

# Configure Pico IO settings
pico_enable_stdio_usb(reaction-tester 1)
pico_enable_stdio_uart(reaction-tester 0)

# Generate UF2 and binary outputs
pico_add_extra_outputs(reaction-tester)