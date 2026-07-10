/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2018 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _BOARD_H_
#define _BOARD_H_

#include "clock_config.h"
#include "fsl_gpio.h"
#include "fsl_port.h"
#include "gpio.h"

/******************************************************************************
* Copiado de nuestro propio board.h
******************************************************************************/

#define MODE_SHIFT

#define PINES_SHIFT  { \
    PORTNUM2PIN(PC,16), \
    PORTNUM2PIN(PC,1), \
    PORTNUM2PIN(PC,18), \
}
enum { PIN_SHIFT_REGISTER, PIN_CLOCK_SHIFT_REGISTER, PIN_LATCH};

//Configuraciones del TP1
#define BOTON_ENCODER 	PORTNUM2PIN(PC, 3)
#define A_ENCODER 		PORTNUM2PIN(PC, 5)
#define B_ENCODER 		PORTNUM2PIN(PC, 4)

#define ENABLE_TARJETA_MAGNETICA 	PORTNUM2PIN(PB, 9) //amarillo
#define DATA_TARJETA 				PORTNUM2PIN(PB, 0) //azul
#define CLOCK_TARJETA_MAGNETICA 	PORTNUM2PIN(PB, 2) //verde

// On Board User LEDs
#define PIN_LED_RED     PORTNUM2PIN(PB,17) // PTB17
#define PIN_LED_GREEN   PORTNUM2PIN(PB,19) // PTE19
#define PIN_LED_BLUE    PORTNUM2PIN(PB,21) // PTB21

#define LED_ACTIVE      LOW

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* The board name */
#define BOARD_NAME "FRDM-K64F"

/* The UART to use for debug messages. */
#define BOARD_DEBUG_UART_TYPE     kSerialPort_Uart
#define BOARD_DEBUG_UART_BASEADDR (uint32_t) UART0
#define BOARD_DEBUG_UART_INSTANCE 0U
#define BOARD_DEBUG_UART_CLKSRC   SYS_CLK
#define BOARD_DEBUG_UART_CLK_FREQ CLOCK_GetCoreSysClkFreq()
#define BOARD_UART_IRQ            UART0_RX_TX_IRQn
#define BOARD_UART_IRQ_HANDLER    UART0_RX_TX_IRQHandler

#ifndef BOARD_DEBUG_UART_BAUDRATE
#define BOARD_DEBUG_UART_BAUDRATE 115200
#endif /* BOARD_DEBUG_UART_BAUDRATE */

/*! @brief The ENET PHY address. */
#define BOARD_ENET0_PHY_ADDRESS (0x00U) /* Phy address of enet port 0. */

/* Define the port interrupt number for the board switches */
#ifndef BOARD_SW2_GPIO
#define BOARD_SW2_GPIO GPIOC
#endif
#ifndef BOARD_SW2_PORT
#define BOARD_SW2_PORT PORTC
#endif
#ifndef BOARD_SW2_GPIO_PIN
#define BOARD_SW2_GPIO_PIN 6U
#endif
#define BOARD_SW2_IRQ         PORTC_IRQn
#define BOARD_SW2_IRQ_HANDLER PORTC_IRQHandler
#define BOARD_SW2_NAME        "SW2"

#ifndef BOARD_SW3_GPIO
#define BOARD_SW3_GPIO GPIOA
#endif
#ifndef BOARD_SW3_PORT
#define BOARD_SW3_PORT PORTA
#endif
#ifndef BOARD_SW3_GPIO_PIN
#define BOARD_SW3_GPIO_PIN 4U
#endif
#define BOARD_SW3_IRQ         PORTA_IRQn
#define BOARD_SW3_IRQ_HANDLER PORTA_IRQHandler
#define BOARD_SW3_NAME        "SW3"

#define LLWU_SW_GPIO        BOARD_SW2_GPIO
#define LLWU_SW_PORT        BOARD_SW2_PORT
#define LLWU_SW_GPIO_PIN    BOARD_SW2_GPIO_PIN
#define LLWU_SW_IRQ         BOARD_SW2_IRQ
#define LLWU_SW_IRQ_HANDLER BOARD_SW2_IRQ_HANDLER
#define LLWU_SW_NAME        BOARD_SW2_NAME

/* Board led color mapping */
#define LOGIC_LED_ON  0U
#define LOGIC_LED_OFF 1U
#ifndef BOARD_LED_RED_GPIO
#define BOARD_LED_RED_GPIO GPIOB
#endif
#define BOARD_LED_RED_GPIO_PORT PORTB
#ifndef BOARD_LED_RED_GPIO_PIN
#define BOARD_LED_RED_GPIO_PIN 22U
#endif
#ifndef BOARD_LED_GREEN_GPIO
#define BOARD_LED_GREEN_GPIO GPIOE
#endif
#define BOARD_LED_GREEN_GPIO_PORT PORTE
#ifndef BOARD_LED_GREEN_GPIO_PIN
#define BOARD_LED_GREEN_GPIO_PIN 26U
#endif
#ifndef BOARD_LED_BLUE_GPIO
#define BOARD_LED_BLUE_GPIO GPIOB
#endif
#define BOARD_LED_BLUE_GPIO_PORT PORTB
#ifndef BOARD_LED_BLUE_GPIO_PIN
#define BOARD_LED_BLUE_GPIO_PIN 21U
#endif

#define BOARD_ARDUINO_INT_IRQ   (PORTC_IRQn)
#define BOARD_ARDUINO_I2C_IRQ   (I2C0_IRQn)
#define BOARD_ARDUINO_I2C_INDEX (0)

