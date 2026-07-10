/*
 * fsm.c
 *
 *  Created on: 6 sep. 2025
 *      Author: andrea
 */
#include "fsm.h"
extern void esperar (void);

extern void do_nothing(void);
extern void reset_encoder(void);
extern void ir_pin_encoder(void);
extern void pin_invalido(void);
extern void bloquear_id(void);
extern void id_invalido(void);
extern void sleep_reset(void);
extern void ir_a_estado_cliente(void);
extern void ir_a_estado_administrador(void);
extern void id_invalido(void);
extern void inicio_a_id_completo(void);
extern void led_azul_prendido(void);
extern void iniciar_encoder(void);
extern void ingrese_id (void);
extern void puerta_timer(void);
extern void ir_a_cant_dig_pin(void);
extern void ir_a_estado_cliente2(void);
extern void imprimir_pin(void);
extern void borrar_id(int usuario);
extern void reiniciar(void);
extern void ir_a_nueva_contrasenia(void);
extern void ir_a_brillo(void);
extern void ir_a_id_encoder(void);
extern void ir_a_borrar_usuario(void);
extern void reset_boton(void);


void do_nothing(void)
{

}



STATE inicio [] = {
		{APRETO_BOTON, id_completo, ir_a_id_encoder},
		{VALIDO_ID, pin_completo, ir_pin_encoder},
		{NO_VALIDO_ID, inicio, id_invalido},
		//{PASO_TARJETA, id_completo, inicio_a_id_completo}, //acen donothing habrIa que imprimir el id completo tarjeta.
	  	{FIN_TABLA, inicio, do_nothing}
};

STATE id_completo []= {
		{VALIDO_ID, pin_completo, ir_pin_encoder},
		{NO_VALIDO_ID, id_completo, id_invalido},
	  	{FIN_TABLA, id_completo , do_nothing}
};
STATE pin_completo []= {
		{ADMINISTRADOR, administrador, ir_a_estado_administrador},
		{CLIENTE, cliente, ir_a_estado_cliente},
		{NO_VALIDO_PIN, pin_completo, pin_invalido},
		{BLOQUEAR, inicio, bloquear_id},
		{FIN_TABLA, pin_completo , do_nothing}
};

STATE administrador[] = {
		{AGREGAR_USUARIO, agregar_id_usuario, ingrese_id}, // vas a inicio.
		{BORRAR_USUARIO, borrar_usuario, ir_a_borrar_usuario}, //supongo que podes poner BLOQUEAR, pin_completo, ir_a_pin_completo
		{ABRIR_PUERTA, inicio, puerta_timer}, //vas a un estado que sea prender el led rojo.
		{AJUSTAR_BRILLO, ajustar_brillo ,ir_a_brillo}, //vas a un estado que sea prender el led rojo.
		{FIN_TABLA, administrador , do_nothing}
};

STATE agregar_id_usuario[] = {
		{APRETO_BOTON, id_encoder_nuevo, reset_encoder},
		{PASO_TARJETA,decision_longitud_pin, inicio_a_id_completo},
		{FIN_TABLA, agregar_id_usuario, do_nothing}
};

STATE id_encoder_nuevo[] = {
		{VALIDO_ID, decision_longitud_pin, ir_a_cant_dig_pin},
		{NO_VALIDO_ID, inicio, id_invalido},
	  	{FIN_TABLA, id_encoder_nuevo , do_nothing}
};


STATE decision_longitud_pin[] = {
		{APRETO_BOTON,agregar_pin_usuario, ir_a_nueva_contrasenia},
		{FIN_TABLA, decision_longitud_pin, do_nothing}
};
STATE agregar_pin_usuario[] = {
		{GUARDAR_PIN, inicio, imprimir_pin},
		{FIN_TABLA, agregar_pin_usuario , do_nothing}
};

STATE borrar_usuario[] = {
		{BORRADO_COMPLETO, inicio, reset_boton},
		{BORRADO_INVALIDO, administrador, do_nothing},
		{FIN_TABLA, borrar_usuario , do_nothing}
};

//STATE abrir_puerta[] = {
//		{ABRIR, inicio, do_nothing},
//		{FIN_TABLA, agregar_usuario , do_nothing}
//};
//
STATE ajustar_brillo[] = {
		{BRILLO_LISTO, inicio, reiniciar},
		{FIN_TABLA, ajustar_brillo , do_nothing}
};

STATE cliente[] = {
		{CAMBIAR_CLAVE, cant_dig_pin, ir_a_cant_dig_pin}, //tenes que ir a inicio.
		{ABRIR_PUERTA, inicio, puerta_timer},//vas a un estado q sea prender led rojo.
		{FIN_TABLA, cliente, do_nothing}
};
STATE cant_dig_pin[] = {
		{APRETO_BOTON,nueva_contrasenia, ir_a_nueva_contrasenia},
		{FIN_TABLA, cant_dig_pin, do_nothing}
};
STATE nueva_contrasenia[] = {
		{LLENO_PIN_ENCODER,cliente, ir_a_estado_cliente2},
		{FIN_TABLA, nueva_contrasenia, do_nothing}
};



/*
STATE prender_led_rojo [] = {
	{,,} //tenes q prender el led y volver al inicio
};
*/

STATE* FSM_GetInitState(void)
{
	return inicio;
}

STATE* fsm(STATE *p_tabla_estado,BYTE evento_actual)
{
	//if (evento_actual==APRETO_BOTON)
	//	evento_actual=APRETO_BOTON;
   	while (p_tabla_estado -> evento != evento_actual		//Recorre las tablas de estado
		   && p_tabla_estado -> evento != FIN_TABLA)
		++p_tabla_estado;
      (*p_tabla_estado -> p_rut_accion) ();          /*rutina de accion corresondiente*/
      p_tabla_estado=p_tabla_estado -> proximo_estado;   /*siguiente estado*/

      return(p_tabla_estado);

}




//FUNCIONES DE TRANSICION








