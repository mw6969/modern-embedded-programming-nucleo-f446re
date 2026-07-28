/*
 * bsp.c -- Board Support Package.
 */

#include "bsp.h"
#include "stm32f446xx.h"

#define SYSTICK_FREQ_HZ   1U   /* LED toggles once per second */

void BSP_init(void) {
    /* Enable clock for GPIOA */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    /* Configure PA5 (LD2) as output */
    GPIOA->MODER &= ~GPIO_MODER_MODER5;
    GPIOA->MODER |=  GPIO_MODER_MODER5_0;

    /* Configure SysTick to fire at SYSTICK_FREQ_HZ */
    SysTick->LOAD = (SystemCoreClock / SYSTICK_FREQ_HZ) - 1U;
    SysTick->VAL  = 0U;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk
                  | SysTick_CTRL_TICKINT_Msk
                  | SysTick_CTRL_ENABLE_Msk;
}

/*******************************************************************************
 * assert_failed() -- single "damage control" entry point for every fault
 * and every currently-unhandled interrupt.
 *
 * TBD: this currently only resets the MCU. Once the project has a way to
 * persist or report the failure, that should happen here before the reset.
 ******************************************************************************/
void assert_failed(char const *file, int line) {
    (void)file;
    (void)line;
    NVIC_SystemReset();
}

/*******************************************************************************
 * SysTick_Handler() -- toggles the on-board LED (LD2, PA5) once per interrupt.
 ******************************************************************************/
void SysTick_Handler(void) {
    GPIOA->ODR ^= GPIO_ODR_OD5;
}
