#include "uart.h"

#include <stddef.h>
#include <stdint.h>

/* Check that the public register structure matches STM32L4 layout. */
typedef char UART_CR1_offset_check[(offsetof(UART_Registers, CR1) == 0x00U) ? 1 : -1];
typedef char UART_CR2_offset_check[(offsetof(UART_Registers, CR2) == 0x04U) ? 1 : -1];
typedef char UART_BRR_offset_check[(offsetof(UART_Registers, BRR) == 0x0CU) ? 1 : -1];
typedef char UART_ISR_offset_check[(offsetof(UART_Registers, ISR) == 0x1CU) ? 1 : -1];
typedef char UART_ICR_offset_check[(offsetof(UART_Registers, ICR) == 0x20U) ? 1 : -1];
typedef char UART_RDR_offset_check[(offsetof(UART_Registers, RDR) == 0x24U) ? 1 : -1];
typedef char UART_TDR_offset_check[(offsetof(UART_Registers, TDR) == 0x28U) ? 1 : -1];

/* Private STM32L476xx configuration */

#define UART_WAIT_LIMIT                 1000000UL
#define UART_DEFAULT_BRR                35UL

#define RCC_BASE_ADDR                   0x40021000UL
#define RCC_AHB2ENR_ADDR                (RCC_BASE_ADDR + 0x4CUL)
#define RCC_APB1ENR1_ADDR               (RCC_BASE_ADDR + 0x58UL)
#define RCC_APB2ENR_ADDR                (RCC_BASE_ADDR + 0x60UL)
#define RCC_CCIPR_ADDR                  (RCC_BASE_ADDR + 0x88UL)

#define REG32(address)                 (*(volatile uint32_t *)(uintptr_t)(address))

#define RCC_AHB2ENR_GPIOAEN_Msk         (1UL << 0U)
#define RCC_APB1ENR1_USART2EN_Msk       (1UL << 17U)
#define RCC_APB2ENR_USART1EN_Msk        (1UL << 14U)

#define RCC_CCIPR_USART1SEL_Pos         0U
#define RCC_CCIPR_USART2SEL_Pos         2U
#define RCC_CCIPR_USARTSEL_Msk          0x3UL

/* USART_CR1 bits */
#define USART_CR1_UE_Msk                (1UL << 0U)
#define USART_CR1_RE_Msk                (1UL << 2U)
#define USART_CR1_TE_Msk                (1UL << 3U)
#define USART_CR1_PCE_Msk               (1UL << 10U)
#define USART_CR1_M0_Msk                (1UL << 12U)
#define USART_CR1_OVER8_Msk             (1UL << 15U)
#define USART_CR1_M1_Msk                (1UL << 28U)

/* USART_CR2 bits */
#define USART_CR2_STOP_Msk              (3UL << 12U)

/* USART_ISR bits */
#define USART_ISR_RXNE_Msk              (1UL << 5U)
#define USART_ISR_TXE_Msk               (1UL << 7U)

/* GPIOA base address and register layout (STM32L476xx). */
#define GPIOA_BASE_ADDR                 0x48000000UL

typedef struct
{
    volatile uint32_t MODER;          /* Offset: 0x00 */
    volatile uint32_t OTYPER;         /* Offset: 0x04 */
    volatile uint32_t OSPEEDR;        /* Offset: 0x08 */
    volatile uint32_t PUPDR;          /* Offset: 0x0C */
    volatile uint32_t IDR;            /* Offset: 0x10 */
    volatile uint32_t ODR;            /* Offset: 0x14 */
    volatile uint32_t BSRR;           /* Offset: 0x18 */
    volatile uint32_t LCKR;           /* Offset: 0x1C */
    volatile uint32_t AFR[2];         /* Offsets: 0x20, 0x24 */

} GPIO_Registers;

typedef struct
{
    UART_Instance instance_id;
    uint32_t peripheral_base_addr;
    uint32_t peripheral_clock_enable_addr;
    uint32_t peripheral_clock_enable_mask;
    uint8_t clock_selection_pos;
    uint8_t tx_pin;
    uint8_t rx_pin;
    uint8_t alternate_function;
} UART_HardwareConfig;

