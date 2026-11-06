#include <stdint.h>
#include "i2c.h"
#include "lcd1602.h"

int main(void)
{
    I2C_Handle hi2c1;
    I2C_Instance instance = I2C_INSTANCE_1;
    I2C_PinConfig cfg = I2C1_PINS_PB6_PB7;
    I2C_Init(&hi2c1, instance, cfg);
    LCD1602_Init(&hi2c1);

    LCD1602_SetCursor(&hi2c1, 0, 0);
    LCD1602_WriteString(&hi2c1, "Hello");

    LCD1602_SetCursor(&hi2c1, 1, 0);
    LCD1602_WriteString(&hi2c1, "BOARD:L476");

    while (1) { }
}