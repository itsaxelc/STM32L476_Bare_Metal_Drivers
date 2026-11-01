cmake_minimum_required(VERSION 3.20)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/gcc-arm-none-eabi.cmake)
project(communication_protocols C ASM)

set(CMAKE_C_STANDARD 11)

# Example to build into the firmware: lcd, uart, ...
# Usage: cmake --preset Debug -DEXAMPLE=uart
set(EXAMPLE "lcd" CACHE STRING "Example to build (file examples/example_<name>.c)")

set(EXAMPLE_SOURCE ${CMAKE_SOURCE_DIR}/examples/example_${EXAMPLE}.c)
if(NOT EXISTS ${EXAMPLE_SOURCE})
    message(FATAL_ERROR "Example '${EXAMPLE}' not found: ${EXAMPLE_SOURCE}")
endif()
message(STATUS "Building example: ${EXAMPLE}")

# Top-level driver folders (their names contain a space: always keep them quoted)
set(PROTOCOLS_DIR  "${CMAKE_SOURCE_DIR}/Communication protocols")
set(SENSORS_DRIVERS_DIR "${CMAKE_SOURCE_DIR}/Sensors drivers")

# Driver sources (re-run cmake if you add a new .c file)
file(GLOB DRIVER_SOURCES CONFIGURE_DEPENDS
    "${PROTOCOLS_DIR}/I2C/*.c"
    "${PROTOCOLS_DIR}/SPI/*.c"
    "${PROTOCOLS_DIR}/UART/*.c"
    "${SENSORS_DRIVERS_DIR}/BME280/*.c"
    "${SENSORS_DRIVERS_DIR}/MPU9250_driver/*.c"
    "${SENSORS_DRIVERS_DIR}/lcd1602_driver/*.c"
)

add_executable(${PROJECT_NAME}
    ${EXAMPLE_SOURCE}
    ${DRIVER_SOURCES}
    system.c
    startup_stm32l476xx.s
)

# Both "i2c1.h" and "I2C/i2c1.h" style includes work with this
# (same for sensors: "bme280.h" and "BME280/bme280.h")
target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_SOURCE_DIR}"
    "${PROTOCOLS_DIR}"
    "${PROTOCOLS_DIR}/I2C"
    "${PROTOCOLS_DIR}/SPI"
    "${PROTOCOLS_DIR}/UART"
    "${SENSORS_DRIVERS_DIR}"
    "${SENSORS_DRIVERS_DIR}/BME280"
    "${SENSORS_DRIVERS_DIR}/MPU9250_driver"
    "${SENSORS_DRIVERS_DIR}/lcd1602_driver"
)

target_compile_options(${PROJECT_NAME} PRIVATE
    -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard
    -Wall -Wextra -Og -g3
)

target_link_options(${PROJECT_NAME} PRIVATE
    -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard
    -T${CMAKE_SOURCE_DIR}/STM32L476RGTx_FLASH.ld
    -Wl,--gc-sections -Wl,-Map=${PROJECT_NAME}.map
    --specs=nano.specs --specs=nosys.specs
)