#ifndef BSP_H_
#define BSP_H_

#include <stdint.h>

/* systems clock tick [Hz] */
#define BSP_TICKS_PER_SEC 100U

/* initialize the board: GPIO, SysTick, and enable interrupts */
void BSP_init(void);

/* get the current value of the system tick counter (thread-safe) */
uint32_t BSP_tickCtr(void);

/* delay for a specified number of system clock ticks */
void BSP_delay(uint32_t ticks);

/* turn the green LED on */
void BSP_ledGreenOn(void);

/* turn the green LED off */
void BSP_ledGreenOff(void);

/* turn the external blue LED (PA6) on */
void BSP_ledBlueOn(void);

/* turn the external blue LED (PA6) off */
void BSP_ledBlueOff(void);

/* single "damage control" entry point for every fault and currently-unhandled interrupt */
void assert_failed(char const *file, int line);

#endif /* BSP_H_ */
