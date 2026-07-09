/*
 * display.c
 *
 *  Created on: 1 sep. 2025
 *      Author: andrea
 */
#include "display.h"
#include "timer.h"
#include "board.h"
static double tiempo_on;


static int nivel_de_brillo=BRILLO_MUY_ALTO;


static uint8_t led_informante;
static uint8_t palabra_auxiliar [LONGITUD_PALABRA + CANTIDAD_DISPLAYS + 10];
tim_id_t iddispl, id_despl, id_disp_off;
void clear_display(void);
static bool flag_desplazamiento=true;
uint8_t transformar_a_binario (char caracter);
int traductor_palabra_a_trama (char palabra [], int que_caracteres);
void prender_display(void);
//void mostrar_en_display(uint8_t caracter_auxiliar, int display_activo); //caracter auxiliar, es el caractera mostrar de la pal aux
void reiniciar_timer (void);
static int desplazamiento = 0;


#ifdef MODE_GPIO

static const int pines[] = PINES_DISP;
static const int pines_decoder[] = PINES_DECO;

#endif

#ifdef MODE_SHIFT
static const int pines[]=PINES_SHIFT;
//bool shift_register_veri = false;
#endif

uint8_t buffer_caracteres[] = DISPLAY_TABLE;

bool latch = false; // Habría que ver una mejor implementación


static int display_activo; //CAMBIAR A UINT8_T
static int longitud_real_palabra; //longitud pensabamos que era long-1 pero no. OJO
//static int que_caracter=0;

void clear_display()
{
	for(int i=0; i<LONGITUD_PALABRA + CANTIDAD_DISPLAYS ; i++)
		palabra_auxiliar[i]=0;
}
void iniciar_display(void)
{
	brillantina_de_display(nivel_de_brillo);
	clear_display();
	id_disp_off=timerStart(tiempo_on, TIM_MODE_PERIODIC_CRITICAL, prender_display);
	id_despl= timerStart(500, TIM_MODE_PERIODIC_CRITICAL, callback_desplazamiento);

	prender_ledcitos(TURN_OFF_ALL_LEDS);
}

void prender_display(void)
{
	timerStop(id_disp_off);
	callback_actualizar_display();
	iddispl= timerStart(5-tiempo_on, TIM_MODE_PERIODIC_CRITICAL, reiniciar_timer);



//	mostrar_en_display(0, display_activo);
//
//    // acá apagás el display (ej: poner todos los segmentos en 0)
//	iddispl= timerStart(5, TIM_MODE_PERIODIC_CRITICAL, callback_actualizar_display);
//	timerExpired(iddispl);
//	prender_ledcitos(PRENDER_ALL);
//	id_disp_off=timerStart(2, TIM_MODE_PERIODIC_CRITICAL, apagar_display);

}

void reiniciar_timer (void){
	id_disp_off=timerStart(tiempo_on, TIM_MODE_PERIODIC_CRITICAL, prender_display);
	mostrar_en_display(0, display_activo);
	timerStop(iddispl);
}

void brillantina_de_display(int nivel)
{

	tiempo_on=nivel;
    switch (nivel) {
    case BRILLO_MUY_BAJO: tiempo_on = 4.5; break;
    case BRILLO_BAJO:     tiempo_on = 4; break;
    case BRILLO_REGULAR:  tiempo_on = 2.5; break;
    case BRILLO_ALTO:     tiempo_on = 1.5; break;
    case BRILLO_MUY_ALTO: tiempo_on = 0.5; break;
    default:              tiempo_on = 2; break;
    }


}




uint8_t transformar_a_binario (char caracter){

	uint8_t display_caracter;

	if (('0' <= caracter) && (caracter <= '9')){
		display_caracter = caracter - '0';
	}

	else if ((caracter >= 'A') && (caracter <= 'Z')){
		display_caracter = (caracter - 'A') + 10;
	}
	else if (caracter== ' '){
		display_caracter = 37;
	}
	else if (caracter=='-'){
		display_caracter = 36;
	}


	//display_caracter = buffer_caracteres[display_caracter] & MASK_NUM_DISPLAY;
	display_caracter = buffer_caracteres[display_caracter];

	return display_caracter;
} //es interna, no se actualiza en actualizador



