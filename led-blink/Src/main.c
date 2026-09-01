#include "bsp.h"
#include "qpc.h"

QXSemaphore B1_sema; /* semaphore to signal a button press */

static uint32_t stack_blinkyGreen[40];
static QXThread blinkyGreen;
static void main_blinkyGreen(QXThread * const this) {
    while (1) {
        QXSemaphore_wait(&B1_sema, /* pointer to semaphore to wait on */
                         QXTHREAD_NO_TIMEOUT); /* timeout for waiting */

    	/* even count: leaves the LED back in the OFF state after each press */
    	for (uint32_t volatile i = 1900U; i != 0U; --i) {
            BSP_ledGreenToggle();
    	}
    }
}

static uint32_t stack_blinkyBlue[40];
static QXThread blinkyBlue;
static void main_blinkyBlue(QXThread * const this) {
    while (1) {
    	BSP_sendMorseCode(0xA8EEE2A0U); /* SOS */
    	QXThread_delay(1U); /* block for 1 tick */
    }
}

static uint32_t stack_blinkyBlue2[40];
static QXThread blinkyBlue2;
static void main_blinkyBlue2(QXThread * const this) {
    while (1) {
    	BSP_sendMorseCode(0xE22A3800U); /* TEST */
    	BSP_sendMorseCode(0xE22A3800U); /* TEST */
    	QXThread_delay(5U); /* block for 5 tick */
    }
}

int main(void) {
    /* QF_init() must run first: it zeroes the AO registry, which would wipe
     * out the QXMutex that BSP_init() registers */
    QF_init();
    BSP_init();

    /* initialize the B1_sema semaphore as binary, signaling semaphore */
    QXSemaphore_init(&B1_sema, /* pointer to semaphore to initialize */
                     0U,  /* initial semaphore count (signaling semaphore) */
                     1U); /* maximum semaphore count (binary semaphore) */

    /* Initialize and start blinkyGreen thread */
    QXThread_ctor(&blinkyGreen, &main_blinkyGreen, 0);
    QXTHREAD_START(&blinkyGreen,
    		       5U, /* priority */
			       (void *)0, 0, /* message queue (not used) */
                   stack_blinkyGreen, sizeof(stack_blinkyGreen), /* stack */
				   (void *)0); /* extra parameter (not used) */

    /* Initialize and start blinkyBlue thread */
    QXThread_ctor(&blinkyBlue, &main_blinkyBlue, 0);
    QXTHREAD_START(&blinkyBlue,
    		       2U, /* priority */
			       (void *)0, 0, /* message queue (not used) */
				   stack_blinkyBlue, sizeof(stack_blinkyBlue), /* stack */
				   (void *)0); /* extra parameter (not used) */

    /* Initialize and start blinkyBlue2 thread */
    QXThread_ctor(&blinkyBlue2, &main_blinkyBlue2, 0);
    QXTHREAD_START(&blinkyBlue2,
    		       1U, /* priority */
			       (void *)0, 0, /* message queue (not used) */
				   stack_blinkyBlue2, sizeof(stack_blinkyBlue2), /* stack */
				   (void *)0); /* extra parameter (not used) */

    QF_run();

    return 0; /* unreachable: OS_run() never returns */
}
