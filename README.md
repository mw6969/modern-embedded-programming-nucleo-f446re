# Modern Embedded Systems Programming — STM32 Nucleo-F446RE Port

Practical exercises from Miro Samek's [Modern Embedded Systems Programming Course](https://www.youtube.com/playlist?list=PLPW8O6W-1chwyTzI3BHwBLbGQoPFxPAPM), ported and adapted for the **STM32 Nucleo-64 F446RE** board (ARM Cortex-M4F), since the original course targets the TI EK-TM4C123GXL (TivaC LaunchPad) / STM32 NUCLEO-C031C6.

Each project may cover several video lessons at once — a topic is committed as a project once it reaches a meaningful, working state.

## Hardware & Tools

| | |
|---|---|
| Board | STM32 Nucleo-64 NUCLEO-F446RE |
| MCU | STM32F446RE, ARM Cortex-M4F |
| IDE / Toolchain | STM32CubeIDE (GCC ARM) |
| Debugger | ST-LINK/V2-1 (on-board) |

## Projects

| Folder | Course lessons | Topic |
|--------|-----------------|-------|
| [`led-blink`](./led-blink) | [#1–#6](https://www.youtube.com/playlist?list=PLPW8O6W-1chwyTzI3BHwBLbGQoPFxPAPM)+ | Blinking the on-board LED |

*(new rows are added when a new topic starts — e.g. LED blinking, RTOS etc. Within one topic, progress is tracked via commits and the project's own README)*

## Repository Structure

Each project is a self-contained STM32CubeIDE project (no CubeMX/HAL — register-level):

```
topic-slug/
├── Inc/                          # header files
├── Src/
│   ├── main.c
│   ├── syscalls.c                # newlib syscall stubs (STM32CubeIDE default)
│   └── sysmem.c                  # newlib heap stub
├── Startup/
│   └── startup_stm32f446retx.s   # reset handler, vector table
├── STM32F446RETX_FLASH.ld        # linker script
├── STM32F446RETX_RAM.ld          # linker script
├── .project / .cproject          # STM32CubeIDE project files, committed so the
│                                  # project can be re-imported as-is
└── README.md                     # what this project does, which lessons it covers,
                                   # board notes
```

## Reference Documents

- [STM32F446xx Reference Manual (RM0390)](https://www.st.com/resource/en/reference_manual/rm0390-stm32f446xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf) — register-level details for the MCU
- [STM32 Nucleo-64 Boards (MB1136) User Manual (UM1724)](https://www.st.com/resource/en/user_manual/um1724-stm32-nucleo64-boards-mb1136-stmicroelectronics.pdf) — board schematics, pin mapping, ST-LINK details
