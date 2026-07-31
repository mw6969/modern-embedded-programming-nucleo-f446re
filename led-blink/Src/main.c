#include "bsp.h"
#include "stm32f446xx.h"

int main(void) {
    BSP_init();

    __enable_irq();   /* GCC/CMSIS intrinsic */

    for (;;) {
        __WFI(); /* sleep until next interruption */
    }
}