#define LED_RED_INIT(output)                                           \
    GPIO_PinWrite(BOARD_LED_RED_GPIO, BOARD_LED_RED_GPIO_PIN, output); \
    BOARD_LED_RED_GPIO->PDDR |= (1U << BOARD_LED_RED_GPIO_PIN)                         /*!< Enable target LED_RED */
#define LED_RED_ON()  GPIO_PortClear(BOARD_LED_RED_GPIO, 1U << BOARD_LED_RED_GPIO_PIN) /*!< Turn on target LED_RED */
#define LED_RED_OFF() GPIO_PortSet(BOARD_LED_RED_GPIO, 1U << BOARD_LED_RED_GPIO_PIN)   /*!< Turn off target LED_RED */
#define LED_RED_TOGGLE() \
    GPIO_PortToggle(BOARD_LED_RED_GPIO, 1U << BOARD_LED_RED_GPIO_PIN) /*!< Toggle on target LED_RED */

#define LED_GREEN_INIT(output)                                             \
    GPIO_PinWrite(BOARD_LED_GREEN_GPIO, BOARD_LED_GREEN_GPIO_PIN, output); \
    BOARD_LED_GREEN_GPIO->PDDR |= (1U << BOARD_LED_GREEN_GPIO_PIN) /*!< Enable target LED_GREEN */
#define LED_GREEN_ON() \
    GPIO_PortClear(BOARD_LED_GREEN_GPIO, 1U << BOARD_LED_GREEN_GPIO_PIN) /*!< Turn on target LED_GREEN */
#define LED_GREEN_OFF() \
    GPIO_PortSet(BOARD_LED_GREEN_GPIO, 1U << BOARD_LED_GREEN_GPIO_PIN) /*!< Turn off target LED_GREEN */
#define LED_GREEN_TOGGLE() \
    GPIO_PortToggle(BOARD_LED_GREEN_GPIO, 1U << BOARD_LED_GREEN_GPIO_PIN) /*!< Toggle on target LED_GREEN */

#define LED_BLUE_INIT(output)                                            \
    GPIO_PinWrite(BOARD_LED_BLUE_GPIO, BOARD_LED_BLUE_GPIO_PIN, output); \
    BOARD_LED_BLUE_GPIO->PDDR |= (1U << BOARD_LED_BLUE_GPIO_PIN) /*!< Enable target LED_BLUE */
#define LED_BLUE_ON()                                                                                 \
    GPIO_PortClear(BOARD_LED_BLUE_GPIO, 1U << BOARD_LED_BLUE_GPIO_PIN) /*!< Turn on target LED_BLUE \ \
                                                                        */
#define LED_BLUE_OFF()                                                                               \
    GPIO_PortSet(BOARD_LED_BLUE_GPIO, 1U << BOARD_LED_BLUE_GPIO_PIN) /*!< Turn off target LED_BLUE \ \
                                                                      */
#define LED_BLUE_TOGGLE() \
    GPIO_PortToggle(BOARD_LED_BLUE_GPIO, 1U << BOARD_LED_BLUE_GPIO_PIN) /*!< Toggle on target LED_BLUE */

#define BOARD_ACCEL_I2C_BASEADDR   I2C0
#define BOARD_ACCEL_I2C_CLOCK_FREQ CLOCK_GetFreq(I2C0_CLK_SRC)

/* ERPC DSPI configuration */
#define ERPC_BOARD_SPI_SLAVE_READY_USE_GPIO (1)
#define ERPC_BOARD_DSPI_BASEADDR            SPI0
#define ERPC_BOARD_DSPI_BAUDRATE            500000U
#define ERPC_BOARD_DSPI_CLKSRC              DSPI0_CLK_SRC
#define ERPC_BOARD_DSPI_CLK_FREQ            CLOCK_GetFreq(DSPI0_CLK_SRC)
#define ERPC_BOARD_DSPI_INT_GPIO            GPIOB
#define ERPC_BOARD_DSPI_INT_PORT            PORTB
#define ERPC_BOARD_DSPI_INT_PIN             2U
#define ERPC_BOARD_DSPI_INT_PIN_IRQ         PORTB_IRQn
#define ERPC_BOARD_DSPI_INT_PIN_IRQ_HANDLER PORTB_IRQHandler

/* DAC base address */
#define BOARD_DAC_BASEADDR DAC0

/* Board accelerometer driver */
#define BOARD_ACCEL_FXOS

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/*******************************************************************************
 * API
 ******************************************************************************/
void BOARD_InitDebugConsole(void);
#if defined(SDK_I2C_BASED_COMPONENT_USED) && SDK_I2C_BASED_COMPONENT_USED
void BOARD_I2C_Init(I2C_Type *base, uint32_t clkSrc_Hz);
status_t BOARD_I2C_Send(I2C_Type *base,
                        uint8_t deviceAddress,
                        uint32_t subAddress,
                        uint8_t subaddressSize,
                        uint8_t *txBuff,
                        uint8_t txBuffSize);
status_t BOARD_I2C_Receive(I2C_Type *base,
                           uint8_t deviceAddress,
                           uint32_t subAddress,
                           uint8_t subaddressSize,
                           uint8_t *rxBuff,
                           uint8_t rxBuffSize);
void BOARD_Accel_I2C_Init(void);
status_t BOARD_Accel_I2C_Send(uint8_t deviceAddress, uint32_t subAddress, uint8_t subaddressSize, uint32_t txBuff);
status_t BOARD_Accel_I2C_Receive(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subaddressSize, uint8_t *rxBuff, uint8_t rxBuffSize);
#endif /* SDK_I2C_BASED_COMPONENT_USED */
#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _BOARD_H_ */
