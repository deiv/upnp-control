cmake_minimum_required(VERSION 3.8.2)
include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)
project(esp_firmware)

FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE ${app_sources})

#
# Generate a .h file for each static file
#
foreach(inc_file
        index.html
        root.xml
        404.html)
    generate_inc_file_for_target(
        app
        fsdata/${inc_file}
        ${ZEPHYR_BINARY_DIR}/include/generated/${inc_file}.inc
    )
endforeach()
