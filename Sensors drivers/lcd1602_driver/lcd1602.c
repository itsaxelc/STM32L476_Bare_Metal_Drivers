#include "lcd1602.h"

/*  P7 P6 P5 P4 P3 P2 P1 P0 
    0  1  0  1  1  0  0  1
sera interprété par le PCF8574 comme :
    D7 D6 D5 D4 BL E RW RS
    0  1  0  1  1 0  0  1
 */
static I2C_Status LCD1602_WritePCF8574(uint8_t value)
{
    return I2C1_Write(LCD1602_I2C_ADDRESS, &value, 1);
}
/*
E = 0 → placer les données sur D4-D7
    ↓
E = 1 → LCD lit les données
    ↓
E = 0
*/
static I2C_Status LCD1602_PulseEnable(uint8_t value)
{
    I2C_Status status;

    /* E = 1 */
    status = LCD1602_WritePCF8574(value | LCD_E);

    if (status != I2C_OK)
        return status;

    /* E = 0 */
    status = LCD1602_WritePCF8574(value & ~LCD_E);

    return status;
}

/*  Comment met-on un nibble de 4 bits sur les broches D4–D7 du LCD ?
    Notre LCD est utilisé en mode 4 bits.
*/
static I2C_Status LCD1602_Write4Bits(uint8_t nibble, uint8_t rs)
{
    uint8_t value = LCD_BACKLIGHT;

    if (rs)
        value |= LCD_RS;

    value |= (nibble & 0x0FU) << 4;

    return LCD1602_PulseEnable(value);
}

/*
    L'application va vouloir envoyer des octets complets :
    Pourquoi WriteByte() ?
    Parce que le LCD est en mode 4 bits, un octet doit être envoyé en deux nibbles
*/
static I2C_Status LCD1602_WriteByte(uint8_t value, uint8_t rs)
{
    I2C_Status status;

    status = LCD1602_Write4Bits(value >> 4, rs);

    if (status != I2C_OK)
        return status;

    status = LCD1602_Write4Bits(value & 0x0FU, rs);

    return status;
}

/* La broche RS signifie Register Select:
RS = 0 → l'octet est une commande
RS = 1 → l'octet est une donnée à afficher 
*/
static I2C_Status LCD1602_SendCommand(uint8_t command)
{
    return LCD1602_WriteByte(command, 0);
}

static I2C_Status LCD1602_SendData(uint8_t data)
{
    return LCD1602_WriteByte(data, 1);
}

static void LCD1602_Delay(void)
{
    for (volatile uint32_t i = 0; i < 10000U; i++)
    {
        __asm volatile ("nop");
    }
}


/*
Power-on
   ↓
delay
   ↓
0x03
   ↓
0x03
   ↓
0x03
   ↓
0x02
   ↓
0x28   → 4 bits, 2 lignes, 5×8
   ↓
0x08   → display OFF
   ↓
0x01   → clear
   ↓
delay
   ↓
0x06   → curseur avance automatiquement
   ↓
0x0C   → display ON
*/

I2C_Status LCD1602_Init(void)
{
    /* Wait after power-on */
    LCD1602_Delay();

    I2C_Status status;
    /* Initialize HD44780 in 4-bit mode */
    status=LCD1602_Write4Bits(0x03U, 0);
    if(status != I2C_OK){
        return status;
    }
    LCD1602_Delay();

    status=LCD1602_Write4Bits(0x03U, 0);
    if(status != I2C_OK){
        return status;
    }
    LCD1602_Delay();

    status=LCD1602_Write4Bits(0x03U, 0);
    if(status != I2C_OK){
        return status;
    }
    LCD1602_Delay();

    status=LCD1602_Write4Bits(0x02U, 0);
    if(status != I2C_OK){
        return status;
    }
    LCD1602_Delay();

    /* 4-bit mode, 2 lines, 5x8 font */
    status=LCD1602_SendCommand(0x28U);
    if(status != I2C_OK){
        return status;
    }

    /* Display OFF, cursor OFF, blink OFF */
    status=LCD1602_SendCommand(0x08U);
    if(status != I2C_OK){
        return status;
    }

    /* Clear display */
    status=LCD1602_SendCommand(0x01U);
    if(status != I2C_OK){
        return status;
    }
    LCD1602_Delay();

    /* Entry mode: cursor moves right */
    status=LCD1602_SendCommand(0x06U);
    if(status != I2C_OK){
        return status;
    }

    /* Display ON, cursor OFF, blink OFF */
    status=LCD1602_SendCommand(0x0CU);
    if(status != I2C_OK)
    {
        return status;
    }
}

void LCD1602_Clear(void)
{
    I2C_Status status= LCD1602_SendCommand(0x01U);
    if(status != I2C_OK) return;
}

void LCD1602_SetCursor(uint8_t row, uint8_t column)
{
    if(row > MAX_ROW || column > MAX_COLUMN)
    {
        return;
    }
    
    uint8_t address;

    if (row == 0)
        address = 0x00U + column;
    else
        address = 0x40U + column;

    I2C_Status status= LCD1602_SendCommand(0x80U | address);
    if(status != I2C_OK) return;
}

void LCD1602_WriteChar(char c)
{
    I2C_Status status= LCD1602_SendData((uint8_t)c);
    if(status != I2C_OK) return;
}

void LCD1602_WriteString(const char *str)
{
    while (*str != '\0')
    {
        LCD1602_WriteChar(*str);
        str++;
    }
}