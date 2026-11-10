#include "main.h"
#include "system_stm32f4xx.h"
#include <stdint.h>

typedef union {
  float single;
  uint8_t bytes[4];
} custom_float_t;

static void SystemClock_Config(void);
static void LED_Init(void);
static void UART_Init(void);
static inline void UART_send_blocking(uint8_t *byte);
static inline void UART_rcv_blocking(uint8_t *byte);

int main(void)
{
  SystemClock_Config();
  LED_Init();
  UART_Init();

  const float ref_TAS = 50.0f;
  const float k_p = 500.0f;
  const float k_i = 30.0f;
  const float k_d = 10.0f;
  const float d = 0.1f;
  float sum_err = 0.0f;
  float old_TAS = 66.5f;
  custom_float_t rcv;
  custom_float_t snd;

  while (1)
  {
    for (int i = 0; i < 4; i++)
    {
      UART_rcv_blocking(&rcv.bytes[i]);
    }

    float TAS = rcv.single;
    float err = ref_TAS - TAS;
    float diff_err = (old_TAS - TAS) / d;
    sum_err += err * d;
    snd.single = k_p * err + k_i * sum_err + k_d * diff_err;
    old_TAS = TAS;

    uint8_t header = 'H';
    uint8_t terminator = '\0';
    UART_send_blocking(&header);
    for (int i = 0; i < 4; i++)
    {
      UART_send_blocking(&snd.bytes[i]);
    }
    UART_send_blocking(&terminator);
  }
}

static void LED_Init(void)
{
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
  GPIOA->MODER &= ~(0x3UL << 2U);
  GPIOA->MODER |= 0x1UL << 2U;
  GPIOA->OTYPER &= ~(0x1UL << 1U);
  GPIOA->OSPEEDR &= ~(0x3UL << 2U);
  GPIOA->PUPDR &= ~(0x3UL << 2U);
}

static void UART_Init(void)
{
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
  RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

  GPIOA->MODER &= ~((0x3UL << 4U) | (0x3UL << 6U));
  GPIOA->MODER |= (0x2UL << 4U) | (0x2UL << 6U);
  GPIOA->OTYPER &= ~((0x1UL << 2U) | (0x1UL << 3U));
  GPIOA->OSPEEDR |= (0x3UL << 4U) | (0x3UL << 6U);
  GPIOA->PUPDR &= ~((0x3UL << 4U) | (0x3UL << 6U));
  GPIOA->AFR[0] &= ~((0xFUL << 8U) | (0xFUL << 12U));
  GPIOA->AFR[0] |= (0x7UL << 8U) | (0x7UL << 12U);

  USART2->BRR = SystemCoreClock / 38400U;
  USART2->CR1 = USART_CR1_RE | USART_CR1_TE | USART_CR1_UE;
}

static void SystemClock_Config(void)
{
  RCC->CR |= RCC_CR_HSION;
  while ((RCC->CR & RCC_CR_HSIRDY) == 0U) {}

  RCC->CFGR &= ~(RCC_CFGR_SW | RCC_CFGR_HPRE |
                 RCC_CFGR_PPRE1 | RCC_CFGR_PPRE2);
  RCC->CFGR |= RCC_CFGR_SW_HSI;
  FLASH->ACR &= ~FLASH_ACR_LATENCY;
  SystemCoreClock = 16000000U;
}

static inline void UART_send_blocking(uint8_t *byte)
{
  while ((USART2->SR & USART_SR_TXE) == 0U) {}
  USART2->DR = *byte;
}

static inline void UART_rcv_blocking(uint8_t *byte)
{
  while ((USART2->SR & USART_SR_RXNE) == 0U) {}
  *byte = (uint8_t)USART2->DR;
}