void atender_llamada_usuario (char palabra [], bool usuario_desplazar, int nivel_brillito){
    nivel_de_brillo = nivel_brillito;
    brillantina_de_display(nivel_de_brillo);
	desplazamiento = 0;
	flag_desplazamiento=usuario_desplazar;
	if(flag_desplazamiento==true)
	{
		timerRestart(id_despl);
		clear_display();
		static int que_caracteres;
		display_activo = DISPLAY_1;
		#ifdef MODE_SHIFT
			gpioWrite(pines[PIN_CLOCK_SHIFT_REGISTER], false);
			gpioWrite(pines[PIN_LATCH], false);
		#endif
			for (que_caracteres = 0; (palabra[que_caracteres] != '\0') && (que_caracteres < LONGITUD_PALABRA); ) {
					que_caracteres++;
			}//si la palabra es longitud 4 que_caracteres vale 4 (NO 3)

			longitud_real_palabra = traductor_palabra_a_trama(palabra, (que_caracteres)); //porque queremos no contar el 0
		//longitud_real_de_la_palabra es con un -1 pero nos sirve
	}
	else 	{
		clear_display();

		int que_caracteres;

		display_activo = DISPLAY_1;
		#ifdef MODE_SHIFT
			gpioWrite(pines[PIN_CLOCK_SHIFT_REGISTER], false);
			gpioWrite(pines[PIN_LATCH], false);
		#endif
			for (que_caracteres = 0; (palabra[que_caracteres] != '\0') && (que_caracteres < CANTIDAD_DISPLAYS); ) {
					que_caracteres++;
			}//si la palabra es longitud 4 que_caracteres vale 4 (NO 3)

			longitud_real_palabra = traductor_palabra_a_trama(palabra, (que_caracteres)); //porque queremos no contar el 0
		//longitud_real_de_la_palabra es con un -1 pero nos sirve
	}


}

int traductor_palabra_a_trama (char palabra [], int que_caracteres){
	int i;
	for (i = 0; i < que_caracteres; i++){
		palabra_auxiliar[i] = transformar_a_binario(palabra[i]);
	}
	if (flag_desplazamiento==true){
		for (i = que_caracteres; i < (que_caracteres + CANTIDAD_DISPLAYS); i++){
			palabra_auxiliar[i] = SPACE;
		}
	}

//		timerStop(id_despl);

	return i; //la idea es devolver el valor de la cantidad de cosas a mostrar en display
} //funcion interna, no se actualiza con el actualizador

//es como juli
void callback_actualizar_display (){
	static int indice=0;
	indice = display_activo + desplazamiento;
	if(longitud_real_palabra > CANTIDAD_DISPLAYS){
		(indice >= longitud_real_palabra) ? (indice -=longitud_real_palabra): indice;
	}
	else{
		desplazamiento = 0;
	}
	 mostrar_en_display(palabra_auxiliar[indice], display_activo); // le digo que parte imprimir y en q display
	 //brillantina_de_display(nivel_de_brillo);

	 if(display_activo == DISPLAY_4){
		 display_activo = DISPLAY_1;
	}
	 else {
		 display_activo = display_activo+1;
	 }
	 // aca aumento depsues de mandafr a mostrar en el display
}
//acordarse de llamarla lento

void callback_desplazamiento(){
//	gpioMode(PRUEBA, OUTPUT);
//	gpioWrite(PRUEBA, HIGH);
	if (flag_desplazamiento==true){
		if (desplazamiento == longitud_real_palabra){
			desplazamiento = 0;
		}
		else desplazamiento++;
	}
	else {
		desplazamiento=0;
	//		timerStop(id_despl);
	}

}
void prender_ledcitos(int led_informantes)
{
	led_informante= led_informantes;
	callback_actualizar_display ();
}


void mostrar_en_display(uint8_t caracter_auxiliar, int display_activo){ //caracter auxiliar, es el caractera mostrar de la pal aux
	#ifdef MODE_GPIO


	for(int i = 0; i < 8 ; i++){
		gpioMode(pines[i], OUTPUT);
		gpioWrite(pines[i], (caracter_auxiliar >> i )& 0b1);
//		gpioWrite(pines[i], (caracter_auxiliar << i )& 0b1);

	}
	for (int i = 0; i< 2 ; i++){
		gpioMode(pines_decoder[i], OUTPUT);
		gpioWrite(pines_decoder[i], (display_activo >> i) & 0b1);
	}

	#endif

#ifdef MODE_SHIFT

	for(int i = 0; i <= PIN_LATCH ; i++){
			gpioMode(pines[i], OUTPUT);
	}
	uint16_t trama_completa_shift_reg=0;
	trama_completa_shift_reg = ((caracter_auxiliar<<4) | (display_activo<<2) | led_informante);
	static int contador_clock = 0;
	for(; contador_clock < BITS_DISPLAY + BITS_DECODER + BITS_LEDS ; contador_clock++){
		gpioWrite(pines[PIN_SHIFT_REGISTER],\
		(bool)(trama_completa_shift_reg & MASK_TO_SHIFT_REGISTER(contador_clock)));
		gpioWrite(pines[PIN_CLOCK_SHIFT_REGISTER], true);
		gpioWrite(pines[PIN_CLOCK_SHIFT_REGISTER], false);
	}
//MANDAR AL REVES
	if (contador_clock == BITS_DISPLAY + BITS_DECODER + BITS_LEDS){
			gpioWrite(pines[PIN_LATCH], true);
			gpioWrite(pines[PIN_LATCH], false);
			contador_clock = 0;
	} //se hace solo
#endif
}
