#include "bsp.h"
#include "qpc.h"

QXSemaphore B1_sema; /* semaphore to signal a button press */

static uint32_t stack_blinkyGreen[40];
static QXThread blinkyGreen;
static void main_blinkyGreen(QXThread * const this) {
    while (1) {
        QXSemaphore_wait(&B1_sema, /* pointer to semaphore to wait on */
                         QXTHREAD_NO_TIMEOUT); /* timeout for waiting */

    	for (uint32_t volatile i = 1500U; i != 0U; --i) {
            BSP_ledGreenOn();
            BSP_ledGreenOff();
    	}
    }
}

static uint32_t stack_blinkyBlue[40];
static QXThread blinkyBlue;
static void main_blinkyBlue(QXThread * const this) {
    while (1) {
    	for (uint32_t volatile i = 3*1500U; i != 0U; --i) {
    		BSP_ledBlueOn();
    		BSP_ledBlueOff();
    	}
    	QXThread_delay(50U); /* block for 50 tick */
    }
}

int main(void) {
    BSP_init();
    QF_init();

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

   QF_run();

    return 0; /* unreachable: OS_run() never returns */
}
