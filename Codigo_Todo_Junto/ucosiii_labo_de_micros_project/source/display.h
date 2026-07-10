/*
 * display.h
 *
 *  Created on: 25 ago. 2025
 *      Author: andrea
 */
/*
 * diplay.h
 *
 *  Created on: Aug 24, 2025
 *      Author: julie
 */

#ifndef DIPLAY_H_
#define DIPLAY_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include <stdint.h>
#include <stdbool.h>


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define DISPLAY_TABLE { \
    /* 0 */ 0b11111100, \
    /* 1 */ 0b01100000, \
    /* 2 */ 0b11011010, \
    /* 3 */ 0b11110010, \
    /* 4 */ 0b01100110, \
    /* 5 */ 0b10110110, \
    /* 6 */ 0b10111110, \
    /* 7 */ 0b11100000, \
    /* 8 */ 0b11111110, \
    /* 9 */ 0b11100110, \
    /* A */ 0b11101111, \
    /* B */ 0b11111111, \
    /* C */ 0b10011101, \
    /* D */ 0b11111101, \
    /* E */ 0b10011111, \
    /* F */ 0b10001111, \
    /* G */ 0b10111111, \
    /* H */ 0b01101111, \
    /* I */ 0b01100001, \
    /* J */ 0b01110001, \
    /* K */ 0b01101111, \
    /* L */ 0b00011101, \
    /* M */ 0b11101101, \
    /* N */ 0b00000001, \
    /* O */ 0b11111101, \
    /* P */ 0b11001111, \
    /* Q */ 0b00000001, \
    /* R */ 0b11101111, \
    /* S */ 0b10110111, \
    /* T */ 0b11100001, \
    /* U */ 0b01111101, \
    /* V */ 0b01111101, \
    /* W */ 0b00000001, \
    /* X */ 0b00000001, \
    /* Y */ 0b01110111, \
    /* Z */ 0b11011111, \
	/* - */ 0b00000010, \
    /* SPACE */ 0b00000000 \
}

//#define ACCESS_ID "ACCESS ID"
#define PASSWORD "ACCESS CODE"

#define LONGITUD_PALABRA 30
#define CANTIDAD_DISPLAYS 4
#define SPACE 0b00000000
#define CANT_DE_ESPACIOS 4

#define BRILLO_MUY_BAJO 1
#define BRILLO_BAJO 2
#define BRILLO_REGULAR 3
#define BRILLO_ALTO 4
#define BRILLO_MUY_ALTO 5


#define BITS_DISPLAY 8
#define BITS_DECODER 2
#define BITS_LEDS 2

#define DISPLAY_1 0u //este es el display mas a la izq
#define DISPLAY_2 1u
#define DISPLAY_3 2u
#define DISPLAY_4 3u

#define PRENDER_TERCER_LED 3
#define PRENDER_SEGUNDO_LED 2
#define PRENDER_PRIMER_LED 1
#define TURN_OFF_ALL_LEDS 0
/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/
void brillantina_de_display(int brillar);


void callback_actualizar_display (void);
void atender_llamada_usuario(char palabra[], bool usuario_desplazar, int nivel_brillito);
void callback_desplazamiento(void); //deberia ser un flanco pero dsps vemos
void prender_ledcitos(int led_informante);

void mostrar_en_display(uint8_t caracter_auxiliar, int display_activo); //caracter auxiliar, es el caractera mostrar de la pal aux

void iniciar_display(void);


#define MASK(num_display) (uint16_t(((num_display)<<8)))
//#define MASK_NUM_DISPLAY 0b1111110011111111


///#define PIN_CLOCK_SHIFT_REGISTER PORTNUM2PIN(PC,6)
//#define PIN_LATCH PORTNUM2PIN(PC,7)



#define MASK_TO_SHIFT_REGISTER(a) ((1u << (a)))


#endif /* DIPLAY_H_ */
