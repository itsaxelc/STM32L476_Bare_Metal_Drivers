#include "spi1.h"
#include<stdint.h>
#include<stddef.h>

#define SPI1_WAIT_LIMIT 1000000UL

static SPI1_PinConfig spi1_pin_config;

/* SPI1_WaitForTXE(): Waits until TXE = 1. The transmit register can then accept new data. */
static SPI_Status SPI1_WaitForTXE(uint32_t timeout);
/* SPI1_WaitForRXNE(): Waits until RXNE = 1. The received data must be read from SPI1_DR. */
static SPI_Status SPI1_WaitForRXNE(uint32_t timeout);
/* SPI1_WaitForNotBusy(): Waits until BSY = 0. The SPI peripheral has finished its activity on the bus. */
static SPI_Status SPI1_WaitForNotBusy(uint32_t timeout);

static SPI_Status SPI1_CheckErrors(void);
static void SPI1_AbortTransfer(void);

/* The timeout is an iteration counter, not a duration in microseconds. 
Its actual duration depends on the processor clock frequency and the compiled code. */

static SPI_Status SPI1_WaitForTXE(uint32_t timeout)
{
    while ((SPI1_SR & SPI1_SR_TXE_Msk) == 0U)
    {
        if (timeout == 0U) return SPI_TIMEOUT;
        timeout--;
    }
    return SPI_OK;
}

static SPI_Status SPI1_WaitForRXNE(uint32_t timeout)
{
    while ((SPI1_SR & SPI1_SR_RXNE_Msk) == 0U)
    {
        if (timeout == 0U) return SPI_TIMEOUT;
        timeout--;
    }
    return SPI_OK;
}

static SPI_Status SPI1_WaitForNotBusy(uint32_t timeout)
{
    while ((SPI1_SR & SPI1_SR_BSY_Msk) != 0U)
    {
        if (timeout == 0U) return SPI_TIMEOUT;
        timeout--;
    }
    return SPI_OK;
}

static SPI_Status SPI1_CheckErrors(void)
{
    uint32_t sr = SPI1_SR;
    if ((sr & SPI1_SR_MODF_Msk) != 0U){
        /* Clear MODF: SR has been read; write CR1 */
        SPI1_CR1 = SPI1_CR1;
        return SPI_ERROR;
    }
    if ((sr & SPI1_SR_OVR_Msk) != 0U){
        /* OVR is cleared by reading DR, then reading SR */
        (void)SPI1_DR;
        (void)SPI1_SR;
        return SPI_ERROR;
    }
    return SPI_OK;
}

void SPI1_NSS_Low(void)
{
    if (spi1_pin_config == SPI1_PINS_PA4_PA5_PA6_PA7) GPIOA_BSRR = (1UL << (GPIOA_SPI1_NSS_Pin_Pos1 + 16U));
    else GPIOA_BSRR = (1UL << (GPIOA_SPI1_NSS_Pin_Pos2 + 16U));
}

void SPI1_NSS_High(void)
{
    if (spi1_pin_config == SPI1_PINS_PA4_PA5_PA6_PA7) GPIOA_BSRR = (1UL << GPIOA_SPI1_NSS_Pin_Pos1);
    else GPIOA_BSRR = (1UL << GPIOA_SPI1_NSS_Pin_Pos2);
}

static void SPI1_AbortTransfer(void)
{
    uint32_t timeout = SPI1_WAIT_LIMIT;
    while ((SPI1_SR & SPI1_SR_BSY_Msk) != 0U)
    {
        if (timeout == 0U) break;
        timeout--;
    }
    /* Clear OVR if present: read DR, then SR */
    if ((SPI1_SR & SPI1_SR_OVR_Msk) != 0U)
    {
        (void)SPI1_DR;
        (void)SPI1_SR;
    }
}

