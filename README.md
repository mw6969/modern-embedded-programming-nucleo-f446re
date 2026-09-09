# LED Blink → uC/OS-II RTOS

Bare-metal on the STM32 Nucleo-F446RE. Started as a simple LED blink, grew
into a hand-written minimal preemptive kernel (`MyROS`), then QP/C's **QXK**
dual-mode kernel, and now runs Silicon Labs' uC/OS-II — a real,
widely-used RTOS, brought in as a deliberate step back to "superloop +
classic RTOS" before revisiting the same problem with an event-driven
Active Object design. No HAL, no CubeMX-generated drivers — registers are
hand-rolled.

Progression is tracked in the commit history — see `git log` for this folder.

## What it does

Two uC/OS-II tasks:

| Task | Priority | LED | Behavior |
|---|---|---|---|
| `main_blinky` | `OS_LOWEST_PRIO - 4` | onboard LD2 (PA5) | blinks continuously; on-time is a shared `shared_blink_time` variable (25% duty cycle) |
| `main_button` | `OS_LOWEST_PRIO - 3` | external LED (PA6 / D12, breadboard + resistor) | lights up while the user button (B1) is held, halves `shared_blink_time` on each press |

Unlike the earlier QXK version, the button (B1 / PC13) is **not** wired to an
EXTI interrupt — it's debounced by polling inside `App_TimeTickHook()`
(called every SysTick tick), using the classic Ganssle/Barr debounce
algorithm. Debounced press/release edges post two binary semaphores
(`BSP_semaPress` / `BSP_semaRelease`) that `main_button` blocks on.
`shared_blink_time` is protected by a uC/OS-II mutex
(`shared_blink_time_mutex`) since both tasks touch it.

Also includes startup-code hardening: CPU fault handlers (NMI, HardFault,
MemManage, BusFault, UsageFault) and every unused peripheral IRQ are routed
to a controlled `NVIC_SystemReset()` instead of silently hanging.
`SysTick_Handler` and `PendSV_Handler` are owned by uC/OS-II's ARMv7-M/GNU
port, not by application code.

`uc_ao.c`/`uc_ao.h` (Quantum Leaps' minimal **uC/AO** — Active Object pattern
on top of uC/OS-II: `Active`/`TimeEvent` base "classes", an event queue per
task instead of raw semaphores/mutexes) are in the tree and compile cleanly,
but nothing calls into them yet — staged for the next step of this lesson,
where the blinky/button logic gets refactored onto Active Objects.

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
│   ├── uc_ao.h                        # uC/AO (Active Object pattern on uC/OS-II) — not wired in yet
│   ├── uCOS2/                         # uC/OS-II headers only (os.h, ucos_ii.h, os_cpu.h, os_trace.h)
│   ├── Vendor/                        # qassert.h — not written by hand
│   └── CMSIS/                         # ARM/ST headers — not written by hand
├── Src/
│   ├── bsp.c                          # GPIO, uC/OS-II app hooks (incl. button debounce), SysTick config
│   ├── main.c                         # creates both tasks + semaphores + mutex, starts uC/OS-II
│   ├── stm32f4xx_it.c                 # fault + unused-IRQ handlers (controlled reset)
│   ├── uc_ao.c                        # uC/AO implementation — not wired in yet
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
