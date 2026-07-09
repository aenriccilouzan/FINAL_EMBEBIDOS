/*
 * encoder.h
 *
 *  Created on: Aug 30, 2025
 *      Author: julie
 */

#ifndef ENCODER_H_
#define ENCODER_H_


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define GIRO_IZQ -1
#define GIRO_DER 1

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

typedef void (*enc_callback_t)(bool);

bool enconderInit (enc_callback_t fun);
void encoderReset(void);
int update_encoder (void);
//void esperar_evento_encoder(void);
bool chequear_movimiento_encoder(void);

#endif /* ENCODER_H_ */
