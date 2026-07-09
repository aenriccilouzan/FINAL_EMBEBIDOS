#include "hardware.h"
#include "DMAPixel.h"
#include "board.h"
#include <stdbool.h>
#include <os.h>

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
#define TASKSTART_PRIO 			2u
static OS_TCB TaskStartTCB;
static CPU_STK TaskStartStk[TASKSTART_STK_SIZE];

/* Task 2 */
#define TASK2_STK_SIZE			256u
#define TASK2_STK_SIZE_LIMIT	(TASK2_STK_SIZE / 10u)
#define TASK2_PRIO              3u
static OS_TCB Task2TCB;
static CPU_STK Task2Stk[TASK2_STK_SIZE];

/* Task 3 */
#define TASK3_STK_SIZE			256u
#define TASK3_STK_SIZE_LIMIT	(TASK3_STK_SIZE / 10u)
#define TASK3_PRIO              4u
static OS_TCB Task3TCB;
static CPU_STK Task3Stk[TASK3_STK_SIZE];

/* Example queue */
static OS_Q AppQ;

static char cmd_brillo_bajo = '1';
static char cmd_brillo_alto = '5';

/* Variables globales para los mensajes de la cola */
static char cmd_piso1 = 'A';
static char cmd_piso2 = 'B';
static char cmd_piso3 = 'C';

static void Task2(void *p_arg) {
    (void)p_arg;
    OS_ERR os_err = OS_ERR_NONE;

    void *p_msg =NULL;
    OS_MSG_SIZE msg_size;
    CPU_TS ts;

    /* Puntero para interpretar el mensaje.
    Asumimos que envían un puntero a un entero o un char */
    char *command_ptr;

    while (1) {

    	/* 1. REVISAR LA COLA (Modo Bloqueante) */
		p_msg = OSQPend(&AppQ, 0, OS_OPT_PEND_BLOCKING, &msg_size, &ts, &os_err);

		/* 2. PROCESAR MENSAJE SI EXISTE */
		if (os_err == OS_ERR_NONE && p_msg != NULL) {
			command_ptr = (char *)p_msg;
			char cmd = *command_ptr;

			/* Procesar comandos de ocupación */
			if (cmd == 'A' || cmd == 'B' || cmd == 'C') {
				IncrementOccupancy(cmd);
			}

			else if (cmd >= '1' && cmd <= '5') {
				 g_BrightnessLevel = cmd - '0';
				 DrawOccupancy(); // Redibujar con nuevo brillo
			}
		}

		/* 3. EJECUTAR EFECTOS */
		/* El brillo se aplicará automáticamente dentro de setPixel en estos efectos */
    	//theaterChase(Red,1000);

    	//colorWipe(Red,400);//red swipe
		//colorWipe(Black,300);
		//colorWipe(Green,400);//green swipe
		//colorWipe(Black,300);
		//colorWipe(Blue,400);//blue swipe
		//colorWipe(Black,300);
		//colorWipe(Purple,300);//purple
		//colorWipe(Black,300);

		//FadeInOut(Green);

		//Twinkle(Yellow,50,800,1,1);
    }
}