/* Only the pin configurations present in the original drivers are included. */
static const UART_HardwareConfig UART_HARDWARE_CONFIGS[] =
{
    /* USART1: PA9 TX, PA10 RX, AF7 */
    {UART_INSTANCE_1, 0x40013800UL, RCC_APB2ENR_ADDR, RCC_APB2ENR_USART1EN_Msk, RCC_CCIPR_USART1SEL_Pos,
        9U, 10U, 7U},
    /* USART2: PA2 TX, PA3 RX, AF7 */
    {UART_INSTANCE_2, 0x40004400UL, RCC_APB1ENR1_ADDR, RCC_APB1ENR1_USART2EN_Msk, RCC_CCIPR_USART2SEL_Pos,
        2U, 3U, 7U}
};

/* Private helper declarations */

static const UART_HardwareConfig *UART_FindHardwareConfig(UART_Instance instance);
static void GPIO_ConfigUART(GPIO_Registers *gpio, uint8_t tx_pin, uint8_t rx_pin, uint8_t alternate_function);
static void GPIO_SetAlternateFunction(GPIO_Registers *gpio, uint8_t pin, uint8_t alternate_function);
static UART_Status UART_WaitForFlag(UART_Handle *huart, uint32_t flag_mask, uint32_t timeout);

/* Private hardware helpers */

static const UART_HardwareConfig *UART_FindHardwareConfig(UART_Instance instance)
{
    size_t i;
    const size_t count = sizeof(UART_HARDWARE_CONFIGS)/sizeof(UART_HARDWARE_CONFIGS[0]);

    for (i = 0U; i < count; i++)
    {
        if (UART_HARDWARE_CONFIGS[i].instance_id == instance)
        {
            return &UART_HARDWARE_CONFIGS[i];
        }
    }

    return NULL;
}

static void GPIO_SetAlternateFunction(GPIO_Registers *gpio, uint8_t pin, uint8_t alternate_function)
{
    const uint32_t register_index = (uint32_t)pin / 8U;
    const uint32_t shift = ((uint32_t)pin % 8U) * 4U;
    const uint32_t mask = 0xFUL << shift;

    gpio->AFR[register_index] &= ~mask;
    gpio->AFR[register_index] |= (uint32_t)alternate_function << shift;
}

static void GPIO_ConfigUART(GPIO_Registers *gpio, uint8_t tx_pin, uint8_t rx_pin, uint8_t alternate_function)
{
    const uint32_t pin_mode_mask = (3UL << ((uint32_t)tx_pin * 2U)) | (3UL << ((uint32_t)rx_pin * 2U));
    const uint32_t tx_mask = 1UL << tx_pin;
    const uint32_t pin_pull_mask = (3UL << ((uint32_t)tx_pin * 2U)) | (3UL << ((uint32_t)rx_pin * 2U));

    /* Select alternate-function mode for TX and RX. */
    gpio->MODER &= ~pin_mode_mask;
    gpio->MODER |= (2UL << ((uint32_t)tx_pin * 2U)) | (2UL << ((uint32_t)rx_pin * 2U));

    /* TX is push-pull, matching the original implementation. */
    gpio->OTYPER &= ~tx_mask;

    /* Configure both pins for very-high speed. */
    gpio->OSPEEDR &= ~pin_mode_mask;
    gpio->OSPEEDR |= pin_mode_mask;

    /* Clear both pull configurations and enable pull-up on RX. */
    gpio->PUPDR &= ~pin_pull_mask;
    gpio->PUPDR |= 1UL << ((uint32_t)rx_pin * 2U);

    GPIO_SetAlternateFunction(gpio, tx_pin, alternate_function);
    GPIO_SetAlternateFunction(gpio, rx_pin, alternate_function);
}

/* Wait until TXE or RXNE is asserted, with a finite polling limit. */
static UART_Status UART_WaitForFlag(UART_Handle *huart, uint32_t flag_mask, uint32_t timeout)
{
    if ((huart == NULL) || (huart->instance == NULL))
    {
        return UART_ERROR;
    }

    while ((huart->instance->ISR & flag_mask) == 0U)
    {
        if (timeout == 0U)
        {
            return UART_TIMEOUT;
        }

        timeout--;
    }

    return UART_OK;
}

