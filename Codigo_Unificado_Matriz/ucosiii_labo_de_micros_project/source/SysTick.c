/*
 * SysTick.c
 *
 *  Created on: Aug 25, 2025
 *      Author: hi
 */

/**
 * @brief Initialize SysTic driver
 * @param funcallback Function to be call every SysTick
 * @return Initialization and registration succeed
 */
#include "core_cm4.h"
#include "SysTick.h"
#include "hardware.h"
#include "gpio.h"
#include "board.h"
#include "MK64F12.h"
#include  <os.h>
//extern OS_SEM  MyTimerSem;
/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/


#define RELOAD_VALUE __CORE_CLOCK__/SYSTICK_ISR_FREQUENCY_HZ  //el clock del procesador en 100M y el del systick es 1k
#define NULL ((void *)0)
#define TRUE 1
#define FALSE 0

void Timer_PIT_Init(uint32_t frequency) {
    // 1. Activar reloj del PIT
    SIM->SCGC6 |= SIM_SCGC6_PIT_MASK;

    // 2. Habilitar módulo PIT
    PIT->MCR = 0x00;

    // 3. Calcular valor de carga (Suponiendo Bus Clock de 60MHz)
    // LDVAL = (Reloj / Frecuencia) - 1
    // Para 2000Hz: (60,000,000 / 2000) - 1 = 29,999
    uint32_t busClock = 50000000; // Ajustar según tu clock real
    PIT->CHANNEL[0].LDVAL = (busClock / frequency) - 1;

    // 4. Habilitar Interrupción y Timer del canal 0
    PIT->CHANNEL[0].TCTRL |= PIT_TCTRL_TIE_MASK | PIT_TCTRL_TEN_MASK;

    // 5. Configurar NVIC para el procesador Cortex-M4
    NVIC_SetPriority(PIT0_IRQn, 2); // Prioridad alta
    NVIC_EnableIRQ(PIT0_IRQn);
}

void PIT0_IRQHandler(void) {
    OS_ERR  err;
    CPU_SR_ALLOC();

    CPU_CRITICAL_ENTER();
    OSIntEnter();          // Avisar a Micriµm que estamos en ISR
    CPU_CRITICAL_EXIT();

    // 1. Limpiar la bandera de interrupción del PIT Canal 0
    PIT->CHANNEL[0].TFLG = PIT_TFLG_TIF_MASK;

    // 2. "Dar" el semáforo para despertar a la tarea
    //OSSemPost(&MyTimerSem,OS_OPT_POST_1,&err);

    OSIntExit();           // Avisar a Micriµm que salimos (aquí ocurre el cambio de tarea)
}

/* Internal variable*/

//static SysIrqFun_t Sys_Fun;

//bool SysTick_Init (SysIrqFun_t Sys_Callback)
//{
//	SysTick->CTRL= 0x00;
//	SysTick->LOAD= RELOAD_VALUE - 1;
//	SysTick->VAL=0x00;
//	SysTick->CTRL= SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
//	if(Sys_Callback != NULL)
//	{
//		Sys_Fun=Sys_Callback;
//		return TRUE;
//	}
//	else
//	{
//		return FALSE;
//	}
//
//
//}
//
//__ISR__ SysTick_Handler (void)
//{
//	gpioWrite(PIN_LED_EXAMEN,HIGH);
//	Sys_Fun();
//	gpioWrite(PIN_LED_EXAMEN,LOW);
//}
