#include "qassert.h" /* embedded-system-friendly assertions */
#include "bsp.h"     /* Board Support Package */
#include "stm32f446xx.h"

#define BTN_B1 (1U << 13)
#define BTN_B2 (1U << 0)
#define BTNS_MASK (BTN_B1 | BTN_B2)

/* uCOS-II application hooks =================================================*/
void App_TimeTickHook(void) {
    /* state of the button debouncing, see below */
    static struct ButtonsDebouncing {
        uint32_t depressed;
        uint32_t previous;
    } buttons = { 0U, 0U };
    uint32_t current;
    uint32_t tmp;

    TimeEvent_tick(); /* process all uC/AO time events */

    /* Perform the debouncing of the B1/B2 buttons. The algorithm for
     * debouncing adapted from the book "Embedded Systems Dictionary" by
     * Jack Ganssle and Michael Barr, page 71. It works bit-parallel over
     * a port mask, so B1 and B2 are debounced together in one pass.
     *
     * NOTE: B1 is active-low (on-board pull-up; reads HIGH at rest, LOW
     * while pressed) -- confirmed on real hardware (holding B1 keeps the
     * bomb from arming; releasing it fires the press). B2 is active-high
     * (external button + internal pull-down configured in BSP_init()).
     * XOR with BTN_B1 before masking flips just that bit so `current` is
     * uniformly "1 == pressed" for both buttons, matching the algorithm
     * below.
     */
    current = (GPIOC->IDR ^ BTN_B1) & BTNS_MASK; /* read B1 (inverted) and B2 */
    tmp = buttons.depressed; /* save the debounced depressed buttons */
    buttons.depressed |= (buttons.previous & current); /* set depressed */
    buttons.depressed &= (buttons.previous | current); /* clear released */
    buttons.previous   = current; /* update the history */
    tmp ^= buttons.depressed;     /* changed debounced depressed */
    if ((tmp & BTN_B1) != 0U) {  /* debounced B1 state changed? */
        if ((buttons.depressed & BTN_B1) != 0U) { /* is B1 depressed? */
        	/* post the "button-pressed" event from ISR */
            static Event const buttonPressedEvt = {BUTTON_PRESSED_SIG};
            Active_post(AO_TimeBomb, &buttonPressedEvt);
        }
        else { /* the button is released */
        	/* post the "button-released" event from ISR */
            static Event const buttonReleasedEvt = {BUTTON_RELEASED_SIG};
            Active_post(AO_TimeBomb, &buttonReleasedEvt);
        }
    }
    if ((tmp & BTN_B2) != 0U) {  /* debounced B2 state changed? */
        if ((buttons.depressed & BTN_B2) != 0U) { /* is B2 depressed? */
        	/* post the "button2-pressed" event from ISR */
            static Event const button2PressedEvt = {BUTTON2_PRESSED_SIG};
            Active_post(AO_TimeBomb, &button2PressedEvt);
        }
        else { /* the button is released */
        	/* post the "button2-released" event from ISR */
            static Event const button2ReleasedEvt = {BUTTON2_RELEASED_SIG};
            Active_post(AO_TimeBomb, &button2ReleasedEvt);
        }
    }
}
/*..........................................................................*/
void App_TaskIdleHook(void) {
#ifdef NDEBUG
    __WFI(); /* Wait-For-Interrupt, low-power idle */
#endif
}
/*..........................................................................*/
void App_TaskCreateHook(OS_TCB *ptcb) { (void)ptcb; }
void App_TaskDelHook    (OS_TCB *ptcb) { (void)ptcb; }
void App_TaskReturnHook (OS_TCB *ptcb) { (void)ptcb; }
void App_TaskStatHook   (void)         {}
void App_TaskSwHook     (void)         {}
void App_TCBInitHook    (OS_TCB *ptcb) { (void)ptcb; }

/* BSP functions ==============================================================*/
void BSP_init(void) {
    /* NOTE: SystemInit() has already been called from the startup code,
     * but SystemCoreClock needs to be updated */
    SystemCoreClockUpdate();

    /* enable clocks for GPIOA (LEDs) and GPIOC (user button B1) */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOCEN;

    /* configure PA5 (LD2, green) and PA6 (external blue LED) as outputs */
    GPIOA->MODER &= ~(GPIO_MODER_MODER5 | GPIO_MODER_MODER6);
    GPIOA->MODER |=  (GPIO_MODER_MODER5_0 | GPIO_MODER_MODER6_0);

    /* configure PC13 (B1) as input; B1 is active-low (on-board pull-up,
     * reads LOW while pressed) -- inversion handled in App_TimeTickHook() */
    GPIOC->MODER &= ~GPIO_MODER_MODER13;

    /* configure PC0 (B2, external button) as input with an internal
     * pull-down, so it reads a defined LOW even before the button is
     * wired up on the breadboard; B2 reads HIGH while pressed */
    GPIOC->MODER &= ~GPIO_MODER_MODER0;
    GPIOC->PUPDR &= ~GPIO_PUPDR_PUPD0;
    GPIOC->PUPDR |=  GPIO_PUPDR_PUPD0_1;
}
/*..........................................................................*/
void BSP_start(void) {
    /* set up the SysTick timer to fire at BSP_TICKS_PER_SEC rate */
    SysTick_Config(SystemCoreClock / OS_TICKS_PER_SEC);

    /* SysTick calls uC/OS-II API (OSIntEnter/OSTimeTick/OSIntExit), so it
     * must run at a kernel-aware priority level (>= the BASEPRI boundary) */
    NVIC_SetPriority(SysTick_IRQn, CPU_CFG_KA_IPL_BOUNDARY + 1U);
}
/*..........................................................................*/
void BSP_ledGreenOn(void) {
    GPIOA->BSRR = GPIO_BSRR_BS5;
}
/*..........................................................................*/
void BSP_ledGreenOff(void) {
    GPIOA->BSRR = GPIO_BSRR_BR5;
}
/*..........................................................................*/
void BSP_ledBlueOn(void) {
    GPIOA->BSRR = GPIO_BSRR_BS6;
}
/*..........................................................................*/
void BSP_ledBlueOff(void) {
    GPIOA->BSRR = GPIO_BSRR_BR6;
}
/*..........................................................................*/
_Noreturn void assert_failed(char const *file, int line) {
    (void)file;
    (void)line;
    NVIC_SystemReset();
}
/*..........................................................................*/
Q_NORETURN Q_onAssert(char const * const module, int_t const location) {
    assert_failed(module, location);
}
