# STM32L476 Bare-Metal Drivers

Register-level peripheral drivers and device drivers for the **STM32L476RG (Cortex-M4F)**, written in C without the STM32 HAL. The project includes reusable I²C and UART/USART drivers, an SPI1 driver, and sensor/display drivers built on top of the communication layer.

## Features

- **Generic I²C driver** for I2C1, I2C2 and I2C3, selected at runtime through an `I2C_Handle` and enums.
- **Generic UART driver** for USART1 and USART2, using the same handle-based API for either peripheral.
- **SPI1 driver** with selectable supported pin mappings and full-duplex transmit/receive.
- **Device drivers** for the BME280 environmental sensor, MPU9250 IMU, AK8963 magnetometer and LCD1602 character display with I²C backpack.
- Register-level peripheral configuration: GPIO alternate functions, peripheral clocks and peripheral registers are configured inside the drivers.
- CMake/Ninja build presets, an Arm GNU toolchain file and VS Code/OpenOCD debugging configuration.

## Hardware

| Item | Details |
|---|---|
| Development board | NUCLEO-L476RG |
| Microcontroller | STM32L476RG, Arm Cortex-M4F |
| Debugger/programmer | On-board ST-Link |
| Serial console | USART2 through the ST-Link virtual COM port (PA2/PA3) |
| Example display | LCD1602 with I²C backpack (typically PCF8574) |
| IMU | MPU9250, including its AK8963 magnetometer |
| Environmental sensor | BME280 |

## Repository layout

```text
STM32L476_Bare_Metal_Drivers/
├── Communication protocols/
│   ├── I2C/                      Generic I2C driver (i2c.c / i2c.h)
│   ├── SPI/                      SPI1 driver (spi1.c / spi1.h)
│   └── UART/                     Generic USART driver (uart.c / uart.h)
├── Sensors drivers/
│   ├── BME280/                   BME280 environmental sensor driver
│   ├── MPU9250_driver/           MPU9250 and AK8963 drivers
│   └── lcd1602_driver/           LCD1602 I²C-backpack driver
├── examples/
│   ├── example_lcd.c             LCD1602 example
│   └── example_uart.c            UART/USART example
├── .vscode/                      VS Code debug configuration
├── CMakeLists.txt
├── CMakePresets.json
├── gcc-arm-none-eabi.cmake
├── startup_stm32l476xx.s
├── STM32L476RGTx_FLASH.ld
├── system.c
└── README.md
```

## Driver architecture

The application chooses a peripheral and initializes a handle. The driver then selects the peripheral's register block and configures the associated clocks and pins. Applications and sensor drivers do not need to access the peripheral's RCC or GPIO registers directly.

```text
Application / example
        │
        ├── Device drivers: BME280, MPU9250/AK8963, LCD1602
        │                    │
        └────────────────────┴── Shared communication APIs
                                  ├── I²C: I2C_Init / I2C_Write / I2C_Read / I2C_WriteRead
                                  ├── UART: UART_Init / UART_Write / UART_Read / UART_TransmitString
                                  └── SPI1: SPI1_Init / SPI1_TransmitReceive
```

### I²C: one driver for I2C1, I2C2 and I2C3

Include `i2c.h`. Select the peripheral and one of the pin mappings supported by the STM32L476 implementation:

| Peripheral | Pin selection enum | SCL | SDA |
|---|---|---|---|
| I2C1 | `I2C1_PINS_PB6_PB7` | PB6 | PB7 |
| I2C1 | `I2C1_PINS_PB8_PB9` | PB8 | PB9 |
| I2C2 | `I2C2_PINS_PB10_PB11` | PB10 | PB11 |
| I2C2 | `I2C2_PINS_PB13_PB14` | PB13 | PB14 |
| I2C3 | `I2C3_PINS_PC0_PC1` | PC0 | PC1 |

Example: initialize I2C1 and read one register from a 7-bit I²C slave address.

```c
#include "i2c.h"

I2C_Handle hi2c1;

void App_Init(void)
{
    I2C_Status status = I2C_Init(
        &hi2c1,
        I2C_INSTANCE_1,
        I2C1_PINS_PB6_PB7
    );

    if (status != I2C_OK)
    {
        /* Handle initialization error. */
    }
}

I2C_Status Read_Device_Register(uint8_t address, uint8_t reg, uint8_t *value)
{
    return I2C_WriteRead(&hi2c1, address, &reg, 1U, value, 1U);
}
```

Public API:

- `I2C_Init(I2C_Handle *hi2c, I2C_Instance instance, I2C_PinConfig pins)`
- `I2C_Write(...)` — send bytes to a slave.
- `I2C_Read(...)` — read bytes from a slave.
- `I2C_WriteRead(...)` — write a register/address phase followed by a repeated-START read.

The `I2C_Status` return value distinguishes success, generic errors, timeout and NACK (`I2C_OK`, `I2C_ERROR`, `I2C_TIMEOUT`, `I2C_NACK`). Slave addresses are passed as **7-bit addresses**, without the R/W bit.

### UART/USART: one driver for USART1 and USART2

Include `uart.h`. `UART_Init()` configures the selected USART, its peripheral clock and the GPIO pins used by the current implementation.

| Instance enum | Peripheral | TX | RX | Alternate function |
|---|---|---|---|---|
| `UART_INSTANCE_1` | USART1 | PA9 | PA10 | AF7 |
| `UART_INSTANCE_2` | USART2 | PA2 | PA3 | AF7 |

Example: send a message through the ST-Link virtual COM port using USART2.

