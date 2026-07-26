#include <stdint.h>

#include "stm32f446xx.h"

// Give initial values to a variables and its definitions
int16_t x = -1;
uint32_t y = GPIO_BSRR_BS5;

int16_t sqt[] = {
    1*1,
    2*2,
    3*3,
    4*4
};

typedef struct {
	uint8_t y;
	uint16_t x;
} Point;

Point p1 = {
	123U,
	0x1234U
};
Point p2;

typedef struct {
    Point top_left;
    Point bottom_right;
} Window;

typedef struct {
    Point corners[3];
} Triangle;

Window w = {
	{ 123U, 0x1234U },
	{ 234U, 0X6789U }
};
Window w2;
Triangle t;

static void delay() {
	volatile int counter = 0;
    while (counter < 1000000) {
        ++counter;
    }
}

int main() {
    // Enable clock for GPIOA
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    // Configure PA5 as output
	// In CMSIS: clear 2 bits of MODER5 (bits 10 and 11), then set to 01 (Output)
    GPIOA->MODER &= ~GPIO_MODER_MODER5; // Resets bits [11:10] to 00
    GPIOA->MODER |=  GPIO_MODER_MODER5_0; // Sets bit 0 (mode 01 = Output)

    // Blink loop
    for (;;) {
    	// Lights up pin PA5 -> GPIO_BSRR_BS5 stands for Bit Set 5
        GPIOA->BSRR = GPIO_BSRR_BS5;
        delay();

        // Turn off pin PA5 -> GPIO_BSRR_BR5 stands for Bit Reset 5
        GPIOA->BSRR = GPIO_BSRR_BR5;
        delay();
    }
}
