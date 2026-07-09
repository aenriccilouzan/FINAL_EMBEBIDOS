/**
 * \file
 * \brief WS2812B Driver using DMA.
 * \author Erich Styger
 *
 * This module deals with the low level timer and eDMA to
 * generate the bitstream for multiple WS2812B lanes.
 * \license
 *  This is a free software and is opened for education,  research and commercial developments under license policy of following terms:
 *  This is a free software and there is NO WARRANTY.
 *  No restriction on use. You can use, modify and redistribute it for personal, non-profit or commercial product UNDER YOUR RESPONSIBILITY.
 *  Redistributions of source code must retain the above copyright notice.
 */

#include "DMAPixel.h"
#include "MK64F12.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>


/* LIBRERIAS NUEVAS SDK 2.x */
#include "fsl_common.h"
#include "fsl_port.h"
#include "fsl_gpio.h"
#include "fsl_ftm.h"
#include "fsl_edma.h"
#include "fsl_dmamux.h" // Antes no se incluía explícitamente, ahora es OBLIGATORIO
#include "fsl_clock.h"

/* semaphore */
OS_SEM DMA_SemEndTransfer;

/* FTM related */
/* Usaremos FTM0 */
#define BOARD_FTM_BASEADDR FTM0

/* Recalculado para Bus Clock de 50 MHz (1 tick = 20ns) */

/* Periodo total: 1.25us / 20ns = 62.5 ticks -> Redondeamos a 63 */
#define FTM_PERIOD_TICKS  (63U)

/* Delay inicial (start bit setup): Un valor pequeño seguro */
#define FTM_CH0_TICKS     (10U)

/* T0H (Cero lógico): 0.4us / 20ns = 20 ticks */
#define FTM_CH1_TICKS     (30U)  /* at 0.4us write data (cambio de 1 a 0 o 1 a 1) */

/* T1H (Uno lógico): 0.8us / 20ns = 40 ticks */
#define FTM_CH2_TICKS     (50U)  /* at 0.8us clear bits (fin del pulso alto del 1) */

/* DMA related */
#define DMA_CH0_BIT_SET     0
#define DMA_CH1_BYTE_WRITE  1
#define DMA_CH2_BIT_CLEAR   2

/* En SDK 2.x necesitamos "Handles" para el DMA */
edma_handle_t g_EDMA_Handle_0; // Para el canal 0
edma_handle_t g_EDMA_Handle_1; // Para el canal 1
edma_handle_t g_EDMA_Handle_2; // Para el canal 2

static volatile bool dmaDone = false;

uint8_t g_BrightnessLevel = 5;

/* Variables globales de ocupación (0 a 4 personas máx) */
uint8_t g_OcupacionPiso1 = 0;
uint8_t g_OcupacionPiso2 = 0;
uint8_t g_OcupacionPiso3 = 0;

/* COLORES DEFINIDOS PARA CADA PISO (Puedes cambiarlos) */
Color ColorPiso1 = {0, 0, 255};   // Azul
Color ColorPiso2 = {0, 255, 0};   // Verde
Color ColorPiso3 = {255, 0, 0};   // Rojo
Color ColorVacio = {0, 0, 0};     // Negro/Apagado

/* Definiciones de GPIO para PTD0 (FRDM-K64F) */
/* Bit 0 del Puerto D */
#define LED_PIN_BIT 0U
static volatile uint8_t OneValue = 0x01; /* Valor constante para el DMA */

/* Callback que se llama cuando el DMA termina */
void EDMA_Callback(edma_handle_t *handle, void *userData, bool transferDone, uint32_t tcds)
{
    //if (transferDone)
    {
    	OS_ERR os_err;
    	// Liberar el semáforo para despertar a la tarea
    	OSSemPost(&DMA_SemEndTransfer, OS_OPT_POST_1, &os_err);
    }
}

static uint8_t transmitBuf[NEO_NOF_PIXEL*NEO_NOF_BITS_PIXEL] ={};

