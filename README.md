# LED Blink

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

One active object, `TimeBomb`, implementing the classic "arm, blink,
explode" state machine used to teach **guard conditions**, **entry/exit
actions**, and the **state handler** implementation strategy: each state
is an ordinary C function switching on `e->sig` (including the synthetic
`ENTRY_SIG`/`EXIT_SIG`), returning `TRAN_STATUS`/`HANDLED_STATUS`/
`IGNORED_STATUS`, with a `TRAN(target_)` macro that sets `me->state` to
the target function. This Fsm plumbing (`Fsm_ctor()`/`Fsm_init()`/
`Fsm_dispatch()`) lives in `uc_ao.c`/`.h` as a reusable base "class" that
`Active` inherits, rather than being hand-rolled per AO.

| State | Meaning |
|---|---|
| `TimeBomb_wait4button` | idle, green LED on (entry), waiting for B1 |
| `TimeBomb_blink` | blue LED on (entry) for 500 ms |
| `TimeBomb_pause` | blue LED off (`blink`'s exit) for 500 ms |
| `TimeBomb_boom` | terminal — both LEDs solid on (entry) |
| `TimeBomb_defused` | terminal — blue LED solid on (entry) |

`BUTTON_PRESSED_SIG` arms the bomb (`blink_ctr = 5`, → `blink`).
`TIMEOUT_SIG` from a self-armed `TimeEvent` then alternates
`blink` ↔ `pause` every 500 ms. `pause`'s `TIMEOUT_SIG` handler is the
lesson's actual topic: a **guard condition**, `[blink_ctr > 0]`, picking
between `TRAN(TimeBomb_blink)` (another cycle) or `TRAN(TimeBomb_boom)`
(counter hit zero) for the *same* event — textbook UML
`event [guard] / action`. `boom` ignores everything except its own
`ENTRY_SIG`.

`BUTTON2_PRESSED_SIG` ("defuse the bomb") transitions to `TimeBomb_defused`
from every other state — currently a `case` duplicated identically in
`wait4button`/`blink`/`pause`/`boom`, the motivating example for
**hierarchical state machines**: the next step is collapsing this
repeated transition into one handler on a shared superstate instead of
copy-pasting it per substate.

Both buttons are still **not** interrupt-driven — debounced together by
polling inside `App_TimeTickHook()` (Ganssle/Barr algorithm, bit-parallel
over both buttons' pins), which also drives `TimeEvent_tick()` and posts
events into the AO's queue via `Active_post()`. B1 (PC13, on-board) is
active-low (on-board pull-up), so its bit is inverted before the shared
debounce logic runs; B2 (PC0, external) is active-high by construction
(internal pull-down, button wired to 3V3), so it needs no inversion.

Also includes startup-code hardening: CPU fault handlers and every unused
peripheral IRQ are routed to a controlled `NVIC_SystemReset()` instead of
silently hanging. `SysTick_Handler`/`PendSV_Handler` are owned by
uC/OS-II's ARMv7-M/GNU port, not application code.

## Hardware & Tools

| | |
|---|---|
| Board | STM32 Nucleo-64 F446RE |
| MCU | STM32F446RE, ARM Cortex-M4F |
| Onboard LED | LD2 — GPIOA, pin 5 (PA5) |
| External LED | PA6 / Arduino header D12 → resistor (~220-330 Ω) → LED anode; cathode → GND |
| User button | B1 (blue) — PC13, polled (not interrupt-driven), active-low (on-board pull-up) |
| Defuse button | B2 (external) — PC0 / Arduino header A5, polled, active-high (internal pull-down, button → 3V3) |
| Tick source | SysTick, 100 Hz (`OS_TICKS_PER_SEC`; `HCLK` = 16 MHz HSI, no PLL configured) |
| IDE / Toolchain | STM32CubeIDE (GCC ARM) |
| Debugger | ST-LINK/V2-1 (on-board) |

## Project structure

```
led-blink/
├── Inc/
│   ├── bsp.h                          # board support package interface
│   ├── app_cfg.h / os_cfg.h           # this project's uC/OS-II configuration
│   ├── uc_ao.h                        # uC/AO — Fsm/Active/TimeEvent base "classes" on top of uC/OS-II
│   ├── uCOS2/                         # uC/OS-II headers only (os.h, ucos_ii.h, os_cpu.h, os_trace.h)
│   ├── Vendor/                        # qassert.h — not written by hand
│   └── CMSIS/                         # ARM/ST headers — not written by hand
├── Src/
│   ├── bsp.c                          # GPIO, uC/OS-II app hooks (button debounce + TimeEvent_tick), SysTick config
│   ├── main.c                         # TimeBomb active object (state handlers), starts uC/OS-II
│   ├── stm32f4xx_it.c                 # fault + unused-IRQ handlers (controlled reset)
│   ├── uc_ao.c                        # uC/AO implementation (Fsm, Active, TimeEvent)
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
3. Wire the external LED and B2 button per the Hardware table above if you
   want to see both LEDs and both buttons (the onboard LED and B1 button
   work with no extra wiring)
4. Connect the Nucleo board via USB, run **Debug** (`led-blink Debug` launch
   configuration is committed in this folder) or **Run**

## Reference Documents

- [STM32F446xx Reference Manual (RM0390)](https://www.st.com/resource/en/reference_manual/rm0390-stm32f446xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [STM32 Nucleo-64 Boards (MB1136) User Manual (UM1724)](https://www.st.com/resource/en/user_manual/um1724-stm32-nucleo64-boards-mb1136-stmicroelectronics.pdf)
