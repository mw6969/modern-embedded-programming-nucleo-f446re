#ifndef MYROS_H_
#define MYROS_H_

#include <stdint.h>

/* Thread Control Block (TCB) */
typedef struct {
    void *stackPtr; /* stack pointer */
    uint32_t timeout; /* timeout delay down-counter */
} OSThread;

/* signature every thread's entry function must match */
typedef void (*OSThreadHandler)(void);

/* register a thread with the scheduler and fake its initial stack frame */
void OSThread_start(OSThread *self,
    OSThreadHandler threadHandler,
    void *stackBuf,
    uint32_t stackSize);

/* configure the kernel (must run before OS_run()) */
void OS_init(void *stackBuf, uint32_t stackSize);

/* callback to handle the idle condition */
void OS_onIdle(void);

/* bootstrap the first thread and hand control to the scheduler (never returns) */
void OS_run(void);

/* process all timeouts */
void OS_tick(void);

/* blocking delay */
void OS_delay(uint32_t ticks);

/* this function must be called with interrupts DISABLED */
void OS_sched(void);

/* callback to configure and start interrupts */
void OS_onStartup(void);

#endif /* MYROS_H_ */
