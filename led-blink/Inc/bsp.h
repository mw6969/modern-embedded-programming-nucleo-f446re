#ifndef BSP_H_
#define BSP_H_

#include "ucos_ii.h" /* uC/OS-II API, port and compile-time configuration */

/* systems clock tick [Hz] */
#define BSP_TICKS_PER_SEC OS_TICKS_PER_SEC

/* initialize the board: GPIO */
void BSP_init(void);

/* configure and enable the SysTick interrupt (must run after OSInit()) */
void BSP_start(void);

/* turn the green LED (PA5) on */
void BSP_ledGreenOn(void);

/* turn the green LED (PA5) off */
void BSP_ledGreenOff(void);

/* turn the external blue LED (PA6) on */
void BSP_ledBlueOn(void);

/* turn the external blue LED (PA6) off */
void BSP_ledBlueOff(void);

/* single "damage control" entry point for every fault and currently-unhandled interrupt */
_Noreturn void assert_failed(char const *file, int line);

/* global RTOS objects, signaled from App_TimeTickHook()'s button debouncing */
extern OS_EVENT *BSP_semaPress;   /* user button (B1/PC13) was just pressed */
extern OS_EVENT *BSP_semaRelease; /* user button (B1/PC13) was just released */

#endif /* BSP_H_ */
