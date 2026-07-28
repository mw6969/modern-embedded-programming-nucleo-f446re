/*
 * stm32f4xx_it.c -- Fault and interrupt handling.
 */

#include "bsp.h"
#include "stm32f446xx.h"

/*******************************************************************************
 * CPU fault handlers
 * These override the weak defaults in startup_stm32f446retx.s.
 ******************************************************************************/
void NMI_Handler(void)        { assert_failed("NMI_Handler",        __LINE__); }
void HardFault_Handler(void)  { assert_failed("HardFault_Handler",  __LINE__); }
void MemManage_Handler(void)  { assert_failed("MemManage_Handler",  __LINE__); }
void BusFault_Handler(void)   { assert_failed("BusFault_Handler",   __LINE__); }
void UsageFault_Handler(void) { assert_failed("UsageFault_Handler", __LINE__); }

/*******************************************************************************
 * Unused_Handler() -- common handler for every interrupt this project
 * does not yet implement.
 ******************************************************************************/
void Unused_Handler(void) {
    assert_failed("Unused_Handler", __LINE__);
}

#define UNUSED_IRQ(name) void name(void) { Unused_Handler(); }

/* Cortex-M system handlers not yet used by this project */
UNUSED_IRQ(SVC_Handler)
UNUSED_IRQ(DebugMon_Handler)
UNUSED_IRQ(PendSV_Handler)

