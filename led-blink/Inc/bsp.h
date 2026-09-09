#ifndef BSP_H_
#define BSP_H_

#include "uc_ao.h" /* UC/AO API */

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

enum EventSignals {
	BUTTON_PRESSED_SIG = USER_SIG,
	BUTTON_RELEASED_SIG,
	TIMEOUT_SIG,
};

/* Active objects */
extern Active *AO_BlinkyButton;

#endif /* BSP_H_ */
