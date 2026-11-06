#include <stdint.h>
#include "uart.h"

int main(void)
{
    UART_Handle huart;
    UART_Instance instance = UART_INSTANCE_2;
    UART_Init(&huart, instance);

    while (1)
    {
        UART_TransmitString(&huart, "hello from USART2\r\n");
        for (volatile uint32_t i = 0U; i < 500000U; i++);
    }
}