/* STM32F446RE peripheral interrupts not yet used by this project */
UNUSED_IRQ(WWDG_IRQHandler)                /* Window Watchdog interrupt */
UNUSED_IRQ(PVD_IRQHandler)                 /* PVD through EXTI line detection interrupt */
UNUSED_IRQ(TAMP_STAMP_IRQHandler)          /* Tamper and TimeStamp interrupts through the EXTI line */
UNUSED_IRQ(RTC_WKUP_IRQHandler)            /* RTC Wakeup interrupt through the EXTI line */
UNUSED_IRQ(FLASH_IRQHandler)               /* Flash global interrupt */
UNUSED_IRQ(RCC_IRQHandler)                 /* RCC global interrupt */
UNUSED_IRQ(EXTI0_IRQHandler)               /* EXTI Line0 interrupt */
UNUSED_IRQ(EXTI1_IRQHandler)               /* EXTI Line1 interrupt */
UNUSED_IRQ(EXTI2_IRQHandler)               /* EXTI Line2 interrupt */
UNUSED_IRQ(EXTI3_IRQHandler)               /* EXTI Line3 interrupt */
UNUSED_IRQ(EXTI4_IRQHandler)               /* EXTI Line4 interrupt */
UNUSED_IRQ(DMA1_Stream0_IRQHandler)        /* DMA1 Stream0 global interrupt */
UNUSED_IRQ(DMA1_Stream1_IRQHandler)        /* DMA1 Stream1 global interrupt */
UNUSED_IRQ(DMA1_Stream2_IRQHandler)        /* DMA1 Stream2 global interrupt */
UNUSED_IRQ(DMA1_Stream3_IRQHandler)        /* DMA1 Stream3 global interrupt */
UNUSED_IRQ(DMA1_Stream4_IRQHandler)        /* DMA1 Stream4 global interrupt */
UNUSED_IRQ(DMA1_Stream5_IRQHandler)        /* DMA1 Stream5 global interrupt */
UNUSED_IRQ(DMA1_Stream6_IRQHandler)        /* DMA1 Stream6 global interrupt */
UNUSED_IRQ(ADC_IRQHandler)                 /* ADC1 global interrupt */
UNUSED_IRQ(CAN1_TX_IRQHandler)             /* CAN1 TX interrupts */
UNUSED_IRQ(CAN1_RX0_IRQHandler)            /* CAN1 RX0 interrupts */
UNUSED_IRQ(CAN1_RX1_IRQHandler)            /* CAN1 RX1 interrupts */
UNUSED_IRQ(CAN1_SCE_IRQHandler)            /* CAN1 SCE interrupt */
UNUSED_IRQ(EXTI9_5_IRQHandler)             /* EXTI Line[9:5] interrupts */
UNUSED_IRQ(TIM1_BRK_TIM9_IRQHandler)       /* TIM1 Break interrupt and TIM9 global interrupt */
UNUSED_IRQ(TIM1_UP_TIM10_IRQHandler)       /* TIM1 Update interrupt and TIM10 global interrupt */
UNUSED_IRQ(TIM1_TRG_COM_TIM11_IRQHandler)  /* TIM1 Trigger and Commutation interrupts and TIM11 global interrupt */
UNUSED_IRQ(TIM1_CC_IRQHandler)             /* TIM1 Capture Compare interrupt */
UNUSED_IRQ(TIM2_IRQHandler)                /* TIM2 global interrupt */
UNUSED_IRQ(TIM3_IRQHandler)                /* TIM3 global interrupt */
UNUSED_IRQ(TIM4_IRQHandler)                /* TIM4 global interrupt */
UNUSED_IRQ(I2C1_EV_IRQHandler)             /* I2C1 event interrupt */
UNUSED_IRQ(I2C1_ER_IRQHandler)             /* I2C1 error interrupt */
UNUSED_IRQ(I2C2_EV_IRQHandler)             /* I2C2 event interrupt */
UNUSED_IRQ(I2C2_ER_IRQHandler)             /* I2C2 error interrupt */
UNUSED_IRQ(SPI1_IRQHandler)                /* SPI1 global interrupt */
UNUSED_IRQ(SPI2_IRQHandler)                /* SPI2 global interrupt */
UNUSED_IRQ(USART1_IRQHandler)              /* USART1 global interrupt */
UNUSED_IRQ(USART2_IRQHandler)              /* USART2 global interrupt */
UNUSED_IRQ(USART3_IRQHandler)              /* USART3 global interrupt */
UNUSED_IRQ(EXTI15_10_IRQHandler)           /* EXTI Line[15:10] interrupts */
UNUSED_IRQ(RTC_Alarm_IRQHandler)           /* RTC Alarms (A and B) through EXTI line interrupt */
UNUSED_IRQ(OTG_FS_WKUP_IRQHandler)         /* USB On-The-Go FS Wakeup through EXTI line interrupt */
UNUSED_IRQ(TIM8_BRK_TIM12_IRQHandler)      /* TIM8 Break interrupt and TIM12 global interrupt */
UNUSED_IRQ(TIM8_UP_TIM13_IRQHandler)       /* TIM8 Update interrupt and TIM13 global interrupt */
UNUSED_IRQ(TIM8_TRG_COM_TIM14_IRQHandler)  /* TIM8 Trigger and Commutation interrupts and TIM14 global interrupt */
UNUSED_IRQ(TIM8_CC_IRQHandler)             /* TIM8 Capture Compare interrupt */
UNUSED_IRQ(DMA1_Stream7_IRQHandler)        /* DMA1 Stream7 global interrupt */
UNUSED_IRQ(FMC_IRQHandler)                 /* FMC global interrupt */
UNUSED_IRQ(SDIO_IRQHandler)                /* SDIO global interrupt */
UNUSED_IRQ(TIM5_IRQHandler)                /* TIM5 global interrupt */
UNUSED_IRQ(SPI3_IRQHandler)                /* SPI3 global interrupt */
UNUSED_IRQ(UART4_IRQHandler)               /* UART4 global interrupt */
UNUSED_IRQ(UART5_IRQHandler)               /* UART5 global interrupt */
UNUSED_IRQ(TIM6_DAC_IRQHandler)            /* TIM6 global interrupt, DAC1 and DAC2 underrun error interrupt */
UNUSED_IRQ(TIM7_IRQHandler)                /* TIM7 global interrupt */
UNUSED_IRQ(DMA2_Stream0_IRQHandler)        /* DMA2 Stream0 global interrupt */
UNUSED_IRQ(DMA2_Stream1_IRQHandler)        /* DMA2 Stream1 global interrupt */
UNUSED_IRQ(DMA2_Stream2_IRQHandler)        /* DMA2 Stream2 global interrupt */
UNUSED_IRQ(DMA2_Stream3_IRQHandler)        /* DMA2 Stream3 global interrupt */
UNUSED_IRQ(DMA2_Stream4_IRQHandler)        /* DMA2 Stream4 global interrupt */
UNUSED_IRQ(CAN2_TX_IRQHandler)             /* CAN2 TX interrupts */
UNUSED_IRQ(CAN2_RX0_IRQHandler)            /* CAN2 RX0 interrupts */
UNUSED_IRQ(CAN2_RX1_IRQHandler)            /* CAN2 RX1 interrupts */
UNUSED_IRQ(CAN2_SCE_IRQHandler)            /* CAN2 SCE interrupt */
UNUSED_IRQ(OTG_FS_IRQHandler)              /* USB On The Go FS global interrupt */
UNUSED_IRQ(DMA2_Stream5_IRQHandler)        /* DMA2 Stream5 global interrupt */
UNUSED_IRQ(DMA2_Stream6_IRQHandler)        /* DMA2 Stream6 global interrupt */
UNUSED_IRQ(DMA2_Stream7_IRQHandler)        /* DMA2 Stream7 global interrupt */
UNUSED_IRQ(USART6_IRQHandler)              /* USART6 global interrupt */
UNUSED_IRQ(I2C3_EV_IRQHandler)             /* I2C3 event interrupt */
UNUSED_IRQ(I2C3_ER_IRQHandler)             /* I2C3 error interrupt */
UNUSED_IRQ(OTG_HS_EP1_OUT_IRQHandler)      /* USB On The Go HS End Point 1 Out */
UNUSED_IRQ(OTG_HS_EP1_IN_IRQHandler)       /* USB On The Go HS End Point 1 In */
UNUSED_IRQ(OTG_HS_WKUP_IRQHandler)         /* USB On The Go HS Wakeup */
UNUSED_IRQ(OTG_HS_IRQHandler)              /* USB On The Go HS global interrupt */
UNUSED_IRQ(DCMI_IRQHandler)                /* DCMI global interrupt */
UNUSED_IRQ(SPI4_IRQHandler)                /* SPI 4 global interrupt */
UNUSED_IRQ(SAI1_IRQHandler)                /* SAI1 global interrupt */
UNUSED_IRQ(SAI2_IRQHandler)                /* SAI2 global interrupt */
UNUSED_IRQ(QuadSPI_IRQHandler)             /* QuadSPI global interrupt */
UNUSED_IRQ(HDMI_CEC_IRQHandler)            /* HDMI-CEC global interrupt */
UNUSED_IRQ(SPDIF_Rx_IRQHandler)            /* SPDIF-Rx global interrupt */
UNUSED_IRQ(FMPI2C1_IRQHandler)             /* FMPI2C1 event interrupt */
UNUSED_IRQ(FMPI2C1_error_IRQHandler)       /* FMPI2C1 error interrupt */