const uint8_t gamma8[] = {
    0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
    1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,  2,  2,  3,  3,  3,
    3,  3,  3,  4,  4,  4,  4,  4,  4,  5,  5,  5,  5,  6,  6,  6,
    6,  6,  7,  7,  7,  8,  8,  8,  8,  9,  9,  9, 10, 10, 10, 11,
   11, 12, 12, 12, 13, 13, 14, 14, 14, 15, 15, 16, 16, 17, 17, 18,
   18, 19, 19, 20, 21, 21, 22, 22, 23, 23, 24, 25, 25, 26, 27, 27,
   28, 29, 30, 30, 31, 32, 33, 33, 34, 35, 36, 37, 37, 38, 39, 40,
   41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56,
   57, 59, 60, 61, 62, 63, 65, 66, 67, 68, 70, 71, 72, 74, 75, 76,
   78, 79, 81, 82, 84, 85, 87, 88, 90, 91, 93, 95, 96, 98, 99,101,
  103,105,106,108,110,112,113,115,117,119,121,123,125,127,129,131,
  133,135,137,139,141,143,146,148,150,152,154,157,159,161,164,166,
  168,171,173,176,178,181,183,186,188,191,194,196,199,202,204,207,
  210,213,216,219,221,224,227,230,233,236,239,242,246,249,252,255
 };

/* --------------------------------------- */
/* FTM                                     */
/* --------------------------------------- */
/* --------------------------------------- */
/* FTM Helpers (Actualizados SDK 2.x)      */
/* --------------------------------------- */
static void StartStopFTM(uint32_t instance, bool startIt) {
    /* Nota: instance se ignora porque usamos FTM0 directo,
       pero lo dejamos para compatibilidad con tu lógica anterior */
    if (startIt) {
        FTM_StartTimer(FTM0, kFTM_SystemClock);
    } else {
        FTM_StopTimer(FTM0);
    }
}

static void ResetFTM(uint32_t instance) {
    /* Detenemos el timer para limpiar */
    FTM_StopTimer(FTM0);
    FTM0->CNT = 0; /* Reset del contador a 0 */

    /* Limpiamos flags de estado por si acaso */
    FTM_ClearStatusFlags(FTM0, kFTM_TimeOverflowFlag);

    /* CONFIGURACIÓN CRÍTICA: */
    /* MSA (Bit 6) = 1 -> Modo Edge Alined PWM (necesario para que ocurra el evento) */
    /* DMA (Bit 0) = 1 -> Habilitar DMA en el evento */
    uint32_t config = FTM_CnSC_MSB_MASK | FTM_CnSC_DMA_MASK | FTM_CnSC_ELSB_MASK | FTM_CnSC_CHIE_MASK;

    FTM0->CONTROLS[0].CnSC = config;
    FTM0->CONTROLS[1].CnSC = config;
    FTM0->CONTROLS[2].CnSC = config;
}

static void StopFTMDMA(uint32_t instance) {
    StartStopFTM(instance, false);
    FTM0->CNT = 0;
    FTM0->CONTROLS[0].CnSC |= FTM_CnSC_DMA_MASK;
    FTM0->CONTROLS[1].CnSC |= FTM_CnSC_DMA_MASK;
    FTM0->CONTROLS[2].CnSC |= FTM_CnSC_DMA_MASK;
}

