/***************************************************************************//**
  @file     timer.h
  @brief    Timer driver. Advance implementation, Non-Blocking services
  @author   Nicolás Magliola
 ******************************************************************************/

#ifndef _TIMER_H_
#define _TIMER_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include "SysTick.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

//#define TIMER_TICK_MS       1
//#define TIMER_MS2TICKS(ms)  ((ms)/TIMER_TICK_MS)

#define TIMERS_MAX_CANT     16
#define TIMER_INVALID_ID    255

//#define TIMER_TICK_US       (1000000U/(SYSTICK_ISR_FREQUENCY_HZ))
#define TIMER_TICK_US       500
#define TIMER_MS2TICKS(ms)  (uint32_t)((((float)ms)/(float)TIMER_TICK_US)*(float)1000)


/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

// Timer Modes
enum { TIM_MODE_SINGLESHOT_NO_CRITICAL, TIM_MODE_SINGLESHOT_CRITICAL, TIM_MODE_PERIODIC_NO_CRITICAL, TIM_MODE_PERIODIC_CRITICAL, CANT_TIM_MODES };

// Timer alias
typedef uint32_t tim_tick_t;
typedef uint8_t tim_id_t;
typedef void (*tim_callback_t)(void);


/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Initialice timer and corresponding peripheral
 */
void timerInit(void);


/**
 * @brief Request a timer
 * @return ID of the timer to use
 */
tim_id_t timerGetId(void);


/**
 * @brief Begin to run a new timer
 * @param id ID of the timer to start
 * @param ticks time until timer expires, in ticks
 * @param mode 0 = NO_CRÍTICO Y SINGLESHOT; 1 = CRÍTICO Y SINGLESHOT; 2 = NO_CRÍTICO Y PERIÓDICO; 3 = CRÍTICO Y PERIÓDICO
 * @return true = timer start succeed
 */
tim_id_t timerStart(float ticks, uint8_t mode, tim_callback_t callback);


/**
 * @brief Finish to run a timer
 * @param id ID of the timer to stop
 */
void timerStop(tim_id_t id);


/**
 * @brief Verify if a timer has run timeout
 * @param id ID of the timer to check for expiration
 * @return true = timer expired
 */
bool timerExpired(tim_id_t id);


/**
 * @brief Call respective callbacks if timeout ocurrs. Must be call from main loop.
 */
void timerUpdate(void);

void interrupt_fun(void);

void timerRestart(tim_id_t);



/*******************************************************************************
 ******************************************************************************/

#endif // _TIMER_H_
