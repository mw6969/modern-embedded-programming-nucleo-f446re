#include "bsp.h"
#include "stm32f446xx.h"

static QXMutex Morse_mutex;

void SysTick_Handler(void) {
	QXK_ISR_ENTRY(); /* inform QXK about entering an ISR */

    QF_TICK_X(0U, (void *)0); /* process time events for  */

    QXK_ISR_EXIT(); /* inform QXK about exiting an ISR */
}

/* user button (B1/PC13): signals B1_sema on press (rising edge) */
void EXTI15_10_IRQHandler(void) {
    QXK_ISR_ENTRY(); /* inform QXK about entering an ISR */

    if ((EXTI->PR & EXTI_PR_PR13) != 0U) {
        EXTI->PR = EXTI_PR_PR13; /* clear the pending bit (write 1 to clear) */
        QXSemaphore_signal(&B1_sema);
    }

    QXK_ISR_EXIT(); /* inform QXK about exiting an ISR */
}

void BSP_init(void) {
    /* enable clock for GPIOA */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    /* configure PA5 (LD2) as output */
    GPIOA->MODER &= ~GPIO_MODER_MODER5;
    GPIOA->MODER |=  GPIO_MODER_MODER5_0;

    /* configure PA6 (external blue LED) as output */
    GPIOA->MODER &= ~GPIO_MODER_MODER6;
    GPIOA->MODER |=  GPIO_MODER_MODER6_0;

    /* enable clocks for GPIOC (user button B1 on PC13) and SYSCFG (EXTI routing) */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    /* configure PC13 as input (button pulls the line high when pressed) */
    GPIOC->MODER &= ~GPIO_MODER_MODER13;

    /* route EXTI13 to GPIOC pin 13 */
    SYSCFG->EXTICR[3] &= ~SYSCFG_EXTICR4_EXTI13;
    SYSCFG->EXTICR[3] |=  SYSCFG_EXTICR4_EXTI13_PC;

    /* EXTI13: trigger on the rising edge (button press), unmask the interrupt */
    EXTI->RTSR |=  EXTI_RTSR_TR13;
    EXTI->FTSR &= ~EXTI_FTSR_TR13;
    EXTI->IMR  |=  EXTI_IMR_MR13;

    /* enable EXTI15_10 in the NVIC at a QF-aware priority */
    NVIC_SetPriority(EXTI15_10_IRQn, QF_AWARE_ISR_CMSIS_PRI);
    NVIC_ClearPendingIRQ(EXTI15_10_IRQn);
    NVIC_EnableIRQ(EXTI15_10_IRQn);

    QXMutex_init(&Morse_mutex, 6U); /* priority ceiling 6 */
}

void BSP_ledGreenOn(void) {
    GPIOA->BSRR = GPIO_BSRR_BS5;
}

void BSP_ledGreenOff(void) {
    GPIOA->BSRR = GPIO_BSRR_BR5;
}

void BSP_ledGreenToggle(void) {
    QF_CRIT_STAT;
    QF_CRIT_ENTRY();
    GPIOA->ODR ^= GPIO_ODR_OD5;
    QF_CRIT_EXIT();
}

void BSP_ledBlueOn(void) {
    GPIOA->BSRR = GPIO_BSRR_BS6;
}

void BSP_ledBlueOff(void) {
    GPIOA->BSRR = GPIO_BSRR_BR6;
}

void BSP_sendMorseCode(uint32_t bitmask) {
    uint32_t volatile delay_ctr;
    enum { DOT_DELAY = 150 };

    QXMutex_lock(&Morse_mutex, QXTHREAD_NO_TIMEOUT); /* timeout for waiting */

    /* MSB first, one dot-length slot per bit: 1 = LED on, 0 = LED off;
     * a run of 3 set bits reads as a dash, a lone set bit as a dot */
    for (; bitmask != 0U; bitmask <<= 1) {
        if ((bitmask & (1U << 31)) != 0U) {
            BSP_ledBlueOn();
        }
        else {
            BSP_ledBlueOff();
        }
        for (delay_ctr = DOT_DELAY; delay_ctr != 0U; --delay_ctr) {
        }
    }
    BSP_ledBlueOff();
    for (delay_ctr = 7*DOT_DELAY; delay_ctr != 0U; --delay_ctr) { /* word gap */
    }

    QXMutex_unlock(&Morse_mutex);
}

void QF_onStartup(void) {
    /* configure SysTick */
    SysTick->LOAD = (SystemCoreClock / BSP_TICKS_PER_SEC) - 1U;
    SysTick->VAL  = 0U;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk
                  | SysTick_CTRL_TICKINT_Msk
                  | SysTick_CTRL_ENABLE_Msk;

    __enable_irq();
}

void QF_onCleanup(void) {
}

void QXK_onIdle(void) {
    __WFI(); /* stop the CPU and wait for interrupt */
}

_Noreturn void assert_failed(char const *file, int line) {
    (void)file;
    (void)line;
    NVIC_SystemReset();
}

Q_NORETURN Q_onError(char const * const module, int_t const id) {
    assert_failed(module, id);
}