static void InitFlexTimer(uint32_t instance) {
    ftm_config_t ftmInfo;

    /* Configuración por defecto */
    FTM_GetDefaultConfig(&ftmInfo);
    FTM_Init(FTM0, &ftmInfo);

    /* Configuramos el Periodo */
    FTM_SetTimerPeriod(FTM0, FTM_PERIOD_TICKS);

    /* CONFIGURACIÓN CRÍTICA: */
    /* MSA (Bit 6) = 1 -> Modo Edge Alined PWM (necesario para que ocurra el evento) */
    /* DMA (Bit 0) = 1 -> Habilitar DMA en el evento */
    uint32_t config = FTM_CnSC_MSB_MASK | FTM_CnSC_DMA_MASK | FTM_CnSC_ELSB_MASK | FTM_CnSC_CHIE_MASK;


    FTM0->CONTROLS[0].CnSC = config;
    FTM0->CONTROLS[1].CnSC = config;
    FTM0->CONTROLS[2].CnSC = config;

    /* Valores de comparación */
    FTM0->CONTROLS[0].CnV = FTM_CH0_TICKS;
    FTM0->CONTROLS[1].CnV = FTM_CH1_TICKS;
    FTM0->CONTROLS[2].CnV = FTM_CH2_TICKS;

}


/* ========================================== */
/* TRANSFERENCIA DE DATOS (SDK 2.x)           */
/* ========================================== */

void DMA2_IRQHandler(void)
{
    CPU_SR_ALLOC();

    // 1. Notificar a Micrium que entramos a una ISR
    CPU_CRITICAL_ENTER();
    OSIntEnter();
    CPU_CRITICAL_EXIT();

    // 2. Ejecutar la lógica del SDK (que llamará a tu Callback)
    EDMA_HandleIRQ(&g_EDMA_Handle_2);

    // 3. Notificar a Micrium que salimos (y permitir cambio de contexto si es necesario)
    OSIntExit();
}

void DMA_Transfer(uint8_t *transmitBuf, uint32_t nofBytes) {
    edma_transfer_config_t transferConfig;

    /* Aseguramos estado inicial */
    dmaDone = false;
    ResetFTM(0);

    /* --- CONFIGURACIÓN CANAL 0: Poner Pin en 1 (SET) --- */
    /* Fuente: Variable constante 1. Destino: Registro PSOR (Set Output) */
    EDMA_PrepareTransfer(&transferConfig,
                         (void *)&OneValue,             /* Source: 1 */
                         sizeof(uint8_t),               /* Source width: 8 bits */
                         (void *)&GPIOD->PSOR,          /* Dest: Port Set Register */
                         sizeof(uint8_t),               /* Dest width: 8 bits */
                         sizeof(uint8_t),               /* Bytes per request: 1 */
                         nofBytes,                      /* Total bytes (Major loop) */
                         kEDMA_MemoryToPeripheral);
    /* Quitamos el incremento de fuente/destino para que siempre escriba en el mismo registro */
    transferConfig.srcOffset = 0;
    transferConfig.destOffset = 0;
    EDMA_SetTransferConfig(DMA0, 0, &transferConfig, NULL);


    /* --- CONFIGURACIÓN CANAL 1: Escribir Datos (DATA) --- */
    /* Fuente: Buffer de colores. Destino: Registro PDOR (Data Output) */
    /* Nota: Usamos PDOR para escribir el 0 o 1 específico del bit de color */
    /* Truco: Escribimos al byte 0 del puerto D. Si tus LEDs no están en PTD0-PTD7, esto requiere ajuste */
    /* Como PTD0 es el LSB, escribir un byte afecta al pin 0. */
    EDMA_PrepareTransfer(&transferConfig,
                         transmitBuf,                   /* Source: Buffer de datos */
                         sizeof(uint8_t),
                         (void *)&GPIOD->PDOR,          /* Dest: Port Data Output */
                         sizeof(uint8_t),
                         sizeof(uint8_t),
                         nofBytes,
                         kEDMA_MemoryToPeripheral);

    transferConfig.srcOffset = 1; /* ¡Importante! Avanzar en el buffer de origen */
    transferConfig.destOffset = 0; /* No avanzar en el registro de destino */
    EDMA_SetTransferConfig(DMA0, 1, &transferConfig, NULL);


    /* --- CONFIGURACIÓN CANAL 2: Poner Pin en 0 (CLEAR) --- */
    /* Fuente: Variable constante 1. Destino: Registro PCOR (Clear Output) */
    EDMA_PrepareTransfer(&transferConfig,
                         (void *)&OneValue,
                         sizeof(uint8_t),
                         (void *)&GPIOD->PCOR,          /* Dest: Port Clear Register */
                         sizeof(uint8_t),
                         sizeof(uint8_t),
                         nofBytes,
                         kEDMA_MemoryToPeripheral);
    transferConfig.srcOffset = 0;
    transferConfig.destOffset = 0;
    EDMA_SetTransferConfig(DMA0, 2, &transferConfig, NULL);

    /* Habilitar interrupción al finalizar el canal 2 para saber cuando parar */
    EDMA_EnableChannelInterrupts(DMA0, 2, kEDMA_MajorInterruptEnable);

    /* --- INICIAR --- */
    /* Habilitamos las peticiones DMA */
    EDMA_StartTransfer(&g_EDMA_Handle_0);
    EDMA_StartTransfer(&g_EDMA_Handle_1);
    EDMA_StartTransfer(&g_EDMA_Handle_2);

    /* Arrancamos el Timer que actúa como gatillo */
    StartStopFTM(0, true);

}

