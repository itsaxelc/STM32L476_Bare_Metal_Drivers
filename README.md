# STM32L476 Bare Metal Drivers

Bare-metal drivers for the main serial communication protocols (I2C, SPI, UART) on the **STM32L476RG**, with sensor and peripheral drivers built on top of them: an **MPU9250** 9-axis IMU (with its **AK8963** magnetometer), a **BME280** environmental sensor and an **LCD1602** character display (I2C backpack).

Written in C, with no HAL and no vendor libraries. Built with CMake and the GNU Arm Embedded toolchain.

## Features

- **I2C1 / I2C2 / I2C3** drivers (register-level, no HAL)
- **SPI1** driver
- **USART1 / USART2** drivers (USART2 tested over the ST-Link virtual COM port)
- **MPU9250** driver (accelerometer, gyroscope, temperature) and **AK8963** magnetometer driver
- **BME280** driver (temperature, pressure, humidity) <!-- TODO: precise the bus used (I2C / SPI) -->
- **LCD1602** driver over I2C, used as the validation target for the I2C driver
- **Selectable pin mapping**: I2C1, I2C2 and SPI1 can be initialized on any of their supported pin combinations by passing an enum
- Reproducible build with CMake presets and Ninja
- VS Code debug configuration (Cortex-Debug + OpenOCD)

## Hardware

| Item | Details |
|------|---------|
| Board | NUCLEO-L476RG |
| MCU | STM32L476RG (Cortex-M4F) |
| Debugger | On-board ST-Link |
| Display | LCD1602 with I2C backpack |
| IMU | MPU9250 (I2C) |
| Environmental sensor | BME280 |

## Pin selection

Most STM32 peripherals can be routed to several pin pairs through alternate functions. Instead of hard-coding a pin pair, the I2C1, I2C2 and SPI1 drivers take an enum that selects the combination to use, so the same driver works with whatever wiring your board allows.

```c
I2C1_Init(I2C1_PINS_PB6_PB7);
```

### I2C

| Peripheral | Enum value | SCL | SDA |
|------------|-----------|-----|-----|
| I2C1 | `I2C1_PINS_PB6_PB7` | PB6 | PB7 |
| I2C1 | `I2C1_PINS_PB8_PB9` | PB8 | PB9 |
| I2C2 | `I2C2_PINS_PB10_PB11` | PB10 | PB11 |
| I2C2 | `I2C2_PINS_PB13_PB14` | PB13 | PB14 |
| I2C3 | fixed (`I2C3_Init(void)`) | PC0 | PC1 |

### SPI

| Peripheral | Enum value | NSS | SCK | MISO | MOSI |
|------------|-----------|-----|-----|------|------|
| SPI1 | `SPI1_PINS_PA4_PA5_PA6_PA7` | PA4 | PA5 | PA6 | PA7 |
| SPI1 | `SPI1_PINS_PA15_PB3_PB4_PB5` | PA15 | PB3 | PB4 | PB5 |

### UART

| Peripheral | TX | RX |
|------------|----|----|
| USART1 | PA9 | PA10 |
| USART2 | PA2 | PA3 |

On the NUCLEO-L476RG, USART2 (PA2/PA3) is connected to the ST-Link virtual COM port, so no extra wiring is needed.

## Project structure

The drivers are split in two groups: the **communication protocols** (the low-level buses) and the **sensor drivers** (the devices that sit on top of those buses).

```
STM32L476_Bare_Metal_Drivers/
├── .vscode/                     VS Code debug configuration (Cortex-Debug + OpenOCD)
├── Communication protocols/     Low-level bus drivers
│   ├── I2C/                     I2C1, I2C2, I2C3 drivers
│   ├── SPI/                     SPI1 driver
│   └── UART/                    USART1, USART2 drivers
├── Sensors drivers/             Drivers for external sensors and peripherals
│   ├── BME280/                  BME280 driver
│   ├── MPU9250_driver/          MPU9250 and AK8963 drivers (use I2C1)
│   └── lcd1602_driver/          LCD1602 driver (uses I2C1)
├── examples/                    Example applications (one file per example)
├── .gitignore
├── CMakeLists.txt               Build configuration
├── CMakePresets.json            Build presets
├── README.md
├── STM32L476RGTx_FLASH.ld       Linker script
├── gcc-arm-none-eabi.cmake      CMake toolchain file
├── startup_stm32l476xx.s        Startup code and vector table
└── system.c                     System initialization
```

## Prerequisites

The following tools must be installed and available in your `PATH`:

- [Arm GNU Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) (`arm-none-eabi-gcc`)
- [CMake](https://cmake.org/download/)
- [Ninja](https://ninja-build.org/)
- [OpenOCD](https://openocd.org/) (to flash and debug)
- [VS Code](https://code.visualstudio.com/) with the **Cortex-Debug** extension (for debugging)

Check your setup:

```
arm-none-eabi-gcc --version
cmake --version
ninja --version
openocd --version
```

## Build

```
git clone https://github.com/itsaxelc/STM32L476_Bare_Metal_Drivers.git
cd STM32L476_Bare_Metal_Drivers
cmake --preset Debug -DEXAMPLE=lcd
cmake --build --preset Debug
```

`EXAMPLE` selects which application from `examples/` is built into the firmware:

| `EXAMPLE` | File | What it does |
|-----------|------|--------------|
| `lcd` | `examples/example_lcd.c` | Displays text on the LCD1602 over I2C1 |
| `uart` | `examples/example_uart.c` | Sends a message on USART2 in a loop |

To switch example, re-run the configure step with another value (for instance `-DEXAMPLE=uart`) and rebuild.

The firmware is produced in `build/` as `communication_protocols.elf`.

## Flash

### Option 1: OpenOCD

```
openocd -f interface/stlink.cfg -f target/stm32l4x.cfg -c "program build/Debug/communication_protocols.elf verify reset exit"
```

### Option 2: STM32CubeProgrammer

```
STM32_Programmer_CLI -c port=SWD -w build/Debug/communication_protocols.elf -v -rst
```

`STM32_Programmer_CLI` is installed with [STM32CubeProgrammer](https://www.st.com/en/development-tools/stm32cubeprog.html) (in its `bin` folder, add it to your `PATH`).

## Debug

A ready-to-use configuration is provided in `.vscode/launch.json` (Cortex-Debug + OpenOCD). Make sure `openocd` and `arm-none-eabi-gdb` are in your `PATH`, then press **F5** in VS Code.

## Examples

### LCD1602 over I2C (`examples/example_lcd.c`)

```c
#include <stdint.h>
#include "i2c1.h"
#include "lcd1602.h"

int main(void)
{
    I2C1_Init(I2C1_PINS_PB6_PB7);
    LCD1602_Init();

    LCD1602_SetCursor(0, 0);
    LCD1602_WriteString("Hello");

    LCD1602_SetCursor(1, 0);
    LCD1602_WriteString("BOARD:L476");

    while (1) { }
}
```

### UART over the ST-Link virtual COM port (`examples/example_uart.c`)

```c
#include <stdint.h>
#include "UART/usart2.h"

int main(void)
{
    USART2_Init();

    while (1)
    {
        USART2_TransmitString("hello from USART2\r\n");
        for (volatile uint32_t i = 0U; i < 500000U; i++);
    }
}
```

Open the board's COM port in a serial terminal (<!-- TODO: baud rate -->) to see the output.


**Cédric Compaoré** ([@itsaxelc](https://github.com/itsaxelc)), Aerospace Engineering student at ENSA Safi.