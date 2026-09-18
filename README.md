# LED Blink

Bare-metal on the STM32 Nucleo-F446RE. Started as a simple LED blink and
went through several increasingly "real" concurrency models (hand-rolled
kernel → QP/C's QXK → uC/OS-II + a hand-rolled uC/AO Active-Object layer).
It now runs on the real **Quantum Leaps QP/C** library (QEP state machine
engine + QF + the preemptive **QXK** kernel) — no more hand-rolled AO/FSM
plumbing. No HAL, no CubeMX drivers; registers are hand-rolled. Progression
is tracked in `git log`.

## What it does

Two flat `QActive` objects demonstrate **Run-to-Completion (RTC) execution**
together with **Rate-Monotonic Scheduling (RMS)**:

- `Blinky1` (priority **5**, highest) — a periodic `QTimeEvt` fires every
  2 ticks; its handler busy-loops the onboard green LED 1500 times to stand
  in for a short, fixed processing time (its WCET).
- `Blinky2` (priority **2**) — woken by a `BUTTON_PRESS_SIG` posted from the
  B1 EXTI ISR; its handler busy-loops the external blue LED 3× as long
  (4500 times) to stand in for a longer, less frequent job.

Each handler always runs to completion once started — no reentrancy, no
locks needed for an AO's own state. But RTC only protects an AO from *its
own* future events; a lower-priority AO's handler can still be **preempted
mid-execution** by a higher-priority one, because QXK is a real preemptive
kernel. Classic RMS says: shorter period → higher priority, which is exactly
how `Blinky1`/`Blinky2` are assigned here — the pair is a minimal, physical
demonstration of priority-based preemptive scheduling among active objects.

Both busy-loops run in a few dozen microseconds — far faster than the eye's
flicker-fusion threshold, so neither LED visibly "blinks" during a handler;
this is meant for a logic analyzer/scope, not naked-eye observation (same
convention as earlier busy-loop lessons in this project).

B1 (PC13, on-board, active-low) is interrupt-driven again (`EXTI15_10_IRQHandler`,
falling edge — pressing pulls the pin LOW), posting `BUTTON_PRESS_SIG`
straight to `AO_Blinky2` via `QACTIVE_POST()`. `SysTick_Handler` only drives
`QF_TICK_X()` now (no polling/debounce — that was specific to the earlier
`TimeBomb`/uC-AO lessons and doesn't apply here).

CPU faults and every unused IRQ resolve to a controlled `NVIC_SystemReset()`
(`stm32f4xx_it.c`), also wired up as QP/C's own `Q_onError()` handler — so
an internal QP/C safety check failure resets the same way a HardFault would.

> **Gotcha we hit:** static `QEvt` instances must be built with
> `QEVT_INITIALIZER(SIG)`, not a bare `{SIG}` aggregate — real QP/C tags
> every event with a safety-marker byte and resets the MCU on the first
> `QACTIVE_POST()` of an untagged one.

## Hardware & Tools

| | |
|---|---|
| Board / MCU | Nucleo-64 F446RE, STM32F446RE (Cortex-M4F) |
| Onboard LED | LD2 — PA5 |
| External LED | PA6 → resistor (~220–330 Ω) → LED anode; cathode → GND |
| User button | B1 (blue, on-board) — PC13, active-low, interrupt-driven (EXTI15_10) |
| Tick source | SysTick, 1 kHz (`BSP_TICKS_PER_SEC`; `HCLK` = 16 MHz HSI) |
| Kernel | QP/C QXK (preemptive) |
| IDE / Debugger | STM32CubeIDE (GCC ARM) / ST-LINK/V2-1 (on-board) |

## Project structure

```
led-blink/
├── Inc/
│   ├── bsp.h        # BSP interface + AO/signal declarations
│   ├── QPC/         # real Quantum Leaps QP/C headers — not written by hand
│   └── CMSIS/       # ARM/ST headers — not written by hand
├── Src/
│   ├── bsp.c        # GPIO, B1 EXTI ISR, SysTick_Handler (QF_TICK_X), QF_on*/QXK_onIdle/Q_onError callbacks
│   ├── main.c       # Blinky1/Blinky2 AOs (RTC + RMS demo), QF_init/QACTIVE_START/QF_run
│   ├── stm32f4xx_it.c
│   └── Vendor/      # newlib stubs + CMSIS system source — not written by hand
├── QPC/             # real Quantum Leaps QP/C sources (QEP/QF/QXK) — not written by hand
├── Startup/         # reset handler, vector table
└── STM32F446RETX_FLASH.ld / _RAM.ld
```

## Build & run

1. Open in STM32CubeIDE and build (`Ctrl+B`)
2. Wire the external blue LED per the table above (onboard LED/B1 need no wiring)
3. Connect via USB, then **Run** or **Debug**

## Reference Documents

- [STM32F446xx Reference Manual (RM0390)](https://www.st.com/resource/en/reference_manual/rm0390-stm32f446xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [STM32 Nucleo-64 Boards (MB1136) User Manual (UM1724)](https://www.st.com/resource/en/user_manual/um1724-stm32-nucleo64-boards-mb1136-stmicroelectronics.pdf)
