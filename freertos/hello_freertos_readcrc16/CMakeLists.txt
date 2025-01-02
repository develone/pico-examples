set(TARGET_NAME hello_freertos_readcrc16)
add_executable(${TARGET_NAME}
    hello_freertos_readcrc16.c readcrc16.h readcrc16.c
    )
target_include_directories(${TARGET_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/..
    )
target_link_libraries(${TARGET_NAME} PRIVATE
    pico_async_context_freertos
    FreeRTOS-Kernel-Heap4
    pico_stdlib
	xmodem lifting crc_crc16 cir-buf klt littlefs openjpeg
    )
if(PICO_CYW43_SUPPORTED)
    # For led support on pico_w
    target_link_libraries(${TARGET_NAME} PRIVATE
        pico_cyw43_arch_none
    )
endif()
pico_add_extra_outputs(${TARGET_NAME})
