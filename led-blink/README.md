# LED Blink → MyROS (mini preemptive kernel)

Bare-metal on the STM32 Nucleo-F446RE. Started as a simple LED blink, grew
into a hand-written minimal preemptive round-robin kernel running two
independent threads. No HAL, no CubeMX-generated drivers, no RTOS library —
registers and context switches are hand-rolled.

Progression is tracked in the commit history — see `git log` for this folder.

## What it does

Two LEDs blink independently, each on its own thread, scheduled preemptively
by a from-scratch mini-kernel (`MyROS`):

| Thread | LED | Rate |
|---|---|---|
| `blinkyGreen` | onboard LD2 (PA5) | 250 ms on / 750 ms off |
| `blinkyBlue`  | external LED (PA6 / D12, breadboard + resistor) | 125 ms on / 125 ms off |

Also includes startup-code hardening: CPU fault handlers (HardFault,
MemManage, BusFault, UsageFault, NMI) and every unused peripheral IRQ are
routed to a controlled `NVIC_SystemReset()` instead of silently hanging.

## MyROS — the mini-kernel (`MyROS/`)

A minimal preemptive round-robin scheduler for Cortex-M, built from scratch:

- `OSThread_start()` hand-builds each thread's initial stack — the exact
  16-word layout the CPU pushes automatically on a real exception — so the
  first "return" into a thread looks identical to a normal exception return.
- `OS_run()` bootstraps the very first thread with that same trick, then
  hands control to the scheduler.
- `SysTick` (100 Hz) calls `OS_tick()`, which picks the next thread in
  round-robin order and, if it differs from the running one, pends `PendSV`.
- `PendSV_Handler` performs the actual context switch: manually swaps R4-R11
  between the two threads' stacks; the hardware restores the rest
  (R0-R3/R12/LR/PC/xPSR) automatically on exception return.

## Hardware

| | |
|---|---|
| Board | STM32 Nucleo-64 F446RE |
| MCU | STM32F446RE, ARM Cortex-M4F |
| Onboard LED | LD2 — GPIOA, pin 5 (PA5) |
| External LED | PA6 / Arduino header D12 → resistor (~220-330 Ω) → LED anode; cathode → GND |
| Tick source | SysTick, 100 Hz (`HCLK` = 16 MHz HSI, no PLL configured) |

## Project structure

```
led-blink/
├── Inc/
│   ├── bsp.h                          # board support package interface
│   └── CMSIS/                         # ARM/ST headers — not written by hand
├── Src/
│   ├── bsp.c                          # GPIO, SysTick, LED control
│   ├── main.c                         # creates both threads, starts the kernel
│   ├── stm32f4xx_it.c                 # fault + unused-IRQ handlers (controlled reset)
│   └── Vendor/                        # newlib stubs + CMSIS system source — not written by hand
├── MyROS/
│   ├── myros.h
│   └── myros.c                        # the mini-kernel described above
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
   threads visible (the onboard LED works with no extra wiring)
4. Connect the Nucleo board via USB, run **Debug** (`led-blink Debug` launch
   configuration is committed in this folder) or **Run**