SPI_Status SPI1_Init(SPI1_PinConfig pin_cfg)
{
    if ((pin_cfg != SPI1_PINS_PA4_PA5_PA6_PA7) && (pin_cfg != SPI1_PINS_PA15_PB3_PB4_PB5)){
        return SPI_ERROR;
    }
    /* Enable GPIO clocks */
    RCC_AHB2ENR |= RCC_AHB2ENR_GPIOAEN_Msk;
    if (pin_cfg == SPI1_PINS_PA15_PB3_PB4_PB5)
    {
        RCC_AHB2ENR |= RCC_AHB2ENR_GPIOBEN_Msk;
    }
    (void)RCC_AHB2ENR;

    /* Enable SPI1 clock */
    RCC_APB2ENR |= RCC_APB2ENR_SPI1_Msk;
    (void)RCC_APB2ENR;

    /* Disable SPI1 during configuration */
    SPI1_CR1 &= ~SPI1_CR1_SPE_Msk;

    spi1_pin_config = pin_cfg;

    if (pin_cfg == SPI1_PINS_PA4_PA5_PA6_PA7)
    {
        /* PA4: NSS software, GPIO output */
        GPIOA_MODER &= ~(3U << (GPIOA_SPI1_NSS_Pin_Pos1 * 2U));
        GPIOA_MODER |=  (1U << (GPIOA_SPI1_NSS_Pin_Pos1 * 2U));
        /* PA4-PA7: push-pull */
        GPIOA_OTYPER &= ~(0x0FU << GPIOA_SPI1_NSS_Pin_Pos1);
        /* PA4 high: slave deselected */
        GPIOA_BSRR = (1U << GPIOA_SPI1_NSS_Pin_Pos1);
        /* PA5, PA6, PA7: Alternate Function mode */
        GPIOA_MODER &= ~(0x3FU << (GPIOA_SPI1_SCK_Pin_Pos1 * 2U));
        GPIOA_MODER |=  (0x2AU << (GPIOA_SPI1_SCK_Pin_Pos1 * 2U));
        /* AF5 for PA5, PA6, PA7 only */
        GPIOA_AFRL &= ~(0xFFFU << (GPIOA_SPI1_SCK_Pin_Pos1 * 4U));
        GPIOA_AFRL |=  (5U << (GPIOA_SPI1_SCK_Pin_Pos1 * 4U)) | (5U << (GPIOA_SPI1_MISO_Pin_Pos1 * 4U)) |
                       (5U << (GPIOA_SPI1_MOSI_Pin_Pos1 * 4U));
        /* High speed */
        GPIOA_OSPEEDR &= ~(0xFFU << (GPIOA_SPI1_NSS_Pin_Pos1 * 2U));
        GPIOA_OSPEEDR |=  (0xAAU << (GPIOA_SPI1_NSS_Pin_Pos1 * 2U));
    }
    else
    {
        /* PA15: NSS software, GPIO output */
        GPIOA_MODER &= ~(3U << (GPIOA_SPI1_NSS_Pin_Pos2 * 2U));
        GPIOA_MODER |=  (1U << (GPIOA_SPI1_NSS_Pin_Pos2 * 2U));
        GPIOA_OTYPER &= ~(1U << GPIOA_SPI1_NSS_Pin_Pos2);
        /* PA15 high: slave deselected */
        GPIOA_BSRR = (1U << GPIOA_SPI1_NSS_Pin_Pos2);
        /* PA15 output speed */
        GPIOA_OSPEEDR &= ~(3U << (GPIOA_SPI1_NSS_Pin_Pos2 * 2U));
        GPIOA_OSPEEDR |=  (2U << (GPIOA_SPI1_NSS_Pin_Pos2 * 2U));
        /* PB3, PB4, PB5: Alternate Function mode */
        GPIOB_MODER &= ~(0x3FU << (GPIOB_SPI1_SCK_Pin_Pos2 * 2U));
        GPIOB_MODER |=  (0x2AU << (GPIOB_SPI1_SCK_Pin_Pos2 * 2U));
        GPIOB_OTYPER &= ~(7U << GPIOB_SPI1_SCK_Pin_Pos2);
        /* AF5 for PB3, PB4, PB5 */
        GPIOB_AFRL &= ~(0xFFFU << (GPIOB_SPI1_SCK_Pin_Pos2 * 4U));
        GPIOB_AFRL |=  (5U << (GPIOB_SPI1_SCK_Pin_Pos2 * 4U)) | (5U << (GPIOB_SPI1_MISO_Pin_Pos2 * 4U)) |
                       (5U << (GPIOB_SPI1_MOSI_Pin_Pos2 * 4U));
        /* High speed */
        GPIOB_OSPEEDR &= ~(0x3FU << (GPIOB_SPI1_SCK_Pin_Pos2 * 2U));
        GPIOB_OSPEEDR |=  (0x2AU << (GPIOB_SPI1_SCK_Pin_Pos2 * 2U));
    }

    /* SPI1 configuration continues below */
    /* CR1: master, software NSS, mode 0, clock prescaler */
    /* CR2: 8-bit data frame, RX threshold */

    SPI1_CR1 &= ~(SPI1_CR1_CPHA_Msk | SPI1_CR1_CPOL_Msk | SPI1_CR1_MSTR_Msk | SPI1_CR1_BR_Msk 
                | SPI1_CR1_LSBFIRST_Msk | SPI1_CR1_SSM_Msk | SPI1_CR1_SSI_Msk);
    SPI1_CR1 |= (SPI1_CR1_MSTR_Msk | (2U << SPI1_CR1_BR_Pos) | SPI1_CR1_SSM_Msk | SPI1_CR1_SSI_Msk);
    
    SPI1_CR2 &= ~(SPI1_CR2_DS_Msk | SPI1_CR2_FRXTH_Msk);
    SPI1_CR2 |= ((7U << SPI1_CR2_DS_Pos) | SPI1_CR2_FRXTH_Msk);

    SPI1_CR1 |= SPI1_CR1_SPE_Msk;
    return SPI_OK;
}

