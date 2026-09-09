#ifndef APP_CFG_H
#define APP_CFG_H

/* task priorities (uC/OS-II: 0 = highest, OS_LOWEST_PRIO = lowest) */
#define APP_CFG_BLINKY_TASK_PRIO  (OS_LOWEST_PRIO - 4U)
#define APP_CFG_BUTTON_TASK_PRIO  (OS_LOWEST_PRIO - 3U)

/* required by uC/OS-II's own timer-management task (OS_TMR_EN in os_cfg.h) */
#define OS_TASK_TMR_PRIO          (OS_LOWEST_PRIO - 2U)

/* task stack size (# of OS_STK entries) */
#define APP_CFG_TASK_STK_SIZE  100U

/* CPU port configuration -- STM32F446RE (Cortex-M4F) implements 4 NVIC
 * priority bits (16 levels). Half the range (8-15) is reserved for
 * "kernel-aware" interrupts, which may call uC/OS-II API and get masked
 * by OS_ENTER_CRITICAL(); the rest (0-7) is for non-kernel-aware
 * interrupts, which run at higher urgency and must never touch the OS. */
#define CPU_CFG_NVIC_PRIO_BITS     4U
#define CPU_CFG_KA_IPL_BOUNDARY    8U

#define INITIAL_BLINK_TIME (OS_TICKS_PER_SEC / 4)

#endif /* APP_CFG_H */
