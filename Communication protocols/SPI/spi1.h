#ifndef SPI1_H
#define SPI1_H

#include<stdint.h>

#define RCC_BASE                    0x40021000
#define SPI1_BASE                   0x40013000
#define GPIOA_BASE                  0x48000000
#define GPIOB_BASE                  0x48000400

#define RCC_AHB2ENR                 (*(volatile uint32_t*)(RCC_BASE + 0x4C))               
#define RCC_APB2ENR                 (*(volatile uint32_t*)(RCC_BASE + 0x60))

#define RCC_AHB2ENR_GPIOAEN_Pos     0U
#define RCC_AHB2ENR_GPIOBEN_Pos     1U
#define RCC_APB2ENR_SPI1_Pos        12U

#define RCC_AHB2ENR_GPIOAEN_Msk     (1UL << RCC_AHB2ENR_GPIOAEN_Pos)
#define RCC_AHB2ENR_GPIOBEN_Msk     (1UL << RCC_AHB2ENR_GPIOBEN_Pos)
#define RCC_APB2ENR_SPI1_Msk        (1UL << RCC_APB2ENR_SPI1_Pos)

#define SPI1_CR1                    (*(volatile uint32_t *)(SPI1_BASE + 0x00))
#define SPI1_CR2                    (*(volatile uint32_t *)(SPI1_BASE + 0x04))
#define SPI1_SR                     (*(volatile uint32_t *)(SPI1_BASE + 0x08UL))
#define SPI1_DR                     (*(volatile uint8_t *)(SPI1_BASE + 0x0CUL))
#define SPI1_CRCPR                  (*(volatile uint32_t *)(SPI1_BASE + 0x10UL))
#define SPI1_RXCRCR                 (*(volatile uint32_t *)(SPI1_BASE + 0x14UL))
#define SPI1_TXCRCR                 (*(volatile uint32_t *)(SPI1_BASE + 0x18UL))

#define SPI1_CR1_CPHA_Pos           0U
#define SPI1_CR1_CPOL_Pos           1U
#define SPI1_CR1_MSTR_Pos           2U
#define SPI1_CR1_BR_Pos             3U
#define SPI1_CR1_SPE_Pos            6U
#define SPI1_CR1_LSBFIRST_Pos       7U
#define SPI1_CR1_SSI_Pos            8U
#define SPI1_CR1_SSM_Pos            9U

#define SPI1_CR2_DS_Pos             8U
#define SPI1_CR2_FRXTH_Pos          12U

#define SPI1_SR_RXNE_Pos            0U
#define SPI1_SR_TXE_Pos             1U
#define SPI1_SR_MODF_Pos            5U
#define SPI1_SR_OVR_Pos             6U
#define SPI1_SR_BSY_Pos             7U

#define SPI1_CR1_CPHA_Msk       (1UL << SPI1_CR1_CPHA_Pos)
#define SPI1_CR1_CPOL_Msk       (1UL << SPI1_CR1_CPOL_Pos)
#define SPI1_CR1_MSTR_Msk       (1UL << SPI1_CR1_MSTR_Pos)
#define SPI1_CR1_BR_Msk         (7UL << SPI1_CR1_BR_Pos)
#define SPI1_CR1_SPE_Msk        (1UL << SPI1_CR1_SPE_Pos)
#define SPI1_CR1_LSBFIRST_Msk   (1UL << SPI1_CR1_LSBFIRST_Pos)
#define SPI1_CR1_SSI_Msk        (1UL << SPI1_CR1_SSI_Pos)
#define SPI1_CR1_SSM_Msk        (1UL << SPI1_CR1_SSM_Pos)

#define SPI1_CR2_DS_Msk         (0xFUL << SPI1_CR2_DS_Pos)
#define SPI1_CR2_FRXTH_Msk      (1UL << SPI1_CR2_FRXTH_Pos)

#define SPI1_SR_RXNE_Msk        (1UL << SPI1_SR_RXNE_Pos)
#define SPI1_SR_TXE_Msk         (1UL << SPI1_SR_TXE_Pos)
#define SPI1_SR_MODF_Msk        (1UL << SPI1_SR_MODF_Pos)
#define SPI1_SR_OVR_Msk         (1UL << SPI1_SR_OVR_Pos)
#define SPI1_SR_BSY_Msk         (1UL << SPI1_SR_BSY_Pos)

#define GPIOA_MODER                 (*(volatile uint32_t *)(GPIOA_BASE + 0x00UL))
#define GPIOA_OTYPER                (*(volatile uint32_t *)(GPIOA_BASE + 0x04UL))
#define GPIOA_OSPEEDR               (*(volatile uint32_t *)(GPIOA_BASE + 0x08UL))
#define GPIOA_BSRR                  (*(volatile uint32_t *)(GPIOA_BASE + 0x18UL))
#define GPIOA_AFRL                  (*(volatile uint32_t *)(GPIOA_BASE + 0x20UL))
#define GPIOA_AFRH                  (*(volatile uint32_t *)(GPIOA_BASE + 0x24UL))

#define GPIOB_MODER                 (*(volatile uint32_t *)(GPIOB_BASE + 0x00UL))
#define GPIOB_OTYPER                (*(volatile uint32_t *)(GPIOB_BASE + 0x04UL))
#define GPIOB_OSPEEDR               (*(volatile uint32_t *)(GPIOB_BASE + 0x08UL))
#define GPIOB_AFRL                  (*(volatile uint32_t *)(GPIOB_BASE + 0x20UL))

/* First set of pins */
#define GPIOA_SPI1_NSS_Pin_Pos1                 4U
#define GPIOA_SPI1_SCK_Pin_Pos1                 5U
#define GPIOA_SPI1_MISO_Pin_Pos1                6U
#define GPIOA_SPI1_MOSI_Pin_Pos1                7U

/* Second set of pins */
#define GPIOA_SPI1_NSS_Pin_Pos2                 15U
#define GPIOB_SPI1_SCK_Pin_Pos2                 3U
#define GPIOB_SPI1_MISO_Pin_Pos2                4U
#define GPIOB_SPI1_MOSI_Pin_Pos2                5U

typedef enum
{
    SPI_OK,
    SPI_ERROR,
    SPI_TIMEOUT,
} SPI_Status;

/* SPI1_PINS_NSSpin_SCKpin_MISOpin_MOSIpin options :
    1: SPI1_PINS_PA4_PA5_PA6_PA7 -> PA4/PA5/PA6/PA7
    2: SPI1_PINS_PA15_PB3_PB4_PB5 -> PA15/PB3/PB4/PB5
*/
typedef enum
{
    SPI1_PINS_PA4_PA5_PA6_PA7,
    SPI1_PINS_PA15_PB3_PB4_PB5
} SPI1_PinConfig;


/*
1. tx_data: The byte to be transmitted.
2. rx_data: The address where the received byte will be stored.
3. The return value indicates whether the exchange completed successfully.
*/

SPI_Status SPI1_Init(SPI1_PinConfig pin_cfg);
SPI_Status SPI1_TransmitReceive(const uint8_t *tx_data, uint8_t *rx_data, uint32_t length);

#endif