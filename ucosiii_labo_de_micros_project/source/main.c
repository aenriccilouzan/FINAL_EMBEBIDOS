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
void App_Run (void);
/* --- OBJETOS EXTERNOS --- */
extern OS_SEM sem_enc_giro;
extern int revisar_eventos(int8_t evento);
extern OS_SEM sem_enc_boton;
extern OS_Q   queue_tarjeta;
extern bool estado_boton;
extern bool decision_tomada;
uint8_t pan_recibido[SIZE_PAN];

extern void cambiar_estado_Boton (bool);
//extern enum {PULSADOR_CAMBIO, GIROENCODER_CAMBIO, LECTOR_TARJETA_CAMBIO};
/* LEDs */
#define LED_R_PORT            PORTB
#define LED_R_GPIO            GPIOB
#define LED_G_PORT            PORTE
#define LED_G_GPIO            GPIOE
#define LED_B_PORT            PORTB
#define LED_B_GPIO            GPIOB
#define LED_R_PIN             22
#define LED_G_PIN             26
#define LED_B_PIN             21
#define LED_B_ON()           (LED_B_GPIO->PCOR |= (1 << LED_B_PIN))
#define LED_B_OFF()          (LED_B_GPIO->PSOR |= (1 << LED_B_PIN))
#define LED_B_TOGGLE()       (LED_B_GPIO->PTOR |= (1 << LED_B_PIN))
#define LED_G_ON()           (LED_G_GPIO->PCOR |= (1 << LED_G_PIN))
#define LED_G_OFF()          (LED_G_GPIO->PSOR |= (1 << LED_G_PIN))
#define LED_G_TOGGLE()       (LED_G_GPIO->PTOR |= (1 << LED_G_PIN))
#define LED_R_ON()           (LED_R_GPIO->PCOR |= (1 << LED_R_PIN))
#define LED_R_OFF()          (LED_R_GPIO->PSOR |= (1 << LED_R_PIN))
#define LED_R_TOGGLE()       (LED_R_GPIO->PTOR |= (1 << LED_R_PIN))

/* Task Start */
#define TASKSTART_STK_SIZE 		512u
#define TASKCLOCK_STK_SIZE		128u
#define TASKSTART_PRIO 			2u
static OS_TCB TaskStartTCB;
static CPU_STK TaskStartStk[TASKSTART_STK_SIZE];

#define TASK2_PRIO              1u   // PRIORIDAD ALTA para 2kHz
#define TASK2_STK_SIZE          512u
static OS_TCB   Task2TCB;
static CPU_STK  Task2Stk[TASK2_STK_SIZE];
// Definimos la prioridad (Alta prioridad para que se parezca a una interrupción)
#define TASK_PERIODIC_PRIO  4
// Definimos el stack
static CPU_STK TaskPeriodicStk[TASKCLOCK_STK_SIZE];
// TCB de la tarea
static OS_TCB TaskPeriodicTCB;
OS_SEM  MyTimerSem;

// Prototipos de funciones
void  App_Task2kHz(void *p_arg);

extern STATE* p2state;


/* --- OBJETOS LOCALES --- */
OS_SEM  MyTimerSem;
OS_PEND_DATA pend_multi_data[3];

