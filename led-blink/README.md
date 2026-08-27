# LED Blink → QP/C QXK (dual-mode RTOS)

Bare-metal on the STM32 Nucleo-F446RE. Started as a simple LED blink, grew
into a hand-written minimal preemptive kernel (`MyROS`), then migrated onto
[QP/C](https://www.state-machine.com/qpc/) running the **QXK** dual-mode
kernel (preemptive + blocking extended threads). No HAL, no
CubeMX-generated drivers — registers are hand-rolled.

Progression is tracked in the commit history — see `git log` for this folder.

## What it does

Two LEDs, each on its own QXK extended thread:

| Thread | Priority | LED | Behavior |
|---|---|---|---|
| `blinkyGreen` | 5 | onboard LD2 (PA5) | blinks once each time the user button (B1) is pressed |
| `blinkyBlue`  | 2 | external LED (PA6 / D12, breadboard + resistor) | blinks continuously, blocking via `QXThread_delay()` |

The button press is detected in `EXTI15_10_IRQHandler` (PC13), which signals
a binary semaphore (`B1_sema`) that `blinkyGreen` blocks on — a minimal
example of ISR-to-thread synchronization.

Also includes startup-code hardening: CPU fault handlers (HardFault,
MemManage, BusFault, UsageFault) and every unused peripheral IRQ are routed
to a controlled `NVIC_SystemReset()` instead of silently hanging.

## Hardware

| | |
|---|---|
| Board | STM32 Nucleo-64 F446RE |
| MCU | STM32F446RE, ARM Cortex-M4F |
| Onboard LED | LD2 — GPIOA, pin 5 (PA5) |
| External LED | PA6 / Arduino header D12 → resistor (~220-330 Ω) → LED anode; cathode → GND |
| User button | B1 (blue) — PC13, EXTI15_10, rising edge |
| Tick source | SysTick, 1 kHz (`HCLK` = 16 MHz HSI, no PLL configured) |

## Project structure

```
led-blink/
├── Inc/
│   ├── bsp.h                          # board support package interface
│   ├── QPC/                           # QP/C headers + QXK/GNU port (qp_port.h, qs_port.h)
│   └── CMSIS/                         # ARM/ST headers — not written by hand
├── Src/
│   ├── bsp.c                          # GPIO, button/EXTI, SysTick, QXK callbacks, IRQ handlers
│   ├── main.c                         # creates both threads + semaphore, starts QF
│   ├── stm32f4xx_it.c                 # fault + unused-IRQ handlers (controlled reset)
│   └── Vendor/                        # newlib stubs + CMSIS system source — not written by hand
├── QPC/                                # QP/C framework sources + qxk_port.c (GNU/ARM-CM port)
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