void DMA_Transfer_finished()
{
    /* --- LIMPIEZA --- */
    StopFTMDMA(0);

    /* Limpiar flags de error si los hubiera */
    EDMA_ClearChannelStatusFlags(DMA0, 0, kEDMA_InterruptFlag);
    EDMA_ClearChannelStatusFlags(DMA0, 1, kEDMA_InterruptFlag);
    EDMA_ClearChannelStatusFlags(DMA0, 2, kEDMA_InterruptFlag);
}

static void InitHardware(void) {
    /* 1. Habilitar relojes de Puertos */
    CLOCK_EnableClock(kCLOCK_PortA);
    CLOCK_EnableClock(kCLOCK_PortB);
    CLOCK_EnableClock(kCLOCK_PortC);
    CLOCK_EnableClock(kCLOCK_PortD);
    CLOCK_EnableClock(kCLOCK_PortE);

    /* 2. Habilitar reloj para DMAMUX (¡Muy importante en SDK 2.x!) */
    CLOCK_EnableClock(kCLOCK_Dmamux0);
    /* 3. Habilitar reloj para DMA */
    CLOCK_EnableClock(kCLOCK_Dma0);

    /* 4. Configurar PTD0 como GPIO Salida (Data In de los LEDs) */
    port_pin_config_t pinConfig = {
        kPORT_PullDisable,
        kPORT_FastSlewRate,
        kPORT_PassiveFilterDisable,
        kPORT_OpenDrainDisable,
        kPORT_LowDriveStrength,
        kPORT_MuxAsGpio,
        0
    };
    PORT_SetPinConfig(PORTD, 0U, &pinConfig);

    gpio_pin_config_t gpioOutput = {
        kGPIO_DigitalOutput,
        0 // Valor inicial bajo
    };
    GPIO_PinInit(GPIOD, 0U, &gpioOutput);

    /* 5. Configurar Pines de FTM0 (Puerto C) para activar DMA */
    /* PTC1 -> FTM0_CH0 */
    PORT_SetPinMux(PORTC, 1U, kPORT_MuxAlt4);
    /* PTC2 -> FTM0_CH1 */
    PORT_SetPinMux(PORTC, 2U, kPORT_MuxAlt4);
    /* PTC3 -> FTM0_CH2 */
    PORT_SetPinMux(PORTC, 3U, kPORT_MuxAlt4);
}

/* ========================================== */
/* INICIALIZACIÓN (SDK 2.x)                   */
/* ========================================== */