// --- TU ANTIGUA FUNCIÓN Sys_Fun AHORA ES ESTO ---
void TaskClock(void *p_arg) {
    OS_ERR err;
    (void)p_arg; // No usamos argumentos

    while (1) {
        // 1. PRENDER LED (Para medir en osciloscopio igual que antes)

        // 4. ESPERAR 1 TICK (Lo que hace que sea periódico)
        // Si el SysTick está a 1000Hz (1ms), esto hará que la tarea corra cada 1ms.
        OSTimeDly(1,                // Esperar 1 tick
                  OS_OPT_TIME_DLY,  // Opción relativa
                  &err);
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


    OSTaskCreate(&Task2TCB,                // TCB correcto
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
    pend_multi_data[0].PendObjPtr = (OS_PEND_OBJ *)&sem_enc_giro;
    pend_multi_data[1].PendObjPtr = (OS_PEND_OBJ *)&sem_enc_boton;
    pend_multi_data[2].PendObjPtr = (OS_PEND_OBJ *)&queue_tarjeta;
    while (1)
    {
	// Espera Eventos o Timeout (10 ticks) para actualizar timers
			OSPendMulti(pend_multi_data, 3, 10, OS_OPT_PEND_BLOCKING, &os_err);
			// A) GIRO
			if (pend_multi_data[0].RdyObjPtr == (OS_PEND_OBJ*)&sem_enc_giro) {
//				int dir = update_encoder(;
				p2state=fsm(p2state,revisar_eventos(GIROENCODER_CAMBIO));

			}
			// B) BOTON
			else if (pend_multi_data[1].RdyObjPtr == (OS_PEND_OBJ*)&sem_enc_boton) {
				//decision_tomada = true;
				cambiar_estado_Boton(estado_boton);
				p2state=fsm(p2state,revisar_eventos(PULSADOR_CAMBIO));

			}
			// C) TARJETA
			else if (pend_multi_data[2].RdyObjPtr == (OS_PEND_OBJ*)&queue_tarjeta)
            {
                // Casteamos a uint8_t* porque ahora solo mandamos el PAN
            	// 1. Agarramos la dirección de memoria que nos manda la cola
            	        uint8_t *puntero_temporal = (uint8_t *)pend_multi_data[2].RdyMsgPtr;

            	        // 2. Copiamos los 19 números a nuestro arreglo real
            	        for (int i = 0; i < 19; i++) {
            	            pan_recibido[i] = puntero_temporal[i];
            	        }
            }

//			timerUpdate(); // Actualizar timers de software
    }
}

void App_Task2kHz(void *p_arg) {
    OS_ERR  err;

    // Crear el semáforo (inicializado en 0)
    OSSemCreate(&MyTimerSem, "Semaforo 2kHz", 0, &err);

    // Inicializar el hardware del PIT a 2000 Hz
    Timer_PIT_Init(2000);

    while (DEF_ON) {
        // Esperar la señal del PIT (bloqueante)
        OSSemPend(&MyTimerSem,
                  0,
                  OS_OPT_PEND_BLOCKING,
                  NULL,
                  &err);

        if (err == OS_ERR_NONE) {
            // ==========================================
            // AQUÍ PONES TU FUNCIÓN O LÓGICA
            // Se ejecutará exactamente cada 2kHz
            // ==========================================
            interrupt_fun();
        }
    }
}
int main(void) {
    OS_ERR err;

#if (CPU_CFG_NAME_EN == DEF_ENABLED)
    CPU_ERR  cpu_err;
#endif

    hw_Init();
    hw_DisableInterrupts();
    App_Init(); /* Program-specific setup */
    hw_EnableInterrupts();


    /* RGB LED */
    SIM->SCGC5 |= (SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTE_MASK);
    LED_B_PORT->PCR[LED_B_PIN] = PORT_PCR_MUX(1);
    LED_G_PORT->PCR[LED_G_PIN] = PORT_PCR_MUX(1);
    LED_R_PORT->PCR[LED_R_PIN] = PORT_PCR_MUX(1);
    LED_B_GPIO->PDDR |= (1 << LED_B_PIN);
    LED_G_GPIO->PDDR |= (1 << LED_G_PIN);
    LED_R_GPIO->PDDR |= (1 << LED_R_PIN);
    LED_B_GPIO->PSOR |= (1 << LED_B_PIN);
    LED_G_GPIO->PSOR |= (1 << LED_G_PIN);
    LED_R_GPIO->PSOR |= (1 << LED_R_PIN);

    OSInit(&err);
 #if OS_CFG_SCHED_ROUND_ROBIN_EN > 0u
	 /* Enable task round robin. */
	 OSSchedRoundRobinCfg((CPU_BOOLEAN)1, 0, &err);
 #endif
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