/* Public initialization API */

UART_Status UART_Init(UART_Handle *huart, UART_Instance instance)
{
    const UART_HardwareConfig *config;
    GPIO_Registers *gpio;
    uint32_t ccipr_mask;
    volatile uint32_t *peripheral_clock_enable;

    if (huart == NULL)
    {
        return UART_ERROR;
    }

    huart->instance = NULL;

    config = UART_FindHardwareConfig(instance);
    if (config == NULL)
    {
        return UART_ERROR;
    }

    huart->instance = (UART_Registers *)(uintptr_t)config->peripheral_base_addr;
    gpio = (GPIO_Registers *)(uintptr_t)GPIOA_BASE_ADDR;

    /* 1. Enable GPIOA clock. */
    REG32(RCC_AHB2ENR_ADDR) |= RCC_AHB2ENR_GPIOAEN_Msk;
    (void)REG32(RCC_AHB2ENR_ADDR);

    /* 2. Configure TX/RX pins using AF7. */
    GPIO_ConfigUART(gpio, config->tx_pin, config->rx_pin, config->alternate_function);

    /* 3. Select the peripheral bus clock, matching the old drivers. */
    ccipr_mask = RCC_CCIPR_USARTSEL_Msk << config->clock_selection_pos;
    REG32(RCC_CCIPR_ADDR) &= ~ccipr_mask;

    /* 4. Enable the selected USART peripheral clock. */
    peripheral_clock_enable = &REG32(config->peripheral_clock_enable_addr);
    *peripheral_clock_enable |= config->peripheral_clock_enable_mask;
    (void)*peripheral_clock_enable;

    /* 5. Configure USART while it is disabled. */
    huart->instance->CR1 &= ~USART_CR1_UE_Msk;

    /* 8 data bits, no parity, oversampling by 16. */
    huart->instance->CR1 &= ~(USART_CR1_M1_Msk |  USART_CR1_M0_Msk |  USART_CR1_PCE_Msk |  USART_CR1_OVER8_Msk);

    /* Enable transmitter and receiver. */
    huart->instance->CR1 |= USART_CR1_RE_Msk | USART_CR1_TE_Msk;

    /* Preserve the BRR value used in both original drivers. */
    huart->instance->BRR = UART_DEFAULT_BRR;

    /* One stop bit. */
    huart->instance->CR2 &= ~USART_CR2_STOP_Msk;

    /* Enable USART. */
    huart->instance->CR1 |= USART_CR1_UE_Msk;

    return UART_OK;
}

/* Public byte-transmit API */

UART_Status UART_Write(UART_Handle *huart, uint8_t data)
{
    UART_Status status;

    if ((huart == NULL) || (huart->instance == NULL))
    {
        return UART_ERROR;
    }

    status = UART_WaitForFlag(huart, USART_ISR_TXE_Msk, UART_WAIT_LIMIT);
    if (status != UART_OK)
    {
        return status;
    }

    huart->instance->TDR = data;
    return UART_OK;
}

/* Public byte-receive API */

UART_Status UART_Read(UART_Handle *huart, uint8_t *data)
{
    UART_Status status;

    if ((huart == NULL) || (huart->instance == NULL) || (data == NULL))
    {
        return UART_ERROR;
    }

    status = UART_WaitForFlag(huart, USART_ISR_RXNE_Msk, UART_WAIT_LIMIT);
    if (status != UART_OK)
    {
        return status;
    }

    *data = (uint8_t)huart->instance->RDR;
    return UART_OK;
}

/* Public string-transmit API */

UART_Status UART_TransmitString(UART_Handle *huart, const char *str)
{
    UART_Status status;

    if ((huart == NULL) || (huart->instance == NULL) || (str == NULL))
    {
        return UART_ERROR;
    }

    while (*str != '\0')
    {
        status = UART_Write(huart, (uint8_t)*str);
        if (status != UART_OK)
        {
            return status;
        }

        str++;
    }

    return UART_OK;
}