static void Task3(void *p_arg) {
    (void)p_arg;
    OS_ERR os_err;
    //CPU_TS  ts;

    while (1) {
    	//codigo de prueba de la ocupacion
    	/* Esperamos 2 segundos para ver el estado actual */
		OSTimeDlyHMSM(0u, 0u, 2u, 0u, OS_OPT_TIME_HMSM_STRICT, &os_err);

		/* Llega una persona al Piso 1 ('A') */
		OSQPost(&AppQ, &cmd_piso1, sizeof(char), OS_OPT_POST_FIFO, &os_err);

		/* Esperamos 2 segundos */
		OSTimeDlyHMSM(0u, 0u, 2u, 0u, OS_OPT_TIME_HMSM_STRICT, &os_err);

		/* Llega una persona al Piso 2 ('B') */
		OSQPost(&AppQ, &cmd_piso2, sizeof(char), OS_OPT_POST_FIFO, &os_err);

		/* Esperamos 2 segundos */
		OSTimeDlyHMSM(0u, 0u, 2u, 0u, OS_OPT_TIME_HMSM_STRICT, &os_err);

		/* Llega una persona al Piso 3 ('C') */
		OSQPost(&AppQ, &cmd_piso3, sizeof(char), OS_OPT_POST_FIFO, &os_err);

		/*Codigo de prueba del brillo
    	// Cada 5 segundos, alterna el brillo
		OSTimeDlyHMSM(0u, 0u, 5u, 0u, OS_OPT_TIME_HMSM_STRICT, &os_err);

		OSQPost(&AppQ, &cmd_brillo_bajo, sizeof(char), OS_OPT_POST_FIFO, &os_err);

		OSTimeDlyHMSM(0u, 0u, 5u, 0u, OS_OPT_TIME_HMSM_STRICT, &os_err);

		OSQPost(&AppQ, &cmd_brillo_alto, sizeof(char), OS_OPT_POST_FIFO, &os_err);
    	*/
    }
}



static void TaskStart(void *p_arg) {
    (void)p_arg;
    OS_ERR os_err;

    /* Initialize the uC/CPU Services. */
    CPU_Init();

    /* Create semaphore */
    OSSemCreate(&DMA_SemEndTransfer, "Sem DMA", 0u, &os_err);

    CPU_SR_ALLOC();
    CPU_CRITICAL_ENTER();

	initBuffer();
	DMA_Init();
	RNG_Init();

	/* En K64F + Micrium, la prioridad 0 bloquea al Kernel. Usar 5 es seguro. */
	NVIC_SetPriority(DMA0_IRQn, 5);
	NVIC_SetPriority(DMA1_IRQn, 5);
	NVIC_SetPriority(DMA2_IRQn, 5);

	NVIC_SetPriority(FTM0_IRQn, 5);
	NVIC_SetPriority(FTM1_IRQn, 5);
	NVIC_SetPriority(FTM2_IRQn, 5);

	CPU_CRITICAL_EXIT();

#if OS_CFG_STAT_TASK_EN > 0u
    /* (optional) Compute CPU capacity with no task running */
    OSStatTaskCPUUsageInit(&os_err);
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN
    CPU_IntDisMeasMaxCurReset();
#endif


    /* Create Task2 */
    OSTaskCreate(&Task2TCB, 			//tcb
                 "Task 2",				//name
                  Task2,				//func
                  0u,					//arg
                  TASK2_PRIO,			//prio
                 &Task2Stk[0u],			//stack
                  TASK2_STK_SIZE_LIMIT,	//stack limit
                  TASK2_STK_SIZE,		//stack size
                  0u,
                  0u,
                  0u,
                 (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 &os_err);

    /* Create Task3 */
        OSTaskCreate(&Task3TCB, 			//tcb
                     "Task 3",				//name
                      Task3,				//func
                      0u,					//arg
                      TASK3_PRIO,			//prio
                     &Task3Stk[0u],			//stack
                      TASK3_STK_SIZE_LIMIT,	//stack limit
                      TASK3_STK_SIZE,		//stack size
                      0u,
                      0u,
                      0u,
                     (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                     &os_err);

    while (1) {
        OSTimeDlyHMSM(0u, 0u, 1u, 0u, OS_OPT_TIME_HMSM_STRICT, &os_err);
    }
}

int main(void) {
    OS_ERR err;

#if (CPU_CFG_NAME_EN == DEF_ENABLED)
    CPU_ERR  cpu_err;
#endif

    hw_Init();

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

    OSQCreate(	(OS_Q *)&AppQ,
    			(CPU_CHAR *)"My App Queue",
				(OS_MSG_QTY)1,
				(OS_ERR *)&err);

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
