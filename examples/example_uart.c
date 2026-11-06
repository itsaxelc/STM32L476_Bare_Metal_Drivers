#include <stdint.h>
#include "uart.h"

int main(void)
{
    USART_Init();

    while (1)
    {
        USART2_TransmitString("hello from USART2\r\n");
        for (volatile uint32_t i = 0U; i < 500000U; i++);
    }
}