void DMA_Init(void) {
    /* 1. Inicializar Hardware (Relojes y Pines) */
    InitHardware();

    /* 2. Inicializar DMAMUX (El ruteador de señales) */
    DMAMUX_Init(DMAMUX0);

    /* --- Configurar Canal 0 (SET) disparado por FTM0_CH0 --- */
    DMAMUX_SetSource(DMAMUX0, 0, kDmaRequestMux0FTM0Channel0);
    DMAMUX_EnableChannel(DMAMUX0, 0);

    /* --- Configurar Canal 1 (DATA) disparado por FTM0_CH1 --- */
    DMAMUX_SetSource(DMAMUX0, 1, kDmaRequestMux0FTM0Channel1);
    DMAMUX_EnableChannel(DMAMUX0, 1);

    /* --- Configurar Canal 2 (CLEAR) disparado por FTM0_CH2 --- */
    DMAMUX_SetSource(DMAMUX0, 2, kDmaRequestMux0FTM0Channel2);
    DMAMUX_EnableChannel(DMAMUX0, 2);

    /* 3. Inicializar EDMA (El motor de transferencia) */
    edma_config_t userConfig;
    EDMA_GetDefaultConfig(&userConfig);

    userConfig.enableRoundRobinArbitration = true;

    EDMA_Init(DMA0, &userConfig);

    /* 4. Crear los Handles (Vínculo software-hardware) */
    EDMA_CreateHandle(&g_EDMA_Handle_0, DMA0, 0);
    EDMA_CreateHandle(&g_EDMA_Handle_1, DMA0, 1);
    EDMA_CreateHandle(&g_EDMA_Handle_2, DMA0, 2);

    /* 5. Instalar Callback solo en el último canal (el que limpia) para saber cuando termina */
    EDMA_SetCallback(&g_EDMA_Handle_2, EDMA_Callback, NULL);

    NVIC_EnableIRQ(DMA0_IRQn);
    NVIC_EnableIRQ(DMA1_IRQn);
    NVIC_EnableIRQ(DMA2_IRQn);

    NVIC_EnableIRQ(FTM0_IRQn);
    NVIC_EnableIRQ(FTM1_IRQn);
    NVIC_EnableIRQ(FTM2_IRQn);

    /* 6. Preparar el Timer (pero no arrancarlo aún) */
    InitFlexTimer(0); // Configura periodo y valores de comparación
    ResetFTM(0);
}

/* * Función auxiliar para mapear coordenadas X,Y a índice lineal 0-63
 * Asume matriz colocada con el pixel 0 abajo-izquierda y conexión ZigZag vertical.
 * x: Columna (0-7)
 * y: Fila (0-7, donde 0 es abajo)
 */
void setPixelXY(uint8_t x, uint8_t y, Color c) {
    if (x > 7 || y > 7) return;

    uint8_t index;

    // Si la matriz es ZigZag Vertical (las columnas pares suben, impares bajan)
    // Verifica si tu matriz es así. Si es Horizontal, la fórmula cambia.
    // Asumimos layout estándar de matriz flexible:
    // Col 0: 0->7 (sube), Col 1: 15->8 (baja), Col 2: 16->23 (sube)...


    index = (x * 8) + y;


    setPixel(index, c);
}

void IncrementOccupancy(char floor_char) {
    switch(floor_char) {
        case 'A': // Piso 1
            if (g_OcupacionPiso1 < 4) g_OcupacionPiso1++;
            else g_OcupacionPiso1 = 0; // Opcional: Resetear o topear en 4? Asumo reset para demo cíclico
            break;
        case 'B': // Piso 2
            if (g_OcupacionPiso2 < 4) g_OcupacionPiso2++;
            else g_OcupacionPiso2 = 0;
            break;
        case 'C': // Piso 3
            if (g_OcupacionPiso3 < 4) g_OcupacionPiso3++;
            else g_OcupacionPiso3 = 0;
            break;
        default:
            break;
    }
    // Después de actualizar el dato, redibujamos
    DrawOccupancy();
}