```c
#include "uart.h"

UART_Handle huart2;

void App_Init(void)
{
    UART_Status status = UART_Init(&huart2, UART_INSTANCE_2);

    if (status != UART_OK)
    {
        /* Handle initialization error. */
    }

    (void)UART_TransmitString(&huart2, "Hello from USART2\r\n");
}
```

Public API:

- `UART_Init(UART_Handle *huart, UART_Instance instance)`
- `UART_Write(UART_Handle *huart, uint8_t data)`
- `UART_Read(UART_Handle *huart, uint8_t *data)`
- `UART_TransmitString(UART_Handle *huart, const char *str)`

These calls return `UART_Status` (`UART_OK`, `UART_ERROR` or `UART_TIMEOUT`). On the NUCLEO-L476RG, USART2 TX/RX on PA2/PA3 is connected to the ST-Link virtual COM port; open that board's COM port in a serial terminal to view output.

### SPI1

The SPI driver remains SPI1-specific and supports these pin mappings:

| Pin selection enum | NSS (software-controlled) | SCK | MISO | MOSI |
|---|---|---|---|---|
| `SPI1_PINS_PA4_PA5_PA6_PA7` | PA4 | PA5 | PA6 | PA7 |
| `SPI1_PINS_PA15_PB3_PB4_PB5` | PA15 | PB3 | PB4 | PB5 |

Public functions include `SPI1_Init(SPI1_PinConfig pin_cfg)` and `SPI1_TransmitReceive(const uint8_t *tx_data, uint8_t *rx_data, uint32_t length)`. A null TX buffer sends dummy bytes (`0xFF`); a null RX buffer discards received bytes.

## Device drivers

- **BME280** — environmental sensor driver with configurable temperature/pressure/humidity oversampling, IIR filter and operating mode, calibration coefficient parsing, raw data acquisition and compensated readings.
- **MPU9250** — accelerometer, gyroscope and temperature measurements through the common I²C API; its APIs accept an `I2C_Handle *` so the application selects the bus.
- **AK8963** — magnetometer associated with the MPU9250; its APIs also accept an `I2C_Handle *`.
- **LCD1602** — character display using an I²C backpack; its API accepts an `I2C_Handle *`.

Example: initialize the I²C bus and the LCD1602 display.

```c
#include "i2c.h"
#include "lcd1602.h"

I2C_Handle hi2c1;

int main(void)
{
    if (I2C_Init(&hi2c1, I2C_INSTANCE_1, I2C1_PINS_PB6_PB7) != I2C_OK)
    {
        while (1) { /* Initialization error. */ }
    }

    if (LCD1602_Init(&hi2c1) != I2C_OK)
    {
        while (1) { /* Display initialization error. */ }
    }

    LCD1602_SetCursor(&hi2c1, 0U, 0U);
    LCD1602_WriteString(&hi2c1, "Hello");
    LCD1602_SetCursor(&hi2c1, 1U, 0U);
    LCD1602_WriteString(&hi2c1, "BOARD:L476");

    while (1) { }
}
```

**I²C association:** `BME280_Handle` contains its own `I2C_Handle hi2c` member alongside the sensor address, calibration coefficients and configuration. The BME280 driver can therefore perform bus transactions through `&dev->hi2c`; individual measurement and register-access functions do not need a separate I²C-handle argument. During setup, ensure the selected, already-initialized I²C handle is stored in the BME280 device object and is not overwritten by later whole-structure initialization. The supplied CMake examples currently cover the LCD and UART paths; BME280 is included as a device driver but does not yet have a dedicated built-in example.

## Build

### Requirements

- Arm GNU Toolchain (`arm-none-eabi-gcc`)
- CMake 3.20 or later
- Ninja
- OpenOCD for flashing/debugging, or STM32CubeProgrammer
- VS Code with Cortex-Debug is optional for the supplied debug configuration

Check that the required tools are available:

```sh
arm-none-eabi-gcc --version
cmake --version
ninja --version
openocd --version
```

### Configure and build an example

```sh
git clone https://github.com/itsaxelc/STM32L476_Bare_Metal_Drivers.git
cd STM32L476_Bare_Metal_Drivers
cmake --preset Debug -DEXAMPLE=lcd
cmake --build --preset Debug
```

The `EXAMPLE` CMake cache variable selects `examples/example_<name>.c`:

| Value | Source file | Purpose |
|---|---|---|
| `lcd` | `examples/example_lcd.c` | Displays text on the LCD1602 over I²C. |
| `uart` | `examples/example_uart.c` | Repeatedly transmits a message through USART2. |

To build the UART example instead, configure again with `-DEXAMPLE=uart`, then build:

```sh
cmake --preset Debug -DEXAMPLE=uart
cmake --build --preset Debug
```

With the supplied single-configuration Ninja preset, the firmware output is `build/communication_protocols.elf`.

### Flash with OpenOCD

```sh
openocd -f interface/stlink.cfg -f target/stm32l4x.cfg \
  -c "program build/communication_protocols.elf verify reset exit"
```

Alternatively, using STM32CubeProgrammer:

```sh
STM32_Programmer_CLI -c port=SWD -w build/communication_protocols.elf -v -rst
```

### Debug in VS Code

The repository includes a `.vscode` configuration for Cortex-Debug/OpenOCD. Ensure OpenOCD and `arm-none-eabi-gdb` are on your `PATH`, connect the board over ST-Link, and start the provided debug configuration from VS Code.

## Clock configuration notes

The current low-level drivers retain fixed timing values from the original implementations (`TIMINGR` for I²C and `BRR` for USART). Those values are tied to the peripheral/kernel clock frequencies. If the clock tree or target baud rate changes, recalculate the timing values and verify communication on the hardware.

---

Cédric COMPAORE · [@itsaxelc](https://github.com/itsaxelc)
