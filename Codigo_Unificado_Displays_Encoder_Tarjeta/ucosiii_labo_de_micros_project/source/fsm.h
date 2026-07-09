/*
 * fsm.h
 *
 *  Created on: 6 sep. 2025
 *      Author: andrea
 */

#ifndef FSM_H_
#define FSM_H_

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/
enum {PULSADOR_CAMBIO, GIROENCODER_CAMBIO, LECTOR_TARJETA_CAMBIO};


typedef	unsigned char BYTE;
typedef struct state_diagram_edge STATE;

enum {VACIO, APRETO_BOTON, PASO_TARJETA, LLENO_ID_ENCODER, VALIDO_ID, LLENO_PIN_ENCODER, VALIDO_PIN, NO_VALIDO_PIN,
	BLOQUEAR, ADMINISTRADOR, CLIENTE, AGREGAR_USUARIO, BORRAR_USUARIO, ABRIR_PUERTA, CAMBIAR_CLAVE, ERROR_TARJETA, AJUSTAR_BRILLO,
	PUERTA_CERRADA,GUARDAR_PIN,GUARDAR_ID_ENCODER,BORRADO_COMPLETO,BORRADO_INVALIDO,BRILLO_LISTO, NO_VALIDO_ID=100,

	FIN_TABLA
};

struct state_diagram_edge
{
	int evento;
	STATE *proximo_estado;
	void (*p_rut_accion)(void);
};

extern STATE inicio[];
extern STATE id_completo[];
extern STATE pin_completo[];
extern STATE tipo_usuario[];
extern STATE administrador[];
extern STATE agregar_usuario[];
extern STATE borrar_usuario[];
extern STATE cliente[];
extern STATE nueva_contrasenia[];
extern STATE puerta_activa[];
extern STATE cant_dig_pin[];
extern STATE ajustar_brillo[];
extern STATE administrador[];
extern STATE agregar_id_usuario[];
extern STATE id_encoder_nuevo[];
extern STATE agregar_pin_usuario[];
extern STATE borrar_usuario[];
extern STATE decision_longitud_pin[];

STATE* fsm(STATE *p_tabla_estado,BYTE evento_actual);

STATE* FSM_GetInitState(void);

#endif /* FSM_H_ */