void DrawOccupancy(void) {
    // 1. Limpiamos la matriz primero (o solo las columnas de interés)
    setAll(ColorVacio);

    // --- DIBUJAR PISO 1 (Columnas 0 y 1) ---
    // Cada persona = 2 filas.
    for (int i = 0; i < g_OcupacionPiso1; i++) {
        // Persona i ocupa filas (i*2) y (i*2 + 1)
        uint8_t fila_base = i * 2;

        // Columna 0
        setPixelXY(0, fila_base, ColorPiso1);
        setPixelXY(0, fila_base + 1, ColorPiso1);

        // Columna 1
        setPixelXY(1, fila_base, ColorPiso1);
        setPixelXY(1, fila_base + 1, ColorPiso1);
    }

    // --- DIBUJAR PISO 2 (Columnas 2 y 3) ---
    for (int i = 0; i < g_OcupacionPiso2; i++) {
        uint8_t fila_base = i * 2;
        setPixelXY(2, fila_base, ColorPiso2);
        setPixelXY(2, fila_base + 1, ColorPiso2);
        setPixelXY(3, fila_base, ColorPiso2);
        setPixelXY(3, fila_base + 1, ColorPiso2);
    }

    // --- DIBUJAR PISO 3 (Columnas 4 y 5) ---
    for (int i = 0; i < g_OcupacionPiso3; i++) {
        uint8_t fila_base = i * 2;
        setPixelXY(4, fila_base, ColorPiso3);
        setPixelXY(4, fila_base + 1, ColorPiso3);
        setPixelXY(5, fila_base, ColorPiso3);
        setPixelXY(5, fila_base + 1, ColorPiso3);
    }

    // --- Columnas 6 y 7 Reservadas (Vacías) ---

    // 2. ENVIAR AL HARDWARE
    OS_ERR os_err;
    DMA_Transfer(transmitBuf, sizeof(transmitBuf));
    OSSemPend(&DMA_SemEndTransfer, 0, OS_OPT_PEND_BLOCKING, NULL, &os_err);
    DMA_Transfer_finished();
}

/* --------------------------------------- */
/* RNG                                     */
/* --------------------------------------- */
void RNG_Init(void){
	SIM->SCGC6 |= SIM_SCGC6_RNGA_MASK;
	RNG->CR &= ~RNG_CR_SLP_MASK;
	RNG->CR |= RNG_CR_HA_MASK;
	RNG->CR |= RNG_CR_GO_MASK;
}  

/* get a random number between 0 and limit
 * Used to get a random pixel for generating random LED patterns
*/
uint32_t getRandom(uint8_t limit){
	while((RNG->SR & RNG_SR_OREG_LVL(0xF)) == 0) {}
	return (RNG->OR) % limit;
}

void initBuffer(void){
	for(uint32_t i=0;i<NEO_NOF_PIXEL*NEO_NOF_BITS_PIXEL;i++){
		transmitBuf[i] = 0;
	}	
}

void setPixel(uint8_t Pixel, Color c) {
    /* Lógica de Brillo:
       Si el nivel es 5, usamos el color original.
       Si es menor, escalamos proporcionalmente.
       Niveles: 1=20%, 2=40%, 3=60%, 4=80%, 5=100%
    */
    if (g_BrightnessLevel < 5 && g_BrightnessLevel > 0) {
        // Usamos uint16_t para evitar desbordamiento antes de la división
        c.r = (uint8_t)( ((uint16_t)c.r * g_BrightnessLevel) / 5 );
        c.g = (uint8_t)( ((uint16_t)c.g * g_BrightnessLevel) / 5 );
        c.b = (uint8_t)( ((uint16_t)c.b * g_BrightnessLevel) / 5 );
    	//c.r=20u;
    	//c.g=0;
    	//c.b=0;
    }
    /* Si g_BrightnessLevel es 0 o >5, dejamos el color original o podrías decidir apagarlo */

    RGB cc = packColor(c);
    for(uint32_t j=(Pixel*NEO_NOF_BITS_PIXEL);j<(Pixel*NEO_NOF_BITS_PIXEL)+NEO_NOF_BITS_PIXEL;j++){
        transmitBuf[j] = cc & 0x1;
        cc=cc>>1;
    }
}

