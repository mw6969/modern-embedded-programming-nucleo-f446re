# LED Blink

Bare-metal on the STM32 Nucleo-F446RE. Started as a simple LED blink and
went through several increasingly "real" concurrency models (hand-rolled
kernel → QP/C's QXK → uC/OS-II + a hand-rolled uC/AO Active-Object layer).
It now runs on the real **Quantum Leaps QP/C** library (QEP hierarchical
state machine engine + QF + the cooperative **QV** kernel) — no more
hand-rolled AO/FSM plumbing. No HAL, no CubeMX drivers; registers are
hand-rolled. Progression is tracked in `git log`.

## What it does

One active object, `TimeBomb` (a real `QActive`/`QHsm` subclass), implements
the "arm, blink, explode" state machine, used to teach **hierarchical
states**, **guard conditions**, and **entry/exit actions**:

```
QHsm_top
├── armed                    (exit: both LEDs off; BUTTON2_PRESSED_SIG → defused)
│   ├── wait4button          green LED on, waiting for B1
│   ├── blink                blue LED on, 500 ms QTimeEvt
│   ├── pause                blue LED off, 500 ms QTimeEvt
│   └── boom                 both LEDs on (terminal within `armed`)
└── defused                  blue LED on; BUTTON2_PRESSED_SIG → armed
```

`BUTTON_PRESSED_SIG` arms the bomb (`blink_ctr = 5` → `blink`). Each
`TIMEOUT_SIG` alternates `blink` ↔ `pause`; `pause`'s handler is the
lesson's actual topic — a **guard condition**, `[blink_ctr > 0]`, choosing
another cycle vs. `boom` for the *same* event. `BUTTON2_PRESSED_SIG` is
handled once on the shared `armed` superstate rather than duplicated per
substate — the textbook reason to introduce a superstate in the first
place.

Buttons are polled (not interrupt-driven), debounced together bit-parallel
in `SysTick_Handler` (Ganssle/Barr algorithm), which also drives
`QF_TICK_X()` and posts events via `QACTIVE_POST()`. B1 (PC13, on-board) is
active-low; B2 (PC0, external) is active-high (internal pull-down).

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
| User button | B1 (blue, on-board) — PC13, active-low |
| Defuse button | B2 (external) — PC0 / Arduino header A5, active-high (internal pull-down) |
| Tick source | SysTick, 100 Hz (`BSP_TICKS_PER_SEC`; `HCLK` = 16 MHz HSI) |
| Kernel | QP/C QV (cooperative) |
| IDE / Debugger | STM32CubeIDE (GCC ARM) / ST-LINK/V2-1 (on-board) |

## Project structure

```
led-blink/
├── Inc/
│   ├── bsp.h        # BSP interface + AO/signal declarations
│   ├── QPC/         # real Quantum Leaps QP/C headers — not written by hand
│   └── CMSIS/       # ARM/ST headers — not written by hand
├── Src/
│   ├── bsp.c        # GPIO, SysTick_Handler (debounce + QF_TICK_X), QF_on*/Q_onError callbacks
│   ├── main.c       # TimeBomb AO (QHsm state handlers), QF_init/QACTIVE_START/QF_run
│   ├── stm32f4xx_it.c
│   └── Vendor/      # newlib stubs + CMSIS system source — not written by hand
├── QPC/             # real Quantum Leaps QP/C sources (QEP/QF/QV) — not written by hand
├── Startup/         # reset handler, vector table
└── STM32F446RETX_FLASH.ld / _RAM.ld
```

## Build & run

1. Open in STM32CubeIDE and build (`Ctrl+B`)
2. Wire the external LED + B2 per the table above (onboard LED/B1 need no wiring)
3. Connect via USB, then **Run** or **Debug**

## Reference Documents

- [STM32F446xx Reference Manual (RM0390)](https://www.st.com/resource/en/reference_manual/rm0390-stm32f446xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [STM32 Nucleo-64 Boards (MB1136) User Manual (UM1724)](https://www.st.com/resource/en/user_manual/um1724-stm32-nucleo64-boards-mb1136-stmicroelectronics.pdf)
