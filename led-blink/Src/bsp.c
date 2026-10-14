#include "bsp.h"
#include "my_printf.h"
#include "stm32f446xx.h"

void SysTick_Handler(void) {
	QXK_ISR_ENTRY(); /* inform QXK about entering an ISR */

    QF_TICK_X(0U, (void *)0); /* process time events for  */

    QXK_ISR_EXIT(); /* inform QXK about exiting an ISR */
}

/* user button (B1/PC13): posts BUTTON_PRESS_SIG on press (falling edge) */
void EXTI15_10_IRQHandler(void) {
    QXK_ISR_ENTRY(); /* inform QXK about entering an ISR */

    if ((EXTI->PR & EXTI_PR_PR13) != 0U) {
        EXTI->PR = EXTI_PR_PR13; /* clear the pending bit (write 1 to clear) */
        static QEvt const buttonPressedEvt = QEVT_INITIALIZER(BUTTON_PRESS_SIG);
        QACTIVE_POST(AO_Blinky2, &buttonPressedEvt, 0U);
        MY_PRINTF("BTN %s %d\n", "B1", 1);
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

    /* configure PC13 as input (B1 is active-low: on-board pull-up holds it
     * HIGH at rest, pressing pulls it LOW) */
    GPIOC->MODER &= ~GPIO_MODER_MODER13;

    /* route EXTI13 to GPIOC pin 13 */
    SYSCFG->EXTICR[3] &= ~SYSCFG_EXTICR4_EXTI13;
    SYSCFG->EXTICR[3] |=  SYSCFG_EXTICR4_EXTI13_PC;

    /* EXTI13: trigger on the falling edge (button press pulls PC13 LOW),
     * unmask the interrupt */
    EXTI->FTSR |=  EXTI_FTSR_TR13;
    EXTI->RTSR &= ~EXTI_RTSR_TR13;
    EXTI->IMR  |=  EXTI_IMR_MR13;

    /* enable EXTI15_10 in the NVIC at a QF-aware priority */
    NVIC_SetPriority(EXTI15_10_IRQn, QF_AWARE_ISR_CMSIS_PRI);
    NVIC_ClearPendingIRQ(EXTI15_10_IRQn);
    NVIC_EnableIRQ(EXTI15_10_IRQn);

    MY_PRINTF_INIT();
}

void BSP_ledGreenOn(void) {
    GPIOA->BSRR = GPIO_BSRR_BS5;
    MY_PRINTF("LED-%s is %d\n", "green", 1);
}

void BSP_ledGreenOff(void) {
    GPIOA->BSRR = GPIO_BSRR_BR5;
    MY_PRINTF("LED-%s is %d\n", "green", 0);
}

void BSP_ledBlueOn(void) {
    GPIOA->BSRR = GPIO_BSRR_BS6;
    MY_PRINTF("LED-%s is %d\n", "blue", 1);
}

void BSP_ledBlueOff(void) {
    GPIOA->BSRR = GPIO_BSRR_BR6;
    MY_PRINTF("LED-%s is %d\n", "blue", 0);
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

#ifdef SPY

/* support for MY_PRINTF() over USART2 (ST-LINK Virtual COM Port) ============ */
#define UART_BAUD_RATE 115200U

/* newlib's printf() -> _write() (syscalls.c) -> __io_putchar() */
int __io_putchar(int c) {
    /* busy-wait until the transmit data register is empty */
    while ((USART2->SR & USART_SR_TXE) == 0U) {
    }
    USART2->DR = (uint32_t)c;
    return c;
}

void uart2_init() {
    /* enable clock for USART2 (APB1); GPIOA clock is enabled in BSP_init() */
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

    /* PA2 (TX) and PA3 (RX) -> alternate function mode (0b10) */
    GPIOA->MODER &= ~(GPIO_MODER_MODER2 | GPIO_MODER_MODER3);
    GPIOA->MODER |=  (GPIO_MODER_MODER2_1 | GPIO_MODER_MODER3_1);

    /* select AF7 (USART2) on PA2 and PA3 */
    GPIOA->AFR[0] &= ~((0xFU << (2U * 4U)) | (0xFU << (3U * 4U)));
    GPIOA->AFR[0] |=  ((7U   << (2U * 4U)) | (7U   << (3U * 4U)));

    /* baud rate: PCLK1 = SystemCoreClock (HSI, no prescalers), 16x oversampling */
    USART2->BRR = (SystemCoreClock + (UART_BAUD_RATE / 2U)) / UART_BAUD_RATE;

    /* 8-N-1 (default CR1/CR2), enable transmitter + receiver + USART */
    USART2->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
}

#endif /* SPY */
