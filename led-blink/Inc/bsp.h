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

/* turn the external blue LED (PA6) on */
void BSP_ledBlueOn(void);

/* turn the external blue LED (PA6) off */
void BSP_ledBlueOff(void);

/* single "damage control" entry point for every fault and currently-unhandled interrupt */
_Noreturn void assert_failed(char const *file, int line);

enum {
    TIMEOUT_SIG = Q_USER_SIG,
    BUTTON_PRESS_SIG,

    /* keep always last */
    MAX_SIG
};

extern QActive * const AO_Blinky1;
extern QActive * const AO_Blinky2;

#endif /* BSP_H_ */
