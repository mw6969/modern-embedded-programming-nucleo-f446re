#include "bsp.h"
#include "myros.h"
#include "stm32f446xx.h"

static volatile uint32_t l_tickCtr;

void BSP_init(void) {
    /* enable clock for GPIOA */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    /* configure PA5 (LD2) as output */
    GPIOA->MODER &= ~GPIO_MODER_MODER5;
    GPIOA->MODER |=  GPIO_MODER_MODER5_0;

    /* configure PA6 (external blue LED) as output */
    GPIOA->MODER &= ~GPIO_MODER_MODER6;
    GPIOA->MODER |=  GPIO_MODER_MODER6_0;

    /* configure SysTick */
    SysTick->LOAD = (SystemCoreClock / BSP_TICKS_PER_SEC) - 1U;
    SysTick->VAL  = 0U;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk
                  | SysTick_CTRL_TICKINT_Msk
                  | SysTick_CTRL_ENABLE_Msk;

    __enable_irq();
}

uint32_t BSP_tickCtr(void) {
	uint32_t tickCtr;

	__disable_irq();
	tickCtr = l_tickCtr;
	__enable_irq();

	return tickCtr;
}

void BSP_delay(uint32_t ticks) {
	uint32_t start = BSP_tickCtr();
	while((BSP_tickCtr() - start) < ticks) {
	}
}

void BSP_ledGreenOn(void) {
	GPIOA->BSRR = GPIO_BSRR_BS5;
}

void BSP_ledGreenOff(void) {
	GPIOA->BSRR = GPIO_BSRR_BR5;
}

void BSP_ledBlueOn(void) {
	GPIOA->BSRR = GPIO_BSRR_BS6;
}

void BSP_ledBlueOff(void) {
	GPIOA->BSRR = GPIO_BSRR_BR6;
}

void assert_failed(char const *file, int line) {
    (void)file;
    (void)line;
    NVIC_SystemReset();
}

void SysTick_Handler(void) {
	++l_tickCtr;
	OS_tick();
}