/*
1: Wait until TXE = 1: the register can accept a byte.
2: Write the byte to SPI1_DR: transmission begins.
3: Wait until RXNE = 1: a byte has been received.
4: Read SPI1_DR to retrieve the received byte.
5: Wait until BSY = 0: the transmission is complete.
*/

/**
    @brief Transmits and receives data over SPI1.

    Exchanges a specified number of bytes with the selected SPI slave.
    Transmission and reception occur simultaneously. If tx_data is NULL,
    dummy bytes (0xFF) are transmitted. If rx_data is NULL, received bytes are discarded.

    The NSS signal is driven low before the exchange and driven high
    after the transfer completes or an error occurs.
 
    @param[in]  tx_data Pointer to the transmit buffer, or NULL to transmit dummy bytes (0xFF).
    @param[out] rx_data Pointer to the receive buffer, or NULL to discard received bytes.
    @param[in]  length  Number of bytes to exchange. Must be greater than zero.
 
    @return SPI_OK      Transfer completed successfully.
    @return SPI_ERROR   Invalid transfer length.
    @return SPI_TIMEOUT A timeout occurred while waiting for the SPI peripheral.

    @note At least one of tx_data or rx_data may be NULL.
    @note The SPI peripheral must be initialized before calling this function.
    @note The slave must support keeping NSS low for the entire transfer.
*/
SPI_Status SPI1_TransmitReceive(const uint8_t *tx_data, uint8_t *rx_data, uint32_t length)
{
    SPI_Status status;
    if (length == 0U) return SPI_ERROR;

    status = SPI1_CheckErrors();
    if (status != SPI_OK) return status;

    /* Wait until the transmit buffer is empty */
    status = SPI1_WaitForTXE(SPI1_WAIT_LIMIT);
    if (status != SPI_OK) return status;

    /* Select the slave */
    SPI1_NSS_Low();

    for (uint32_t i = 0U; i < length; i++){
        /* Wait until the transmit buffer is empty */
        status = SPI1_WaitForTXE(SPI1_WAIT_LIMIT);
        if (status != SPI_OK)   goto error;

        /* Transmit real data or a dummy byte */
        SPI1_DR = (tx_data != NULL) ? tx_data[i] : 0xFFU;

        /* Wait until one byte is received */
        status = SPI1_WaitForRXNE(SPI1_WAIT_LIMIT);
        if (status != SPI_OK)   goto error;

        uint8_t received = SPI1_DR;

        if (rx_data != NULL)    rx_data[i] = received;
    }

    /* Wait until the SPI peripheral is no longer busy */
    status = SPI1_WaitForNotBusy(SPI1_WAIT_LIMIT);
    if (status != SPI_OK)   goto error;

    /* Deselect the slave */
    SPI1_NSS_High();
    return SPI1_CheckErrors();

error:
    SPI1_AbortTransfer();
    SPI1_NSS_High();
    return status;
}


/*
    Example: Reading a register using SPI1_TransmitReceive().
 
    tx_data[0]: Register read command (1 for "Reading" + register address).
    tx_data[1]: Dummy byte to generate clock pulses for data reception.
    rx_data[0]: Byte received during command transmission.
    rx_data[1]: Potential register value.
 
    Note: The command format, data position, and dummy byte value depend on the SPI device protocol.
*/