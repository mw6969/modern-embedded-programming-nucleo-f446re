#ifndef BSP_H_
#define BSP_H_

#include "qpc.h"

/* systems clock tick [Hz] */
#define BSP_TICKS_PER_SEC 1000U

/* initialize the board: GPIO */
void BSP_init(void);

/* turn the green LED on */
void BSP_ledGreenOn(void);

/* turn the green LED off */
void BSP_ledGreenOff(void);

/* toggle the green LED (PA5) */
void BSP_ledGreenToggle(void);

/* turn the external blue LED (PA6) on */
void BSP_ledBlueOn(void);

/* turn the external blue LED (PA6) off */
void BSP_ledBlueOff(void);

/* send a Morse-code bit pattern (MSB first) on the external blue LED (PA6);
 * serialized across callers via a priority-ceiling QXMutex */
void BSP_sendMorseCode(uint32_t bitmask);

/* single "damage control" entry point for every fault and currently-unhandled interrupt */
_Noreturn void assert_failed(char const *file, int line);

/* semaphore signaled by EXTI15_10_IRQHandler() when the user button (B1/PC13) is pressed */
extern QXSemaphore B1_sema;

#endif /* BSP_H_ */
