/***************************************************************************//**
  @file     board.h
  @brief    Board management
  @author   Nicolás Magliola
 ******************************************************************************/

#ifndef _BOARD_H_
#define _BOARD_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "gpio.h"


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
//#define MODE_GPIO
#define MODE_SHIFT

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

/***** BOARD defines **********************************************************/
#ifdef MODE_GPIO
#define PRUEBA PORTNUM2PIN(PD,1)
#define PINES_DISP  { \
    PORTNUM2PIN(PC,16), \
    PORTNUM2PIN(PC,17), \
    PORTNUM2PIN(PB,9), \
    PORTNUM2PIN(PA,1), \
    PORTNUM2PIN(PB,23), \
    PORTNUM2PIN(PA,2),  \
    PORTNUM2PIN(PC,2),  \
    PORTNUM2PIN(PC,3)  \
}
#define PRUEBA22 PORTNUM2PIN(PC, 10);


#define PINES_DECO  { \
    PORTNUM2PIN(PC,4), \
    PORTNUM2PIN(PD,0)  \
}
#endif


#ifdef MODE_SHIFT

#define PINES_SHIFT  { \
    PORTNUM2PIN(PC,16), \
    PORTNUM2PIN(PC,1), \
    PORTNUM2PIN(PC,18), \
}
enum { PIN_SHIFT_REGISTER, PIN_CLOCK_SHIFT_REGISTER, PIN_LATCH};

#endif


/***** BOARD defines **********************************************************/

//Configuraciones del TP1
#define BOTON_ENCODER 	PORTNUM2PIN(PC, 3)
#define A_ENCODER 		PORTNUM2PIN(PC, 5)
#define B_ENCODER 		PORTNUM2PIN(PC, 4)

#define ENABLE_TARJETA_MAGNETICA 	PORTNUM2PIN(PB, 9) //amarillo
#define DATA_TARJETA 				PORTNUM2PIN(PB, 0) //azul
#define CLOCK_TARJETA_MAGNETICA 	PORTNUM2PIN(PB, 2) //verde

// On Board User LEDs
#define PIN_LED_RED     PORTNUM2PIN(PB,17) // PTB22
#define PIN_LED_GREEN   PORTNUM2PIN(PB,19) // PTE26
#define PIN_LED_BLUE    PORTNUM2PIN(PB,21) // PTB21

#define LED_ACTIVE      LOW

//#define PIN_LED_EXAMEN PORTNUM2PIN(PB,2)
//#define PIN_LED_EXAMEN1 PORTNUM2PIN(PB,11)

// On Board User Switches
//#define PIN_SW2		PORTNUM2PIN(PC,6) // PTC6
//#define PIN_SW3     PORTNUM2PIN(PA,4) // PTA4

#define SW_ACTIVE		LOW		// Switch is connected to ground
//#define SW_INPUT_TYPE	INPUT	// Switch has external pull up

/*******************************************************************************
 ******************************************************************************/

#endif // _BOARD_H_
