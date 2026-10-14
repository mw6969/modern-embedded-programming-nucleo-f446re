#ifndef MY_PRINTF_H_
#define MY_PRINTF_H_

#define SPY

#ifdef SPY /* tracing enabled? */

#include <stdio.h>

#define MY_PRINTF(format_, ...) printf(format_, ##__VA_ARGS__)
#define MY_PRINTF_INIT() uart2_init()

void uart2_init();

#else /* tracing disabled */

#define MY_PRINTF(format_, ...) (0)
#define MY_PRINTF_INIT() ((void)0)

#endif /* SPY */

#endif /* MY_PRINTF_H_ */
