#include "myros.h"
#include "stm32f446xx.h"
#include "qassert.h"

Q_DEFINE_THIS_MODULE("MYROS")

#define OS_MAX_THREADS 33U

static OSThread * volatile OS_curr;
static OSThread * volatile OS_next;

static OSThread *OS_thread[OS_MAX_THREADS];
static uint32_t OS_readySet; /* bitmask of threads that are ready to run */
static uint32_t OS_delayedSet; /* bitmask of threads that are delayed */

#define LOG2(x) (32U - __CLZ(x))

static OSThread idleThread;
static void main_idleThread(void) {
    while (1) {
        OS_onIdle();
    }
}

void OS_init(void *stackBuf, uint32_t stackSize) {
    /* PendSV must run at the lowest priority: a context switch must never
     * preempt (and interleave with) another exception */
    NVIC_SetPriority(PendSV_IRQn, (1U << __NVIC_PRIO_BITS) - 1U);

    /* start idleThread thread */
    OSThread_start(&idleThread, 0U, &main_idleThread, stackBuf, stackSize);
}

void OS_tick(void) {
    uint32_t workingSet = OS_delayedSet;
    while (workingSet != 0U) {
    	OSThread *t = OS_thread[LOG2(workingSet)];
    	uint32_t bit;
    	Q_ASSERT((t != (OSThread *)0) && (t->timeout != 0U));

    	bit = (1U << (t->prio - 1U));
    	--t->timeout;
    	if (t->timeout == 0U) {
    		OS_readySet |= bit; /* insert to set */
		    OS_delayedSet &= ~bit; /* remove from set */
    	}
    	workingSet &= ~bit; /* remove from working set */
    }
}

void OS_delay(uint32_t ticks) {
	uint32_t bit;
    __disable_irq();

    /* never call OS_delay from the idleThread */
    Q_REQUIRE(OS_curr != OS_thread[0]);

    OS_curr->timeout = ticks;
    bit = (1U << (OS_curr->prio - 1U));
    OS_readySet &= ~bit;
    OS_delayedSet |= bit;
    OS_sched();
    __enable_irq();
}

void OSThread_start(OSThread *self,
	uint8_t prio,
    OSThreadHandler threadHandler,
    void *stackBuf,
    uint32_t stackSize)
{
    /* stack grows down on Cortex-M; round the top down to an 8-byte
     * boundary, since AAPCS requires 8-byte stack alignment at any
     * exception/function-call boundary */
    uint32_t *sp = (uint32_t *)(((uint32_t)stackBuf + stackSize) & ~0x7U);

    /* priority must be in range and the priority level must be unused */
    Q_REQUIRE((prio < OS_MAX_THREADS) && (OS_thread[prio] == (OSThread *)0));


    /* fake the exception stack frame the CPU would push automatically
     * on exception entry (hardware order, from high to low address) */
    *(--sp) = (1U << 24);              /* xPSR: Thumb (T) bit must be set */
    *(--sp) = (uint32_t)threadHandler; /* PC: where the thread starts     */
    *(--sp) = 0x0000000EU;             /* LR  */
    *(--sp) = 0x0000000CU;             /* R12 */
    *(--sp) = 0x00000003U;             /* R3  */
    *(--sp) = 0x00000002U;             /* R2  */
    *(--sp) = 0x00000001U;             /* R1  */
    *(--sp) = 0x00000000U;             /* R0  */
    /* additionally fake R4-R11, which PendSV_Handler pops manually */
    *(--sp) = 0x0000000BU;             /* R11 */
    *(--sp) = 0x0000000AU;             /* R10 */
    *(--sp) = 0x00000009U;             /* R9  */
    *(--sp) = 0x00000008U;             /* R8  */
    *(--sp) = 0x00000007U;             /* R7  */
    *(--sp) = 0x00000006U;             /* R6  */
    *(--sp) = 0x00000005U;             /* R5  */
    *(--sp) = 0x00000004U;             /* R4  */

    self->stackPtr = sp;

    /* register the thread with the OS */
    OS_thread[prio] = self;
    self->prio = prio;
    /* make the thread ready to run */
    if (prio > 0U) {
        OS_readySet |= (1U << (prio - 1U));
    }
}

void OS_sched(void) {
    if (OS_readySet == 0U) { /* idle condition? */
    	OS_next = OS_thread[0]; /* the idle thread */
    }
    else {
    	OS_next = OS_thread[LOG2(OS_readySet)];
    	Q_ASSERT(OS_next != (OSThread *)0);
    }

    /* trigger PendSV, if needed */
    if (OS_next != OS_curr) {
        SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
    }
}

/* one-time bootstrap into OS_curr: identical trick to a PendSV context
 * restore, but done manually because there is no earlier thread to
 * return from, and thus no genuine exception to piggy-back on yet */
__attribute__((naked))
static void OS_bootstrap(void) {
    __asm volatile (
    "  .syntax unified            \n"
    "  LDR   r0,=OS_curr          \n"
    "  LDR   r0,[r0]              \n" /* r0 = OS_curr             */
    "  LDR   r0,[r0]              \n" /* r0 = OS_curr->stackPtr   */
    "  LDMIA r0!,{r4-r11}         \n" /* pop the faked R4-R11     */
    "  MSR   PSP,r0               \n" /* PSP = rest of the frame  */
    "  MOVS  r0,#0x02             \n"
    "  MSR   CONTROL,r0           \n" /* switch SP to PSP, stay privileged */
    "  ISB                        \n"
    "  BL    OS_onStartup         \n" /* PSP is live now: safe to start ticking */
    "  POP   {r0-r3}              \n"
    "  POP   {r4}                 \n" /* faked R12, discarded */
    "  POP   {lr}                 \n" /* faked LR, discarded  */
    "  POP   {pc}                 \n" /* -> jump into the thread */
    );
}

void OS_run(void) {
    /* pick the highest-priority ready thread to bootstrap into */
    OS_curr = OS_thread[LOG2(OS_readySet)];

    OS_bootstrap();

    /* never reached: OS_bootstrap() jumps straight into OS_curr */
    Q_ERROR();
}

/* real context switch: called only after OS_run() has bootstrapped the
 * first thread, so the interrupted context is always Thread-mode-on-PSP
 * and BX LR below is guaranteed to return the same way */
__attribute__((naked))
void PendSV_Handler(void) {
    __asm volatile (
    "  .syntax unified            \n"
    "  CPSID   I                  \n"
    "  LDR     r1,=OS_curr        \n"
    "  LDR     r1,[r1]            \n" /* r1 = OS_curr               */
    "  MRS     r0,PSP             \n" /* r0 = outgoing thread's PSP */
    "  STMDB   r0!,{r4-r11}       \n" /* save R4-R11 of outgoing thread */
    "  STR     r0,[r1]            \n" /* OS_curr->stackPtr = r0     */
    "  LDR     r0,=OS_next        \n"
    "  LDR     r0,[r0]            \n" /* r0 = OS_next               */
    "  LDR     r1,=OS_curr        \n"
    "  STR     r0,[r1]            \n" /* OS_curr = OS_next          */
    "  LDR     r2,[r0]            \n" /* r2 = OS_next->stackPtr     */
    "  LDMIA   r2!,{r4-r11}       \n" /* restore R4-R11 of incoming thread */
    "  MSR     PSP,r2             \n"
    "  CPSIE   I                  \n"
    "  BX      lr                 \n" /* hw restores R0-R3,R12,LR,PC,xPSR */
    );
}
