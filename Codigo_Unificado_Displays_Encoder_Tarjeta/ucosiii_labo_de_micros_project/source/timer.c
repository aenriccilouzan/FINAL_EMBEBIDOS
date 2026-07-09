/*
 * timer.c
 *
 *  Created on: Aug 26, 2025
 *      Author: julie
 */
#include "timer.h"
#include <stdbool.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define MASK_ID_NUM 0b00001111U
#define MASK_ID_ACTIVE 0b01000000U
#define MASK_ID_PERIODIC 0b00100000U
#define MASK_ID_CRITICAL 0b00010000U
#define NULL ((void *)0)
#define TRUE 1
#define FALSE 0
/* ============================================================================
 * Module internal variables
 * ==========================================================================*/

static tim_callback_t timer_fun_pun[TIMERS_MAX_CANT] ={0}; //ARREGLO DE PUNTEROS A FUNCIONES QUE VAN A LLAMAR LOS TIMERS CUANDO SE ACTIVEN.
static tim_id_t ids[TIMERS_MAX_CANT] ={0};
static tim_tick_t timer_ticks[TIMERS_MAX_CANT] ={0};
static tim_tick_t timer_ticks_max[TIMERS_MAX_CANT] ={0};

/* Prototipos de funciones internas */
//para el id no usamos el último ID porque se puede superponer con timer invalid

void timerInit(void){
	bool a;
	for(int i=0;i<TIMERS_MAX_CANT;i++)
	{
		ids[i]=(ids[i] | MASK_ID_ACTIVE);
	}
	//a=SysTick_Init(&interrupt_fun);
}

tim_id_t timerGetId(void){
	for(int i=0;i<TIMERS_MAX_CANT;i++)
	{
		if(ids[i] & MASK_ID_ACTIVE)// si el bit 7 es 1 el timer está disponible (o sea no esta siendo usado)
		{
			return i;
		}
	}
	return TIMER_INVALID_ID;
}

tim_id_t timerStart(float ticks, uint8_t mode, tim_callback_t callback){
	tim_id_t id= timerGetId();
	uint8_t index=id & MASK_ID_NUM;
	ids[index] |= (id & MASK_ID_NUM);
	if(ids[index] & MASK_ID_ACTIVE)
	{
		switch(mode)
		{
			case TIM_MODE_SINGLESHOT_NO_CRITICAL:
				ids[index]=(ids[index] & ~MASK_ID_CRITICAL) & (~MASK_ID_PERIODIC);
				break;
			case TIM_MODE_SINGLESHOT_CRITICAL:
				ids[index]=(ids[index] | MASK_ID_CRITICAL) & (~MASK_ID_PERIODIC);
				break;
			case TIM_MODE_PERIODIC_NO_CRITICAL:
				ids[index]=(ids[index] & ~MASK_ID_CRITICAL) | MASK_ID_PERIODIC;
				break;
			case TIM_MODE_PERIODIC_CRITICAL:
				ids[index]=(ids[index] | MASK_ID_CRITICAL) | MASK_ID_PERIODIC;
				break;
			default:
				return TIMER_INVALID_ID;
				break;
		}
		if(callback != NULL)
		{
			timer_fun_pun[index]=callback;
		}
		else
		{
			return TIMER_INVALID_ID;
		}

		timer_ticks[index]=TIMER_MS2TICKS(ticks);
		timer_ticks_max[index]=TIMER_MS2TICKS(ticks);
		//timer_ticks[index]=ticks;
		//timer_ticks_max[index]=ticks;
		ids[index]=(ids[index] & (~MASK_ID_ACTIVE));
		return id;
	}
	else
	{
		return TIMER_INVALID_ID;
	}
}

void timerStop(tim_id_t id){
	ids[id & MASK_ID_NUM]=ids[id & MASK_ID_NUM] | MASK_ID_ACTIVE;
}

bool timerExpired(tim_id_t id){ //asumimos que el usuario es inteligente (?) y esto lo usa solamente si el timer esta activo
	return (timer_ticks[id & MASK_ID_NUM]==0);
}

void timerUpdate(void){// esto se hace en el main loop, no en el handler del systick
	for(int i=0;i<TIMERS_MAX_CANT;i++){
		if((ids[i] & MASK_ID_ACTIVE) == 0) // si el bit 7 es 0 el timer esta siendo utilizado
		{
			if((ids[i] & MASK_ID_CRITICAL) == 0) // asumiendo que un 0 en el 5to bit es timer no crítico
			{
				if(timerExpired(ids[i]))
				{
					if(ids[i] & MASK_ID_PERIODIC) // asumiendo que un 1 en el 6to bit es timer periódico
					{
						timer_ticks[i]=timer_ticks_max[i];
					}
					else
					{
						timerStop(ids[i]);
					}
					timer_fun_pun[i]();
				}
			}
		}
	}
}

void interrupt_fun(void){
	for(int i=0;i<TIMERS_MAX_CANT;i++){
		if(((ids[i] & MASK_ID_ACTIVE) == 0)) // si el bit 7 es 0 el timer esta siendo utilizado
		{
			timer_ticks[i]--;
			if(ids[i] & MASK_ID_CRITICAL) // asumiendo que un 1 en el 5to bit es timer crítico
			{
				if(timerExpired(ids[i]))
				{
					if(ids[i] & MASK_ID_PERIODIC) // asumiendo que un 1 en el 6to bit es timer periódico
					{
						timer_ticks[i]=timer_ticks_max[i];
					}
					else
					{
						timerStop(ids[i]);
					}
					timer_fun_pun[i]();
				}
			}
		}
	}
}
void timerRestart(tim_id_t id)
{
	timer_ticks[id&MASK_ID_NUM]=timer_ticks_max[id&MASK_ID_NUM];
}

