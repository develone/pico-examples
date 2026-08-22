cmake_minimum_required(VERSION 3.13)

# Pull in SDK code
include($ENV{PICO_SDK_PATH}/pico_sdk_init.cmake)

project(pico2_cpp_uart_tx_project C CXX ASM)
set(CMAKE_CXX_STANDARD 17)

# Initialize the SDK
pico_sdk_init()

add_executable(pico_cpp_uart_tx_project
    main.cpp
)

# Link the standard standard-library dependencies
target_link_libraries(pico_cpp_uart_tx_project 
    pico_stdlib
)

# Enable USB serial output, disable hardware UART pin routing
pico_enable_stdio_usb(pico_cpp_uart_tx_project 1)
pico_enable_stdio_uart(pico_cpp_uart_tx_project 0)

# Create extra build outputs including the flashable .uf2 file
pico_add_extra_outputs(pico_cpp_uart_tx_project)
