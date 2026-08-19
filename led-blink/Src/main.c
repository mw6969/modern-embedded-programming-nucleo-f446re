#include <stdint.h>

#include "bsp.h"
#include "myros.h"

static uint32_t stack_blinkyGreen[40];
static OSThread blinkyGreen;
static void main_blinkyGreen(void) {
    while (1) {
    	for (uint32_t volatile i = 1500U; i != 0U; --i) {
            BSP_ledGreenOn();
            BSP_ledGreenOff();
    	}
    	OS_delay(1U); /* block for 1 tick */
    }
}

static uint32_t stack_blinkyBlue[40];
static OSThread blinkyBlue;
static void main_blinkyBlue(void) {
    while (1) {
    	for (uint32_t volatile i = 3*1500U; i != 0U; --i) {
    		BSP_ledBlueOn();
    		BSP_ledBlueOff();
    	}
    	OS_delay(50U); /* block for 50 tick */
    }
}

static uint32_t stack_idleThread[40];

int main(void) {
    BSP_init();
    OS_init(stack_idleThread, sizeof(stack_idleThread));

    OSThread_start(&blinkyGreen, 5U, &main_blinkyGreen,
        stack_blinkyGreen, sizeof(stack_blinkyGreen));
    OSThread_start(&blinkyBlue, 2U, &main_blinkyBlue,
        stack_blinkyBlue, sizeof(stack_blinkyBlue));

    OS_run();

    return 0; /* unreachable: OS_run() never returns */
}
