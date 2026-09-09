# LED Blink → uC/OS-II RTOS → uC/AO Active Objects

Bare-metal on the STM32 Nucleo-F446RE. Started as a simple LED blink, grew
into a hand-written minimal preemptive kernel (`MyROS`), then QP/C's **QXK**
dual-mode kernel, then a step back to "superloop + classic RTOS" on Silicon
Labs' uC/OS-II, and now uses **uC/AO** — Quantum Leaps' minimal Active
Object layer on top of uC/OS-II — to express the same blinky/button problem
as a single event-driven active object instead of two raw tasks sharing
state through a semaphore/mutex pair. No HAL, no CubeMX-generated drivers —
registers are hand-rolled.

Progression is tracked in the commit history — see `git log` for this folder.

## What it does

One active object, `BlinkyButton`, reacting to two kinds of events:

| Event | Source | Effect |
|---|---|---|
| `TIMEOUT_SIG` | `TimeEvent` armed by the AO itself | toggles the onboard green LED (LD2/PA5); on-time is `blink_time` ticks, off-time is `blink_time * 3` (25% duty cycle) |
| `BUTTON_PRESSED_SIG` / `BUTTON_RELEASED_SIG` | posted from `App_TimeTickHook()` | lights the external blue LED (PA6/D12) while B1 is held; on press, halves `blink_time` (wraps back to `INITIAL_BLINK_TIME` once it underflows to 0) |

The button (B1 / PC13) is **not** wired to an EXTI interrupt — it's
debounced by polling inside `App_TimeTickHook()` (called every SysTick
tick), using the classic Ganssle/Barr debounce algorithm. Debounced
press/release edges are posted straight into the AO's own event queue via
`Active_post()`, replacing the old binary semaphores. `App_TimeTickHook()`
also drives `TimeEvent_tick()` every tick, which is what actually expires
the AO's armed `TimeEvent` and posts `TIMEOUT_SIG` — since there's now only
one task, `blink_time` needs no mutex; it's private state read and written
only from inside `BlinkyButton_dispatch()`.

Also includes startup-code hardening: CPU fault handlers (NMI, HardFault,
MemManage, BusFault, UsageFault) and every unused peripheral IRQ are routed
to a controlled `NVIC_SystemReset()` instead of silently hanging.
`SysTick_Handler` and `PendSV_Handler` are owned by uC/OS-II's ARMv7-M/GNU
port, not by application code.

## Hardware & Tools

| | |
|---|---|
| Board | STM32 Nucleo-64 F446RE |
| MCU | STM32F446RE, ARM Cortex-M4F |
| Onboard LED | LD2 — GPIOA, pin 5 (PA5) |
| External LED | PA6 / Arduino header D12 → resistor (~220-330 Ω) → LED anode; cathode → GND |
| User button | B1 (blue) — PC13, polled (not interrupt-driven), reads HIGH while pressed |
| Tick source | SysTick, 100 Hz (`OS_TICKS_PER_SEC`; `HCLK` = 16 MHz HSI, no PLL configured) |
| IDE / Toolchain | STM32CubeIDE (GCC ARM) |
| Debugger | ST-LINK/V2-1 (on-board) |

## Project structure

```
led-blink/
├── Inc/
│   ├── bsp.h                          # board support package interface
│   ├── app_cfg.h / os_cfg.h           # this project's uC/OS-II configuration
│   ├── uc_ao.h                        # uC/AO — Active/TimeEvent base "classes" on top of uC/OS-II
│   ├── uCOS2/                         # uC/OS-II headers only (os.h, ucos_ii.h, os_cpu.h, os_trace.h)
│   ├── Vendor/                        # qassert.h — not written by hand
│   └── CMSIS/                         # ARM/ST headers — not written by hand
├── Src/
│   ├── bsp.c                          # GPIO, uC/OS-II app hooks (button debounce + TimeEvent_tick), SysTick config
│   ├── main.c                         # BlinkyButton active object + dispatch, starts uC/OS-II
│   ├── stm32f4xx_it.c                 # fault + unused-IRQ handlers (controlled reset)
│   ├── uc_ao.c                        # uC/AO implementation (Active, TimeEvent)
│   └── Vendor/                        # newlib stubs + CMSIS system source — not written by hand
├── uCOS2/                             # uC/OS-II kernel + ARMv7-M/GNU port, one .c per translation
│                                       # unit (os_core.c, os_task.c, ... + os_cpu_c.c/os_cpu_a.s/os_dbg.c),
│                                       # matching the vendor's own Keil reference project layout
├── Startup/
│   └── startup_stm32f446retx.s        # reset handler, vector table
├── STM32F446RETX_FLASH.ld / _RAM.ld   # linker scripts
└── .project / .cproject / .settings   # STM32CubeIDE project files
```

## Build & run

1. Open in STM32CubeIDE (`File > Open Projects from File System...`, point at
   this folder)
2. Build (`Ctrl+B`)
3. Wire the external LED per the Hardware table above if you want both
   LEDs visible (the onboard LED and B1 button work with no extra wiring)
4. Connect the Nucleo board via USB, run **Debug** (`led-blink Debug` launch
   configuration is committed in this folder) or **Run**

## Reference Documents

- [STM32F446xx Reference Manual (RM0390)](https://www.st.com/resource/en/reference_manual/rm0390-stm32f446xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [STM32 Nucleo-64 Boards (MB1136) User Manual (UM1724)](https://www.st.com/resource/en/user_manual/um1724-stm32-nucleo64-boards-mb1136-stmicroelectronics.pdf)
