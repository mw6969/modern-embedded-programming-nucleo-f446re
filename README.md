# LED Blink

Bare-metal on the STM32 Nucleo-F446RE, running on the real **Quantum Leaps
QP/C** library (QEP + QF + the preemptive **QXK** kernel). No HAL, no CubeMX
drivers; registers are hand-rolled. Project history is in `git log`.

## What it does

Two `QActive` objects demonstrate **Run-to-Completion (RTC)** together with
**Rate-Monotonic Scheduling (RMS)**:

| AO | Prio | Trigger | Handler (busy-loop = stand-in for WCET) |
|---|---|---|---|
| `Blinky1` | 5 (highest) | periodic `QTimeEvt`, 2 ticks initially | green LED, 1500 iterations |
| `Blinky2` | 2 | `BUTTON_PRESS_SIG` from the B1 EXTI ISR | blue LED, 4500 iterations, then posts `BlinkPatternEvt` to `Blinky1` |

RTC protects an AO only from its *own* future events; a lower-priority handler
can still be **preempted mid-execution** by a higher-priority AO. RMS (shorter
period → higher priority) is how the two priorities are assigned.

Both loops last a few dozen µs, so the LEDs don't visibly blink — observe with
a logic analyzer/scope.

### Dynamic events

On each B1 press `Blinky2` allocates a `BlinkPatternEvt` (a `QEvt` subclass
with `ticks` and `iter`) via `Q_NEW()` from a QF pool (`QF_poolInit()`, 10
blocks) and posts it to `AO_Blinky1` with `QACTIVE_POST()`. `Blinky1` re-arms
its `QTimeEvt` with `ticks` and stores `iter`. `Blinky2` alternates between
`{2 ticks, 1500}` and `{1 tick, 750}`. QF recycles the event after the
receiver's handler returns.

### Interrupts & faults

- B1 (PC13, active-low) uses `EXTI15_10_IRQHandler` on the falling edge and
  posts `BUTTON_PRESS_SIG` to `AO_Blinky2`.
- `SysTick_Handler` only drives `QF_TICK_X()` — no polling/debounce.
- CPU faults, unused IRQs and QP/C's `Q_onError()` all end in
  `NVIC_SystemReset()` (`stm32f4xx_it.c`).

> **Gotcha:** static `QEvt`s must use `QEVT_INITIALIZER(SIG)`, not a bare
> `{SIG}` — QP/C tags events with a safety marker and resets the MCU on the
> first `QACTIVE_POST()` of an untagged one.

## Hardware & Tools

| | |
|---|---|
| Board / MCU | Nucleo-64 F446RE, STM32F446RE (Cortex-M4F) |
| Onboard LED | LD2 — PA5 |
| External LED | PA6 → ~220–330 Ω → LED anode; cathode → GND |
| User button | B1 — PC13, active-low, EXTI15_10 |
| Tick source | SysTick, 1 kHz (`BSP_TICKS_PER_SEC`); HCLK = 16 MHz HSI |
| Kernel | QP/C QXK (preemptive) |
| IDE / Debugger | STM32CubeIDE (GCC ARM) / on-board ST-LINK/V2-1 |

## Project structure

```
led-blink/
├── Inc/
│   ├── bsp.h        # BSP interface + AO/signal declarations
│   ├── QPC/, CMSIS/ # vendor headers
├── Src/
│   ├── bsp.c        # GPIO, B1 ISR, SysTick, QF_on*/QXK_onIdle/Q_onError
│   ├── main.c       # Blinky1/Blinky2 AOs, QF_init/QACTIVE_START/QF_run
│   ├── stm32f4xx_it.c
│   └── Vendor/      # newlib stubs + CMSIS system source
├── QPC/             # QP/C sources (QEP/QF/QXK)
├── Startup/         # reset handler, vector table
└── STM32F446RETX_FLASH.ld / _RAM.ld
```

`QPC/`, `CMSIS/` and `Vendor/` are third-party code, not hand-written.

## Build & run

1. Open in STM32CubeIDE and build (`Ctrl+B`)
2. Wire the external LED per the table above (LD2 and B1 need no wiring)
3. Connect via USB, then **Run** or **Debug**

## References

- [STM32F446xx Reference Manual (RM0390)](https://www.st.com/resource/en/reference_manual/rm0390-stm32f446xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [Nucleo-64 User Manual (UM1724)](https://www.st.com/resource/en/user_manual/um1724-stm32-nucleo64-boards-mb1136-stmicroelectronics.pdf)
