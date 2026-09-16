#ifndef BSP_H_
#define BSP_H_

#include "qpc.h"

/* systems clock tick [Hz] */
#define BSP_TICKS_PER_SEC 100

/* initialize the board: GPIO */
void BSP_init(void);

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
	BUTTON_PRESSED_SIG = Q_USER_SIG,
	BUTTON_RELEASED_SIG,
	TIMEOUT_SIG,
	BUTTON2_PRESSED_SIG,
	BUTTON2_RELEASED_SIG,
	MAX_SIG
};

/* Active objects */
extern QActive *AO_TimeBomb;

#endif /* BSP_H_ */
