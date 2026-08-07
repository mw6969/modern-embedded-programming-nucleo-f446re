#include <stdint.h>

#include "bsp.h"
#include "myros.h"

static uint32_t stack_blinkyGreen[40];
static OSThread blinkyGreen;

static uint32_t stack_blinkyBlue[40];
static OSThread blinkyBlue;

static void main_blinkyGreen(void) {
	while (1) {
    	BSP_ledGreenOn();
    	BSP_delay(BSP_TICKS_PER_SEC / 4U);
    	BSP_ledGreenOff();
    	BSP_delay(BSP_TICKS_PER_SEC * 3U / 4U);
	}
}

static void main_blinkyBlue(void) {
	while (1) {
    	BSP_ledBlueOn();
    	BSP_delay(BSP_TICKS_PER_SEC / 8U);
    	BSP_ledBlueOff();
    	BSP_delay(BSP_TICKS_PER_SEC / 8U);
	}
}

int main(void) {
    BSP_init();
    OS_init();

    OSThread_start(&blinkyGreen, &main_blinkyGreen,
    	stack_blinkyGreen, sizeof(stack_blinkyGreen));
    OSThread_start(&blinkyBlue, &main_blinkyBlue,
    	stack_blinkyBlue, sizeof(stack_blinkyBlue));

    OS_run();

    return 0; /* unreachable: OS_run() never returns */
}
