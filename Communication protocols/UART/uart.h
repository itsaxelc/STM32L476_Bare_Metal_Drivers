#ifndef UART_H
#define UART_H

#include <stdint.h>

/* ============================================================
 * USART peripheral register layout (STM32L476xx)
 * Reserved fields preserve the offsets of registers used below.
 * ============================================================ */

typedef struct
{
    volatile uint32_t CR1;          /* Offset: 0x00 */
    volatile uint32_t CR2;          /* Offset: 0x04 */
    volatile uint32_t CR3;          /* Offset: 0x08 */
    volatile uint32_t BRR;          /* Offset: 0x0C */
    volatile uint32_t GTPR;         /* Offset: 0x10 */
    uint32_t RESERVED0[2];          /* Offsets: 0x14, 0x18 */
    volatile uint32_t ISR;          /* Offset: 0x1C */
    volatile uint32_t ICR;          /* Offset: 0x20 */
    volatile uint32_t RDR;          /* Offset: 0x24 */
    volatile uint32_t TDR;          /* Offset: 0x28 */

} UART_Registers;

/* ============================================================
 * Supported USART instances
 * ============================================================ */

typedef enum
{
    UART_INSTANCE_1 = 1,
    UART_INSTANCE_2 = 2

} UART_Instance;

/* ============================================================
 * Driver status
 * ============================================================ */

typedef enum
{
    UART_OK = 0,
    UART_ERROR,
    UART_TIMEOUT

} UART_Status;

/* ============================================================
 * Handle
 * The driver fills instance during UART_Init().
 * Application code does not provide peripheral addresses.
 * ============================================================ */

typedef struct
{
    UART_Registers *instance;

} UART_Handle;

/* ============================================================
 * Public API
 *
 * UART_Init uses the pin mapping from the former drivers:
 *   USART1: PA9 (TX), PA10 (RX), AF7
 *   USART2: PA2 (TX), PA3 (RX), AF7
 *
 * Both instances retain the original BRR value (35) and select
 * their peripheral bus clock, matching the previous implementation.
 * ============================================================ */

UART_Status UART_Init(UART_Handle *huart, UART_Instance instance);
UART_Status UART_Write(UART_Handle *huart, uint8_t data);
UART_Status UART_Read(UART_Handle *huart, uint8_t *data);
UART_Status UART_TransmitString(UART_Handle *huart, const char *str);

#endif /* UART_H */
