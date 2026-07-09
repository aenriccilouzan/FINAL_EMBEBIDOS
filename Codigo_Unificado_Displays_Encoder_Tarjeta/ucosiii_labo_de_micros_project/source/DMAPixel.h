/**
 * \file    DMAPixel.h
 * \brief   WS2812B Driver using DMA and FTM for FRDM-K64F (SDK 2.x)
 * \author  Erich Styger (Adapted for SDK 2.x)
 *
 * This module deals with the low level timer and eDMA to
 * generate the bitstream for multiple WS2812B lanes.
 */

#ifndef SOURCES_DMAPIXEL_H_
#define SOURCES_DMAPIXEL_H_

#include <stdint.h>
#include "Color.h"
#include <os.h>

/* Definición de la cantidad de pixeles en la tira */
#define NEO_NOF_PIXEL       64
/* Bits por pixel (3 colores * 8 bits = 24 bits) */
#define NEO_NOF_BITS_PIXEL  24

/* semaphore */
extern OS_SEM DMA_SemEndTransfer;

extern uint8_t g_BrightnessLevel;

extern uint8_t g_OcupacionPiso1;
extern uint8_t g_OcupacionPiso2;
extern uint8_t g_OcupacionPiso3;

/*******************************************************************************
 * API FUNCTIONS
 ******************************************************************************/

/* * Procesa el mensaje de incremento de ocupación.
 * floor_char: 'A' (Piso 1), 'B' (Piso 2), 'C' (Piso 3)
 */
void IncrementOccupancy(char floor_char);

/*
 * Dibuja el estado actual de la ocupación en la matriz
 * basándose en las variables globales.
 */
void DrawOccupancy(void);

/*!
 * @brief Realiza la transferencia de datos a los LEDs vía DMA.
 * Esta función inicia la secuencia de pulsos enviando el buffer al pin GPIO.
 *
 * @param transmitBuf Puntero al buffer de datos crudos (bits) a enviar.
 * @param nofBytes    Tamaño del buffer en bytes.
 */
void DMA_Transfer(uint8_t *transmitBuf, uint32_t nofBytes);

//funcion que se llama despues ossempend
void DMA_Transfer_finished();

/*!
 * @brief Inicializa todo el hardware necesario: Relojes, GPIO, DMAMUX, EDMA y FlexTimer.
 * Debe llamarse una vez al inicio del programa.
 */
void DMA_Init(void);

/*!
 * @brief Inicializa el Generador de Números Aleatorios (RNG) del hardware.
 * Necesario para los efectos de luces aleatorios.
 */
void RNG_Init(void);

/*!
 * @brief Genera un número aleatorio utilizando el hardware RNG del K64F.
 *
 * @param limit El límite superior del número aleatorio (retorna entre 0 y limit).
 * @return uint32_t Número aleatorio generado.
 */
uint32_t getRandom(uint8_t limit);

/*!
 * @brief Limpia el buffer de transmisión (pone todos los bits en 0/apagado).
 * No actualiza los LEDs, solo la memoria interna.
 */
void initBuffer(void);

/*!
 * @brief Ejecuta una secuencia de demostración con colores aleatorios parpadeando.
 *
 * @param count Cantidad de ciclos de parpadeo a ejecutar.
 */
void randomColors(uint8_t count);

/*!
 * @brief Efecto de "Centelleo" (Twinkle). Enciende pixeles aleatorios.
 *
 * @param c           Color del centelleo.
 * @param Count       Cantidad de pixeles a encender.
 * @param SpeedDelay  Retardo en milisegundos entre encendidos.
 * @param OnlyOne     Si es 1, borra el pixel anterior antes de encender el nuevo.
 * @param reset       Si es 1, apaga todo antes de empezar.
 */
void Twinkle(Color c, uint32_t Count, uint32_t SpeedDelay, uint8_t OnlyOne, uint8_t reset); 

/*!
 * @brief Efecto de "Barrido" (Color Wipe). Llena la tira pixel por pixel progresivamente.
 *
 * @param c           Color de relleno.
 * @param SpeedDelay  Velocidad del barrido (ms entre pixeles).
 */
void colorWipe(Color c, uint32_t SpeedDelay);

/*!
 * @brief Efecto de desvanecimiento (Fade In / Fade Out).
 * Sube y baja el brillo de un color base.
 *
 * @param c Color base para el efecto.
 */
void FadeInOut(Color c);

/*!
 * @brief Efecto de marquesina de teatro (luces que persiguen).
 *
 * @param c           Color de las luces.
 * @param SpeedDelay  Velocidad del movimiento (ms).
 */
void theaterChase(Color c, uint32_t SpeedDelay);

/*!
 * @brief Establece todos los pixeles del buffer a un mismo color.
 * Nota: Requiere llamar a DMA_Transfer para ver el cambio.
 *
 * @param c Color a establecer.
 */
void setAll(Color c);

/*!
 * @brief Establece un pixel específico en el buffer a un color dado.
 * Convierte el color RGB a la secuencia de bits necesaria para el WS2812B.
 *
 * @param Pixel Índice del pixel (0 a NEO_NOF_PIXEL-1).
 * @param c     Color a establecer.
 */
void setPixel(uint8_t Pixel, Color c);

/*!
 * @brief Función de retardo bloqueante simple (busy wait).
 * Usa ciclos de ensamblador (nop) para perder tiempo.
 *
 * @param ms Cantidad de milisegundos a esperar.
 */
void delay(uint32_t ms);

#endif /* SOURCES_DMAPIXEL_H_ */
