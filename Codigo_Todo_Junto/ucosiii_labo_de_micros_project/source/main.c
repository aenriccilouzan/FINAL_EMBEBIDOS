#include "hardware.h"
#include  <os.h>
#include "MK64F12.h"
#include "MK64F12_features.h"
#include "SysTick.h"
#include "timer.h"
#include "encoder.h"
#include "display.h"
#include "fsm.h"
#include "tarjeta_magnetica.h"
#include <stddef.h>

void App_Init (void);

/* --- LIBRERÍAS DE LA MATRIZ --- */
#include "DMAPixel.h"

/* --- VARIABLES DE LA MATRIZ --- */
OS_Q AppQ; // Cola para mandar comandos a la matriz

/* --- CONFIGURACIÓN DE TAREA MATRIZ --- */
#define TASK_MATRIZ_PRIO         3u
#define TASK_MATRIZ_STK_SIZE     512u
static OS_TCB  TaskMatrizTCB;
static CPU_STK TaskMatrizStk[TASK_MATRIZ_STK_SIZE];

/* --- OBJETOS EXTERNOS --- */
extern OS_SEM sem_enc_giro;
extern int revisar_eventos(int8_t evento);
extern OS_SEM sem_enc_boton;
extern OS_Q   queue_tarjeta;
extern bool estado_boton;
extern void cambiar_estado_Boton (bool);
extern STATE* p2state;

/* Task Start */
#define TASKSTART_STK_SIZE 		1024u
#define TASKSTART_PRIO 			2u
static OS_TCB TaskStartTCB;
static CPU_STK TaskStartStk[TASKSTART_STK_SIZE];

#define TASK2_PRIO              1u   // PRIORIDAD ALTA para 2kHz
#define TASK2_STK_SIZE          1024u
static OS_TCB   Task2TCB;
static CPU_STK  Task2Stk[TASK2_STK_SIZE];

OS_SEM  MyTimerSem;

// Prototipos de funciones
void  App_Task2kHz(void *p_arg);

/* --- OBJETOS LOCALES --- */
OS_PEND_DATA pend_multi_data[3];


uint8_t pan_recibido[SIZE_PAN];


/* ============================================================================
 * TAREA DE LA MATRIZ DE LEDS (Ex Task 2)
 * ============================================================================ */
static void Task_Matriz(void *p_arg) {
    (void)p_arg;
    OS_ERR os_err = OS_ERR_NONE;
    void *p_msg = NULL;
    OS_MSG_SIZE msg_size;
    CPU_TS ts;
    char *command_ptr;

    colorWipe(Black,300);

    while (1) {
        /* 1. Esperar mensajes en la cola (Modo Bloqueante) */
        p_msg = OSQPend(&AppQ, 0, OS_OPT_PEND_BLOCKING, &msg_size, &ts, &os_err);

        /* 2. Procesar mensaje si existe */
        if (os_err == OS_ERR_NONE && p_msg != NULL) {
            command_ptr = (char *)p_msg;
            char cmd = *command_ptr;

            /* Procesar comandos de ocupación de pisos */
            if (cmd == 'A' || cmd == 'B' || cmd == 'C') {
                IncrementOccupancy(cmd);
            }
            /* Procesar comandos de cambio de brillo */
            else if (cmd >= '1' && cmd <= '5') {
                 g_BrightnessLevel = cmd - '0';
                 DrawOccupancy();
            }
        }
    }
}

