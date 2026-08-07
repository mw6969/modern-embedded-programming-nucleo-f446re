#ifndef MYROS_H_
#define MYROS_H_

#include <stdint.h>

/* Thread Control Block (TCB) */
typedef struct {
	void *stackPtr;
} OSThread;

/* signature every thread's entry function must match */
typedef void (*OSThreadHandler)(void);

/* register a thread with the scheduler and fake its initial stack frame */
void OSThread_start(OSThread *self,
	OSThreadHandler threadHandler,
	void *stackBuf,
	uint32_t stackSize);

/* configure the kernel (must run before OS_run()) */
void OS_init(void);

/* bootstrap the first thread and hand control to the scheduler (never returns) */
void OS_run(void);

/* advance the round-robin schedule; call this from SysTick_Handler */
void OS_tick(void);

#endif /* MYROS_H_ */
