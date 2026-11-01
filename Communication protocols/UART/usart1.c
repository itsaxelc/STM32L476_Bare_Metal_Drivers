#include "usart1.h"
#include <stdint.h>

void USART1_Init(void)
{
    RCC_AHB2ENR |= RCC_AHB2ENR_GPIOAEN_Msk;
    RCC_CCIPR &= ~RCC_CCIPR_USART1SEL_Msk;

    (void)RCC_AHB2ENR;

    RCC_APB2ENR |= RCC_APB2ENR_USART1EN_Msk;
    (void)RCC_APB2ENR;

    GPIOA_MODER &= ~((3U << (USART1_TX_GPIO_Pin*2)) | (3U << (USART1_RX_GPIO_Pin*2)));
    GPIOA_MODER |= ((2U << (USART1_TX_GPIO_Pin*2)) | (2U << (USART1_RX_GPIO_Pin*2)));
    GPIOA_OTYPER &= ~(1<< USART1_TX_GPIO_Pin);
    GPIOA_OSPEEDR &= ~((3U << (USART1_TX_GPIO_Pin*2)) | (3U << (USART1_RX_GPIO_Pin*2)));
    GPIOA_OSPEEDR |= ((3U << (USART1_TX_GPIO_Pin*2)) | (3U << (USART1_RX_GPIO_Pin*2)));
    GPIOA_PUPDR &= ~((3U << (USART1_TX_GPIO_Pin * 2U)) | (3U << (USART1_RX_GPIO_Pin * 2U)));
    GPIOA_PUPDR |= (1U << (USART1_RX_GPIO_Pin*2));
    GPIOA_AFRH &= ~((15U << ((USART1_TX_GPIO_Pin-8)*4)) | (15U << ((USART1_RX_GPIO_Pin-8)*4)));
    GPIOA_AFRH |= ((7U << ((USART1_TX_GPIO_Pin-8)*4)) | (7U << ((USART1_RX_GPIO_Pin-8)*4)));

    USART1_CR1 &= ~USART1_CR1_UE_Msk;

    USART1_CR1 &= ~(USART1_CR1_M1_Msk | USART1_CR1_M0_Msk | USART1_CR1_PCE_Msk | USART1_CR1_OVER8_Msk);
    USART1_CR1 |= (USART1_CR1_RE_Msk | USART1_CR1_TE_Msk);
    USART1_BRR = 35;
    USART1_CR2 &= ~USART1_CR2_STOP_Msk;

    USART1_CR1 |= USART1_CR1_UE_Msk;

    /* USART1 configuration will follow */
}

void USART1_Write(uint8_t data)
{
    while (!(USART1_ISR & USART1_ISR_TXE_Msk))
    {
        /* Wait until transmit register is empty */
    }

    USART1_TDR = data;
}

uint8_t USART1_Read(void)
{
    while (!(USART1_ISR & USART1_ISR_RXNE_Msk))
    {
        /* Wait until data is received */
    }

    return (uint8_t)USART1_RDR;
}

void USART1_TransmitString(const char *str)
{
    if (str == 0U)
    {
        return;
    }

    while (*str != '\0')
    {
        USART1_Write((uint8_t)*str);
        str++;
    }
}