static void TaskStart(void *p_arg) {
    (void)p_arg;
    OS_ERR os_err;

    /* Initialize the uC/CPU Services. */
    CPU_Init();

#if OS_CFG_STAT_TASK_EN > 0u
    /* (optional) Compute CPU capacity with no task running */
    OSStatTaskCPUUsageInit(&os_err);
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN
    CPU_IntDisMeasMaxCurReset();
#endif

    /* --- INICIALIZACIÓN DE OBJETOS DE LA MATRIZ --- */
	OSQCreate(&AppQ, "Cola Matriz", 1, &os_err);
	OSSemCreate(&DMA_SemEndTransfer, "Sem DMA Matriz", 0, &os_err);

	/* --- INICIALIZACIÓN DE HARDWARE DE LA MATRIZ --- */
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();

	initBuffer();
	DMA_Init();
	RNG_Init();

	/* ¡CRÍTICO! Bajar prioridad de interrupciones para no colgar el RTOS */
	NVIC_SetPriority(DMA0_IRQn, 5);
	NVIC_SetPriority(DMA1_IRQn, 5);
	NVIC_SetPriority(DMA2_IRQn, 5);
	NVIC_SetPriority(FTM0_IRQn, 5);
	NVIC_SetPriority(FTM1_IRQn, 5);
	NVIC_SetPriority(FTM2_IRQn, 5);

	CPU_CRITICAL_EXIT();

	/* Crear tarea de la Matriz */
	    OSTaskCreate(&TaskMatrizTCB,
	                 "Task Matriz",
	                 Task_Matriz,
	                 0u,
	                 TASK_MATRIZ_PRIO,
	                 &TaskMatrizStk[0u],
	                 TASK_MATRIZ_STK_SIZE / 10u,
	                 TASK_MATRIZ_STK_SIZE,
	                 0u, 0u, 0u,
	                 (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
	                 &os_err);

    OSTaskCreate(&Task2TCB,
                     "Task 2kHz",
                     App_Task2kHz,
                     0u,
                     TASK2_PRIO,
                     &Task2Stk[0u],
                     TASK2_STK_SIZE / 10u,
                     TASK2_STK_SIZE,
                     0u, 0u, 0u,
                     (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                     &os_err);

    OSQCreate(&queue_tarjeta, "Queue TM", 1, &os_err);
    OSSemCreate(&sem_enc_giro,  "Sem Giro",  0, &os_err);
    OSSemCreate(&sem_enc_boton, "Sem Boton", 0, &os_err);

    while (1)
    {
    	pend_multi_data[0].PendObjPtr = (OS_PEND_OBJ *)&sem_enc_giro;
    	pend_multi_data[1].PendObjPtr = (OS_PEND_OBJ *)&sem_enc_boton;
    	pend_multi_data[2].PendObjPtr = (OS_PEND_OBJ *)&queue_tarjeta;

    	OS_ERR os_err2;
    	// Espera Eventos o Timeout (10 ticks) para actualizar timers
		OSPendMulti(pend_multi_data, 3, 0, OS_OPT_PEND_BLOCKING, &os_err2);

		if (os_err2 == OS_ERR_NONE) {
			// A) GIRO
			if (pend_multi_data[0].RdyObjPtr == (OS_PEND_OBJ*)&sem_enc_giro) {
				p2state=fsm(p2state,revisar_eventos(GIROENCODER_CAMBIO));
			}
			// B) BOTON
			else if (pend_multi_data[1].RdyObjPtr == (OS_PEND_OBJ*)&sem_enc_boton) {
				cambiar_estado_Boton(estado_boton);
				p2state=fsm(p2state,revisar_eventos(PULSADOR_CAMBIO));
			}
			// C) TARJETA

			else if (pend_multi_data[2].RdyObjPtr == (OS_PEND_OBJ*)&queue_tarjeta)
			{
				// Casteamos a uint8_t* porque ahora solo mandamos el PAN
				// 1. Agarramos la dirección de memoria que nos manda la cola
				uint8_t *puntero_temporal = (uint8_t *)pend_multi_data[2].RdyMsgPtr;

				// 2. Copiamos los 16 números a nuestro arreglo real
				for (int i = 0; i < 16; i++) {
					pan_recibido[i] = puntero_temporal[i];
				}
				p2state=fsm(p2state,revisar_eventos(LECTOR_TARJETA_CAMBIO));
			}
		}
    }
}

void App_Task2kHz(void *p_arg) {
    OS_ERR  err;

    // Crear el semáforo (inicializado en 0)
    OSSemCreate(&MyTimerSem, "Semaforo 2kHz", 0, &err);

    // Inicializar el hardware del PIT a 2000 Hz
    Timer_PIT_Init(2000);

    while (1) {
        // Esperar la señal del PIT (bloqueante)
        OSSemPend(&MyTimerSem,
                  0,
                  OS_OPT_PEND_BLOCKING,
                  NULL,
                  &err);

        if (err == OS_ERR_NONE) {
            interrupt_fun();
        }
    }
}

int main(void) {
    OS_ERR err;

    hw_Init();
    hw_DisableInterrupts();
    App_Init(); /* Program-specific setup */
    hw_EnableInterrupts();

    OSInit(&err);

//#if OS_CFG_SCHED_ROUND_ROBIN_EN > 0u
	//OSSchedRoundRobinCfg((CPU_BOOLEAN)1, 0, &err);
//#endif

    OS_CPU_SysTickInit(SystemCoreClock / (uint32_t)OSCfg_TickRate_Hz);

    OSTaskCreate(&TaskStartTCB,
                 "App Task Start",
                  TaskStart,
                  0u,
                  TASKSTART_PRIO,
                 &TaskStartStk[0u],
                 (TASKSTART_STK_SIZE / 10u),
                  TASKSTART_STK_SIZE,
                  0u,
                  0u,
                  0u,
                 (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR | OS_OPT_TASK_SAVE_FP),
                 &err);

    OSStart(&err);

	/* Should Never Get Here */
    while (1) {

    }
}
