#ifndef USART_1
#define USART_1

#include <stdint.h>

#define USART1_BASE                     0x40013800
#define USART1_CR1                      (*(volatile uint32_t *)(USART1_BASE + 0x00U))
#define USART1_CR2                      (*(volatile uint32_t *)(USART1_BASE + 0x04U))
#define USART1_CR3                      (*(volatile uint32_t *)(USART1_BASE + 0x08U))
#define USART1_BRR                      (*(volatile uint32_t *)(USART1_BASE + 0x0CU))
#define USART1_GTPR                     (*(volatile uint32_t *)(USART1_BASE + 0x10U))
#define USART1_ISR                      (*(volatile uint32_t *)(USART1_BASE + 0x1CU))
#define USART1_ICR                      (*(volatile uint32_t *)(USART1_BASE + 0x20U))
#define USART1_RDR                      (*(volatile uint32_t *)(USART1_BASE + 0x24U))
#define USART1_TDR                      (*(volatile uint32_t *)(USART1_BASE + 0x28U))

#define RCC_BASE                        0x40021000
#define RCC_AHB2ENR                     (*(volatile uint32_t*)(RCC_BASE + 0x4C))
#define RCC_APB2ENR                     (*(volatile uint32_t*)(RCC_BASE + 0x60))
#define RCC_CCIPR                       (*(volatile uint32_t*)(RCC_BASE + 0x88))

#define RCC_AHB2ENR_GPIOAEN_Pos         0U
#define RCC_AHB2ENR_GPIOAEN_Msk         (1UL << RCC_AHB2ENR_GPIOAEN_Pos)
#define RCC_APB2ENR_USART1EN_Pos        14U
#define RCC_APB2ENR_USART1EN_Msk        (1UL << RCC_APB2ENR_USART1EN_Pos)
#define RCC_CCIPR_USART1SEL_Pos         0U
#define RCC_CCIPR_USART1SEL_Msk         (3UL << RCC_CCIPR_USART1SEL_Pos)


#define USART1_CR1_UE_Pos               0U  // Active USART1 
#define USART1_CR1_RE_Pos               2U  // Active le recepteur
#define USART1_CR1_TE_Pos               3U  // Active l'émetteur
#define USART1_CR1_PCE_Pos               10U // Désactiver la parité
#define USART1_CR1_M0_Pos               12U // Sélectionnent la longueur des données
#define USART1_CR1_OVER8_Pos            15U // Choisit le mode de suréchantillonnage
#define USART1_CR1_M1_Pos               28U // Sélectionnent la longueur des données

#define USART1_CR1_UE_Msk       (1UL << USART1_CR1_UE_Pos)
#define USART1_CR1_RE_Msk       (1UL << USART1_CR1_RE_Pos)
#define USART1_CR1_TE_Msk       (1UL << USART1_CR1_TE_Pos)
#define USART1_CR1_PCE_Msk      (1UL << USART1_CR1_PCE_Pos)
#define USART1_CR1_M0_Msk       (1UL << USART1_CR1_M0_Pos)
#define USART1_CR1_OVER8_Msk    (1UL << USART1_CR1_OVER8_Pos)
#define USART1_CR1_M1_Msk       (1UL << USART1_CR1_M1_Pos)

#define USART1_CR2_STOP_Pos     12U
#define USART1_CR2_STOP_Msk    (3UL << USART1_CR2_STOP_Pos)


#define USART1_ISR_PE_Pos       0U  // Erreur de parité
#define USART1_ISR_FE_Pos       1U  // Erreur de trame
#define USART1_ISR_NE_Pos       2U  // Erreur de bruit
#define USART1_ISR_ORE_Pos      3U  // Erreur d'overflow
#define USART1_ISR_RXNE_Pos     5U  // Une donnée est disponible à la réception.
#define USART1_ISR_TC_Pos       6U  // La transmission complète est terminée, y compris le dernier bit sur la ligne
#define USART1_ISR_TXE_Pos      7U  // Le registre de transmission accepte une nouvelle donnée
#define USART1_ISR_BUSY_Pos     16U // La liaison est occupée
#define USART1_ISR_PE_Msk       (1UL << USART1_ISR_PE_Pos)
#define USART1_ISR_FE_Msk       (1UL << USART1_ISR_FE_Pos)
#define USART1_ISR_NE_Msk       (1UL << USART1_ISR_NE_Pos)
#define USART1_ISR_ORE_Msk      (1UL << USART1_ISR_ORE_Pos)
#define USART1_ISR_RXNE_Msk     (1UL << USART1_ISR_RXNE_Pos)
#define USART1_ISR_TC_Msk       (1UL << USART1_ISR_TC_Pos)
#define USART1_ISR_TXE_Msk      (1UL << USART1_ISR_TXE_Pos)
#define USART1_ISR_BUSY_Msk     (1UL << USART1_ISR_BUSY_Pos)


#define USART1_ICR_PECF_Pos     0U  // Parity error clear flag
#define USART1_ICR_FECF_Pos     1U  // Framing error clear flag
#define USART1_ICR_NECF_Pos     2U  // Noise error clear flag
#define USART1_ICR_ORECF_Pos    3U  // Overrun error clear flag
#define USART1_ICR_TCCF_Pos     6U  // Transmission complete clear flag
#define USART1_ICR_PECF_Msk     (1UL << USART1_ICR_PECF_Pos)
#define USART1_ICR_FECF_Msk     (1UL << USART1_ICR_FECF_Pos)
#define USART1_ICR_NECF_Msk     (1UL << USART1_ICR_NECF_Pos)
#define USART1_ICR_ORECF_Msk    (1UL << USART1_ICR_ORECF_Pos)
#define USART1_ICR_TCCF_Msk     (1UL << USART1_ICR_TCCF_Pos)



#define GPIOA_BASE              0x48000000
#define GPIOA_MODER             (*(volatile uint32_t*)(GPIOA_BASE + 0x00))
#define GPIOA_OTYPER            (*(volatile uint32_t*)(GPIOA_BASE + 0x04))
#define GPIOA_OSPEEDR           (*(volatile uint32_t*)(GPIOA_BASE + 0x08))
#define GPIOA_PUPDR             (*(volatile uint32_t*)(GPIOA_BASE + 0x0C))
#define GPIOA_AFRH              (*(volatile uint32_t*)(GPIOA_BASE + 0x24))
#define USART1_TX_GPIO_Pin      9U
#define USART1_RX_GPIO_Pin      10U  

void USART1_Init(void);
void USART1_Write(uint8_t data);
uint8_t USART1_Read(void);
void USART1_TransmitString(const char *str);

#endif