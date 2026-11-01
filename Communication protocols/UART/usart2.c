#include "usart2.h"
#include <stdint.h>

void USART2_Init(void)
{
    RCC_AHB2ENR |= RCC_AHB2ENR_GPIOAEN_Msk;
    RCC_CCIPR &= ~RCC_CCIPR_USART2SEL_Msk;

    (void)RCC_AHB2ENR;

    RCC_APB1ENR1 |= RCC_APB1ENR1_USART2EN_Msk;
    (void)RCC_APB1ENR1;

    GPIOA_MODER &= ~((3U << (USART2_TX_GPIO_Pin*2)) | (3U << (USART2_RX_GPIO_Pin*2)));
    GPIOA_MODER |= ((2U << (USART2_TX_GPIO_Pin*2)) | (2U << (USART2_RX_GPIO_Pin*2)));
    GPIOA_OTYPER &= ~(1<< USART2_TX_GPIO_Pin);
    GPIOA_OSPEEDR &= ~((3U << (USART2_TX_GPIO_Pin*2)) | (3U << (USART2_RX_GPIO_Pin*2)));
    GPIOA_OSPEEDR |= ((3U << (USART2_TX_GPIO_Pin*2)) | (3U << (USART2_RX_GPIO_Pin*2)));
    GPIOA_PUPDR &= ~((3U << (USART2_TX_GPIO_Pin * 2U)) | (3U << (USART2_RX_GPIO_Pin * 2U)));
    GPIOA_PUPDR |= (1U << (USART2_RX_GPIO_Pin*2));
    GPIOA_AFRL &= ~((15U << (USART2_TX_GPIO_Pin*4)) | (15U << (USART2_RX_GPIO_Pin*4)));
    GPIOA_AFRL |= ((7U << (USART2_TX_GPIO_Pin*4)) | (7U << (USART2_RX_GPIO_Pin*4)));

    USART2_CR1 &= ~USART2_CR1_UE_Msk;

    USART2_CR1 &= ~(USART2_CR1_M1_Msk | USART2_CR1_M0_Msk | USART2_CR1_PCE_Msk | USART2_CR1_OVER8_Msk);
    USART2_CR1 |= (USART2_CR1_RE_Msk | USART2_CR1_TE_Msk);
    USART2_BRR = 35;
    USART2_CR2 &= ~USART2_CR2_STOP_Msk;

    USART2_CR1 |= USART2_CR1_UE_Msk;

    /* USART2 configuration will follow */
}

void USART2_Write(uint8_t data)
{
    while (!(USART2_ISR & USART2_ISR_TXE_Msk))
    {
        /* Wait until transmit register is empty */
    }

    USART2_TDR = data;
}

uint8_t USART2_Read(void)
{
    while (!(USART2_ISR & USART2_ISR_RXNE_Msk))
    {
        /* Wait until data is received */
    }

    return (uint8_t)USART2_RDR;
}

void USART2_TransmitString(const char *str)
{
    if (str == 0U)
    {
        return;
    }

    while (*str != '\0')
    {
        USART2_Write((uint8_t)*str);
        str++;
    }
}