#include <stdint.h>

#include "bsp.h"
#include "myros.h"

static uint32_t stack_blinkyGreen[40];
static OSThread blinkyGreen;
static void main_blinkyGreen(void) {
    while (1) {
        BSP_ledGreenOn();
        OS_delay(BSP_TICKS_PER_SEC / 4U);
        BSP_ledGreenOff();
        OS_delay(BSP_TICKS_PER_SEC * 3U / 4U);
    }
}

static uint32_t stack_blinkyBlue[40];
static OSThread blinkyBlue;
static void main_blinkyBlue(void) {
    while (1) {
        BSP_ledBlueOn();
        OS_delay(BSP_TICKS_PER_SEC / 8U);
        BSP_ledBlueOff();
        OS_delay(BSP_TICKS_PER_SEC / 8U);
    }
}

static uint32_t stack_idleThread[40];

int main(void) {
    BSP_init();
    OS_init(stack_idleThread, sizeof(stack_idleThread));

    OSThread_start(&blinkyGreen, &main_blinkyGreen,
        stack_blinkyGreen, sizeof(stack_blinkyGreen));
    OSThread_start(&blinkyBlue, &main_blinkyBlue,
        stack_blinkyBlue, sizeof(stack_blinkyBlue));

    OS_run();

    return 0; /* unreachable: OS_run() never returns */
}
