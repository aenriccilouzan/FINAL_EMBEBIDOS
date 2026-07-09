/*
 * tarjeta_magnetica.h
 *
 *  Created on: 31 ago. 2025
 *      Author: andrea
 */

#ifndef TARJETA_MAGNETICA_H_
#define TARJETA_MAGNETICA_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define PAR 1
#define IMPAR 0
#define SIZE_PAN 16
#define SIZE_AD 7
#define SIZE_DD 13
#define CANTIDAD_DATOS_TARJETA 4
#define NULL ((void *) 0)

void guardar_dato_serie(void);
/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

enum {SS, PAN, AD, DD, LRC};


struct dataLM {
    uint8_t PAN[SIZE_PAN];  // Primary Account Number (máx 19 dígitos)
    uint8_t AD[SIZE_AD];    // Additional Data (7 caracteres máx)
    uint8_t DD[SIZE_DD];    // Discretionary Data (8 caracteres máx)
    bool error;       // Indica si hubo error en la lectura
};

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/
/**
 * @brief Inicializa los pines y la interrupción para la lectura de la tarjeta.
 *
 * Configura los GPIO correspondientes (ENABLE, DATA y CLOCK) y asocia
 * la rutina de interrupción para recibir los bits en serie.
 */
void inicializar_tarjeta(void);

/**
 * @brief Retorna los datos leídos de la tarjeta si la lectura fue completada.
 *
 * @return Puntero a la estructura con los datos de la tarjeta, o NULL si
 *         todavía no se terminó de recibir los datos.
 */
struct dataLM* recibiendo_data(void);

void reset_datos_tarjeta(void);















#endif /* TARJETA_MAGNETICA_H_ */