void setAll(Color c) {
  for(uint8_t i=0; i<NEO_NOF_PIXEL; i++ ) {
    setPixel(i, c); 
  }
}

void theaterChase(Color c, uint32_t SpeedDelay) {
  for (uint8_t j=0; j<10; j++) {  //do 10 cycles of chasing
    for (uint8_t q=0; q < 4; q++) {
      for (uint8_t i=0; i < NEO_NOF_PIXEL; i=i+4) {
        setPixel(i+q, c);    //turn every third pixel on
        //setPixel(i+q-1, red, green, blue);
      }
      OS_ERR os_err;
      DMA_Transfer(transmitBuf, sizeof(transmitBuf));
      OSSemPend(&DMA_SemEndTransfer, 0, OS_OPT_PEND_BLOCKING, NULL, &os_err);
      DMA_Transfer_finished();
      delay(SpeedDelay);
      for (uint8_t i=0; i < NEO_NOF_PIXEL; i=i+4) {
        setPixel(i+q, Black);        //turn every third pixel off
        //setPixel(i+q-1, 0,0,0);
      }
    }
  }
}

void FadeInOut(Color c){
  float r, g, b;
  Color cc;
      
  for(int k = 0; k < 256; k=k+1) { 
    r = (k/256.0)*c.r;
    g = (k/256.0)*c.g;
    b = (k/256.0)*c.b;
   	cc.r=r; cc.g=g; cc.b=b; 
    setAll(cc);
    OS_ERR os_err;
    DMA_Transfer(transmitBuf, sizeof(transmitBuf));
    OSSemPend(&DMA_SemEndTransfer, 0, OS_OPT_PEND_BLOCKING, NULL, &os_err);
    DMA_Transfer_finished();
  }
     
  for(int k = 255; k >= 0; k=k-2) {
    r = (k/256.0)*c.r;
    g = (k/256.0)*c.g;
    b = (k/256.0)*c.b;
    c.r=r; c.g=g; c.b=b; 
    setAll(c);
    OS_ERR os_err;
    DMA_Transfer(transmitBuf, sizeof(transmitBuf));
    OSSemPend(&DMA_SemEndTransfer, 0, OS_OPT_PEND_BLOCKING, NULL, &os_err);
    DMA_Transfer_finished();
  }
}

void colorWipe(Color c, uint32_t SpeedDelay) {
  for(uint8_t i=0; i<NEO_NOF_PIXEL; i++) {
      setPixel(i, c);
      OS_ERR os_err;
      DMA_Transfer(transmitBuf, sizeof(transmitBuf));
      OSSemPend(&DMA_SemEndTransfer, 0, OS_OPT_PEND_BLOCKING, NULL, &os_err);
      DMA_Transfer_finished();
      delay(SpeedDelay);
  }
  
}

void Twinkle(Color c, uint32_t Count, uint32_t SpeedDelay, uint8_t OnlyOne, uint8_t reset) {
  if(reset){
  	setAll(Black);
  }  
  for (int i=0; i<Count; i++) {
     setPixel(getRandom(NEO_NOF_PIXEL),c);
     OS_ERR os_err;
     DMA_Transfer(transmitBuf, sizeof(transmitBuf));
     OSSemPend(&DMA_SemEndTransfer, 0, OS_OPT_PEND_BLOCKING, NULL, &os_err);
     DMA_Transfer_finished();
     delay(SpeedDelay);
     if(OnlyOne) { 
       setAll(Black); 
     }
   }
  delay(SpeedDelay);
}

void randomColors(uint8_t count){
	for(uint8_t i=0;i<count;i++){
		Twinkle(Red,6,400,0,0);
		Twinkle(Green,4,400,0,0);
		Twinkle(Blue,7,400,0,0);
		Twinkle(Black,3,400,0,0);
	}
}

/*!brief Delay routine */
void delay(uint32_t ms)
{
  for(uint32_t i=0; i<1000*ms; i++){
      __asm__("nop");
  }
}
