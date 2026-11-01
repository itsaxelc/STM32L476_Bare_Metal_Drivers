#ifndef LCD1602_H
#define LCD1602_H

#include <stdint.h>
#include "i2c1.h"

#define LCD1602_I2C_ADDRESS     0x27U
// #define LCD1602_I2C_ADDRESS     0x3FU
#define MAX_ROW                 1U
#define MAX_COLUMN              15U

#define LCD_RS        (1U << 0)   // P0
#define LCD_RW        (1U << 1)   // P1
#define LCD_E         (1U << 2)   // P2
#define LCD_BACKLIGHT (1U << 3)   // P3

#define LCD_D4        (1U << 4)   // P4
#define LCD_D5        (1U << 5)   // P5
#define LCD_D6        (1U << 6)   // P6
#define LCD_D7        (1U << 7)   // P7

I2C_Status LCD1602_Init(void);
void LCD1602_Clear(void);
void LCD1602_SetCursor(uint8_t row, uint8_t column);
void LCD1602_WriteChar(char c);
void LCD1602_WriteString(const char *str);

#endif