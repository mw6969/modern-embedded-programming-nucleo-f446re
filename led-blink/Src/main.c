#include <stdbool.h>
#include "bsp.h"
#include "qassert.h"
#include "uc_ao.h"   /* UC/AO API */

Q_DEFINE_THIS_MODULE("main") /* this module name for Q_ASSERT() */

/* the TimeBomb AO */
typedef struct {
	Active super;  /* inherit Active base class  */
	enum {
		WAIT4BUTTON_STATE,
		BLINK_STATE,
		PAUSE_STATE,
		BOOM_STATE
	} state;
	TimeEvent te;
	uint32_t blink_ctr;
} TimeBomb;

static void TimeBomb_dispatch(TimeBomb * const me, Event const * const e) {
	if (e->sig == INIT_SIG) {
		BSP_ledGreenOn();
		me->state = WAIT4BUTTON_STATE;
	}

	switch (me->state) {
	    case WAIT4BUTTON_STATE: {
	    	switch (e->sig) {
				case BUTTON_PRESSED_SIG: {
					BSP_ledGreenOff();
					BSP_ledBlueOn();
					TimeEvent_arm(&me->te, OS_TICKS_PER_SEC/2, 0U);
					me->blink_ctr = 3U;
					me->state = BLINK_STATE;
					break;
				}
	    	}
	    	break;
	    }
	    case BLINK_STATE: {
	    	switch (e->sig) {
				case TIMEOUT_SIG: {
					BSP_ledBlueOff();
					TimeEvent_arm(&me->te, OS_TICKS_PER_SEC/2, 0U);
					me->state = PAUSE_STATE;
					break;
				}
	    	}
			break;
		}
	    case PAUSE_STATE: {
	    	switch (e->sig) {
				case TIMEOUT_SIG: {
					--me->blink_ctr;
					if (me->blink_ctr > 0U) {
						BSP_ledBlueOn();
						TimeEvent_arm(&me->te, OS_TICKS_PER_SEC/2, 0U);
						me->state = BLINK_STATE;
					} else {
						BSP_ledBlueOn();
						BSP_ledGreenOn();
						me->state = BOOM_STATE;
					}
					break;
				}
	    	}
			break;
		}
	    case BOOM_STATE: {
	    	break;
	    }
	    default: {
	    	Q_ASSERT(0); /* invalid state */
			break;
		}
	}
}

void TimeBomb_ctor(TimeBomb * const me) {
	Active_ctor(&me->super, (DispatchHandler)&TimeBomb_dispatch);
	TimeEvent_ctor(&me->te, TIMEOUT_SIG, &me->super);
}

OS_STK stack_timeBomb[APP_CFG_TASK_STK_SIZE]; /* task stack */
static Event *timeBomb_queue[10];
static TimeBomb timeBomb;
Active *AO_TimeBomb = &timeBomb.super;

/* the main function */
int main(void) {
    BSP_init(); /* initialize the BSP */
    OSInit();   /* initialize uC/OS-II */

    /* create AO and start it */
    TimeBomb_ctor(&timeBomb);
    Active_start(AO_TimeBomb,
    		     2U,
				 timeBomb_queue,
				 sizeof(timeBomb_queue)/sizeof(timeBomb_queue[0]),
				 stack_timeBomb,
				 sizeof(stack_timeBomb),
				 0U);

    BSP_start(); /* configure and start the interrupts */

    OSStart(); /* start the uC/OS-II scheduler... */
    return 0; /* NOTE: the scheduler does NOT return */
}
