/***************************************************************************//**
  @file     App.c
  @brief    Application functions
  @author   LAS + KPAS
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "tarjeta_magnetica.h"
#include "board.h"
#include "gpio.h"
#include "timer.h"
#include "MK64F12.h"
#include "MK64F12_features.h"
#include "encoder.h"
#include "display.h"
#include "fsm.h"
#include "os.h"

// Objetos para OSPendMulti

extern OS_SEM sem_enc_giro;   // Semáforo para A/B
extern OS_SEM sem_enc_boton;  // Semáforo para el botón

extern OS_Q AppQ;
static char cmd_piso1 = 'A';
static char cmd_piso2 = 'B';
static char cmd_piso3 = 'C';

//OS_ERR os_err;
/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define CANT_DIGITOS_ID 8
#define MAX_USERS 10
#define ENCODER_ACTIVE LOW
#define CANT_DIGITOS_PIN 5
#define MAX_DIGITOS 8

#define DIG4 4
#define DIG5 5
/* juli fijate si esto esta al pedo por el enum de fsm.h
define VALIDO_ID 1
*/

struct usuario
{
	char id[CANT_DIGITOS_ID];
	char pin[CANT_DIGITOS_PIN+1];
	bool es_un_pez_gordo;
	bool no_espacio_disponible;
};

typedef struct usuario USUARIO;

USUARIO usuarios[MAX_USERS];

typedef uint8_t id_t;

int tamanio=0;
struct dataLM* punt_dataLM;
void reset_encoder(void);
bool boton_encoder_presionado;
int num_id_activo = 0;
static int digito_activo = 0;
static char id_aux[MAX_DIGITOS]={0};
char data_encoder[MAX_DIGITOS]={0};
//uint8_t evento_global = VACIO;
static tim_id_t id_boton_encoder;
static bool timer_active_encoder = false;
int indicador_suma_resta = 0; //la G del paint lele
char valor_digito_encoder='0';
bool auxiliar=false;
bool correccion=false;
bool flag_clean = true;
tim_id_t id_sleep;
void esperar (void);
char covered_pin[MAX_DIGITOS]={0};
uint8_t evento_global2 = VACIO;
uint8_t evento_global_1= VACIO;
int usuario_agregado=0;
bool bloquear_boton=false;
/*Used to store FSM state*/
STATE* p2state;
extern STATE tabla_inicio[];
bool flag_derecha = false;
bool flag_izquierda = false;
enum {ADD_USER, DEL_USER, OPEN_DOOR, SUNSET};
static int evento_a_devolver=VACIO;
//static int evento_a_devolver_1;
bool no_avanzar=false;
int brillo_actual= BRILLO_REGULAR;
/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/
bool chequear_espacio_user(void);
void cambiar_estado_Boton (bool);
void inicio_a_id_completo(void);
void fun_timer_encoder(void);
void desplazar_digito_izquierda(void);
void desplazar_digito_derecha(void);
void giro_encoder(void);
int revisar_id(char id[]);
int revisar_pin(char id[]);
int atender_llamado_tarjeta_magnetica(void);
//int atender_llamado_encoder(void);
void sleep_reset(void);
int revisar_eventos(uint8_t);
void giro_encoder_opciones(void);
void menu_cliente (void);
int menu_nuevo_pin(void);
void update_cover_pin(void);
void imprimir_pin(void);
void borrar_id(int usuario);
void giro_encoder_opciones_brillito(void);
void update_cover_pin2(void);
void led_azul_prendido(void);

bool decision_tomada = false;
uint8_t decision_a_tomar = 0;
int decision_cliente = ABRIR_PUERTA;
int decision_pin = DIG4;
/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

/* Función que se llama 1 vez, al comienzo del programa */
void App_Init(void)
{
	//Inicializar los timers y configurarlos
     timerInit();
     //enconderInit(NULL);
     //Inicializar displays
     iniciar_display();

     //inicializamos la tarjeta
     inicializar_tarjeta();

     id_sleep = timerStart(10000, TIM_MODE_PERIODIC_CRITICAL, sleep_reset);

     atender_llamada_usuario("HOLA", false, brillo_actual);
     boton_encoder_presionado=enconderInit(&cambiar_estado_Boton);

 	//p2state=FSM_GetInitState();
     p2state=inicio;
 	//primer usuario
	for(int i=0; i<CANT_DIGITOS_ID;i++)
		usuarios[1].id[i]=i+'1';
 	for(int i=0; i<CANT_DIGITOS_PIN-1; i++)
 		usuarios[1].pin[i]=i+'1';
 	usuarios[1].es_un_pez_gordo=false;
 	usuarios[1].no_espacio_disponible=true;

 	//segundo usuario
 	for(int i=0; i<CANT_DIGITOS_ID;i++)
		usuarios[0].id[i]=i+'0';
 	for(int i=0; i<CANT_DIGITOS_PIN; i++)
 		usuarios[0].pin[i]=i+'0';
 	usuarios[0].es_un_pez_gordo=true;
 	usuarios[0].no_espacio_disponible=true;

 	usuarios[2].id[0]='6';
 	usuarios[2].id[1]='0';
 	usuarios[2].id[2]='6';
 	usuarios[2].id[3]='1';
 	usuarios[2].id[4]='2';
 	usuarios[2].id[5]='6';
 	usuarios[2].id[6]='8';
 	usuarios[2].id[7]='4';

 	usuarios[2].pin[0]='0';
 	usuarios[2].pin[1]='0';
 	usuarios[2].pin[2]='0';
 	usuarios[2].pin[3]='1';


 	usuarios[2].es_un_pez_gordo=true;
 	usuarios[2].no_espacio_disponible=true;

//	gpioMode(PIN_LED_BLUE, OUTPUT);
//	gpioWrite(PIN_LED_BLUE,HIGH);
}

/* Función que se llama constantemente en un ciclo infinito */
void App_Run (void)
{
   // mostrar_en_display(0b11111100, 1, 3);

	//Se actualizan los timers que no son críticos
	timerUpdate();
	//atender_llamada_usuario("IDEAS", true);
//	// Inicializo la FSM con el estado inicial, inicio en nuestro caso inicio


////
////	//Guarda en p2state un puntero al estado que se corresponde con el evento.
	//p2state=fsm(p2state,revisar_eventos());
}

/*******************************************************************************
 *******************************************************************************
						REVISAR LOS EVENTOS
 *******************************************************************************
 ******************************************************************************/

int revisar_eventos(uint8_t evento)
{
	//Se hace una cadena de ifs, else ifs donde podes recorrer todos los estados y las funciones que se hacen en cada uno de ellos.
	if (p2state == inicio) {

		//evento_a_devolver = atender_llamado_tarjeta_magnetica(); //Consulta si llego algun ID con la tarjeta magnética
		if(evento== LECTOR_TARJETA_CAMBIO)
		{
			inicio_a_id_completo();
			num_id_activo=revisar_id(data_encoder);
			if (num_id_activo == NO_VALIDO_ID){
				return NO_VALIDO_ID;
			}
			else
				return VALIDO_ID;
		}
		if(decision_tomada==true)
		{
			decision_tomada=false;
			return APRETO_BOTON;
		}
		else
			return VACIO;
		//evento_a_devolver = atender_llamado_encoder();
		//if(evento_a_devolver!= VACIO)
		//	return evento_a_devolver;
		//else
		//	return VACIO;
	}
	else if (p2state == id_completo) {
		static int lleno_id=0;
		if (evento == GIROENCODER_CAMBIO){
			giro_encoder();
		}
		if (digito_activo == CANT_DIGITOS_ID)
		{
			digito_activo = 0;
			lleno_id= LLENO_ID_ENCODER;
		}
		if(lleno_id == LLENO_ID_ENCODER)
		{
			lleno_id=0;
		//revisar_id devuelve un int, xq a la vez te da el número de cliente que entro.
			num_id_activo = revisar_id(data_encoder);//esta mal el nombre data_encoder xq también es de la tarjeta.
			if (num_id_activo == NO_VALIDO_ID)
				return NO_VALIDO_ID;
			else
				return VALIDO_ID;
		}
	}
	else if (p2state == pin_completo) {
		static int lleno_pin=0;
		if (evento == GIROENCODER_CAMBIO)giro_encoder();
			//evento_a_devolver_1 = atender_llamado_encoder();
			//if(evento_a_devolver_1 != VACIO)
			//	return FIN_TABLA;

			if (digito_activo == tamanio)
			{
				digito_activo = 0;
				lleno_pin = LLENO_PIN_ENCODER;
			}

			if (lleno_pin == LLENO_PIN_ENCODER) {
				lleno_pin=0;
				int pin_activo = 0;
				pin_activo = revisar_pin(data_encoder); //esta mal el nombre data_encoder xq tambi[en es de la tarjeta.
				if (pin_activo == NO_VALIDO_PIN)
					return NO_VALIDO_PIN;
				else if (pin_activo == VALIDO_PIN){
					led_azul_prendido();

					OS_ERR os_err;

					OSQPost(&AppQ,
					        &cmd_piso1,
					        sizeof(char),
					        OS_OPT_POST_FIFO,
					        &os_err);

					if(usuarios[num_id_activo].es_un_pez_gordo == true){
						atender_llamada_usuario("ADMIN", true, brillo_actual);
						OS_ERR os_err;
						OSTimeDlyHMSM(0u, 0u, 2u, 0u, OS_OPT_TIME_HMSM_STRICT, &os_err);
						return ADMINISTRADOR;
					}
					else{
						atender_llamada_usuario("USER", false, brillo_actual);
						OS_ERR os_err;
						OSTimeDlyHMSM(0u, 0u, 2u, 0u, OS_OPT_TIME_HMSM_STRICT, &os_err);
						return CLIENTE;
					}
				}
				else{
					return BLOQUEAR;
				}
			}
	}




	else if (p2state == administrador){
		if(evento==GIROENCODER_CAMBIO) giro_encoder_opciones();
		if(decision_tomada == true){
			decision_tomada=false;
			switch(decision_a_tomar){
			case(ADD_USER):
					return AGREGAR_USUARIO;
			break;
			case(DEL_USER):
					return BORRAR_USUARIO;
			break;
			case(OPEN_DOOR):
					return ABRIR_PUERTA;
			break;
			case(SUNSET):
					return AJUSTAR_BRILLO;
			break;

			default:
				break;
			}

		}

	}
	else if (p2state == cliente )
	{
		if(evento == GIROENCODER_CAMBIO) menu_cliente();
	    if(decision_tomada==true)
	    {
	    	decision_tomada=false;
	    	return decision_cliente;
	    }
	    else
	    	return VACIO;
	}

	else if (p2state == cant_dig_pin) //cambiar contraseña user
	{
		if(evento== GIROENCODER_CAMBIO) menu_nuevo_pin();
		if(decision_tomada==true)
		{
			decision_tomada=false;
			return APRETO_BOTON;
		}
		else
			return VACIO;

	}

	else if (p2state == nueva_contrasenia)
	{
		if(evento== GIROENCODER_CAMBIO)
		{
			giro_encoder();
			if (digito_activo == decision_pin)
			{
				digito_activo = 0;
				return LLENO_PIN_ENCODER;
			}
		}
		else
			return VACIO;
	}
	else if (p2state == agregar_id_usuario){ //esto es del admin
		if(evento == LECTOR_TARJETA_CAMBIO)
		{
			inicio_a_id_completo();
			bool validar_id=true;
			int j;
			for(j=0;  j<MAX_USERS && validar_id ; j++ )
			{
				for(int i=0;usuarios[j].id[i]==data_encoder[i] && i<CANT_DIGITOS_ID; i++)
				{
					if(i==CANT_DIGITOS_ID-1)
						validar_id=false;
				}
			}
			if (j == MAX_USERS){
				if(chequear_espacio_user())
					return VALIDO_ID;
				else
					return NO_VALIDO_ID;
			}
			else
				return NO_VALIDO_ID;
		}
			 //Consulta si llego algun ID con la tarjeta magnética
		if(decision_tomada==true)
		{
			decision_tomada=false;
			return APRETO_BOTON;
		}
		else
			return VACIO;
	}
	else if (p2state == id_encoder_nuevo){
		static int guardar_id_aux=0;
		if(evento == GIROENCODER_CAMBIO)	giro_encoder();
		if (digito_activo == CANT_DIGITOS_ID)
		{
			digito_activo = 0;
			guardar_id_aux = GUARDAR_ID_ENCODER;
		}
		else
			return VACIO;
		if (guardar_id_aux == GUARDAR_ID_ENCODER){
			guardar_id_aux=0;
			int j;
			bool validar_id=true;
			for(j=0;  j<MAX_USERS && validar_id ; j++ )
			{
				for(int i=0;usuarios[j].id[i]==data_encoder[i] && i<CANT_DIGITOS_ID; i++)
				{
					if(i==CANT_DIGITOS_ID-1)
						validar_id=false;
				}
			}
			if (j == MAX_USERS){
				if(chequear_espacio_user())
					return VALIDO_ID;
				else
					return NO_VALIDO_ID;
			}
			else
				return NO_VALIDO_ID;
		}
	}
	else if (p2state == decision_longitud_pin)
	{
		 if (evento == GIROENCODER_CAMBIO) menu_nuevo_pin();
		 if(decision_tomada==true)
		 {
			decision_tomada=false;
			return APRETO_BOTON;
		 }
		 else
		    return VACIO;

	}
	else if (p2state == agregar_pin_usuario){
		if(evento== GIROENCODER_CAMBIO) giro_encoder();
		if (digito_activo == decision_pin)
		{
			digito_activo = 0;
			for(int i = 0; i < decision_pin; i++){
				usuarios[usuario_agregado].pin[i] = data_encoder[i];
			}
			usuario_agregado=0;
			return GUARDAR_PIN;
		}
		else
			return VACIO;
	}
	else if (p2state==borrar_usuario){
		if(evento== GIROENCODER_CAMBIO)	giro_encoder();
		if (digito_activo == CANT_DIGITOS_ID)
		{
			digito_activo = 0;
			bool validar_id=true;
			int j;
			for(j=0;  j<MAX_USERS && validar_id ; j++ )
			{
				for(int i=0;usuarios[j].id[i]==data_encoder[i] && i<CANT_DIGITOS_ID; i++)
				{
					if(i==CANT_DIGITOS_ID-1)
						validar_id=false;
				}
			}
			if(validar_id || (usuarios[j-1].es_un_pez_gordo==true))
			{
				atender_llamada_usuario("USER NO VALIDO", true, brillo_actual);
				return BORRADO_INVALIDO;
			}
			else{
				borrar_id(j);
				return BORRADO_COMPLETO;
			}
		}
	}
	else if (p2state == ajustar_brillo) {
		if(evento== GIROENCODER_CAMBIO)	giro_encoder_opciones_brillito();
		if (decision_tomada == true) {
			brillo_actual=decision_a_tomar;
			return BRILLO_LISTO;
		}
	}
	return VACIO;
}

/*******************************************************************************
 *******************************************************************************
			FUNCIONES QUE SE LLAMAN EN EL SWITCH DE ARRIBA
 *******************************************************************************
 ******************************************************************************/

int menu_nuevo_pin(void){
	switch (update_encoder()){
	    case GIRO_DER:
	    	timerRestart(id_sleep);
	    	bloquear_boton=false;
	        if (decision_pin == DIG4){
	            decision_pin = DIG5;
	            atender_llamada_usuario("DIG5", false, brillo_actual);
	        }
	        else if (decision_pin == DIG5){
	            decision_pin = DIG4;
	            atender_llamada_usuario("DIG4", false, brillo_actual);
	        }
	        break;
	    case GIRO_IZQ:
	    	timerRestart(id_sleep);
	    	bloquear_boton=false;
	        if (decision_pin == DIG4){
	            decision_pin = DIG5;
	            atender_llamada_usuario("DIG5", false, brillo_actual);
	        }
	        else if (decision_pin == DIG5){
	            decision_pin = DIG4;
	            atender_llamada_usuario("DIG4", false, brillo_actual);
	        }
	        break;
	    }
	return decision_pin;
}
void menu_cliente (void){
    switch (update_encoder()){

    case GIRO_DER:
    	timerRestart(id_sleep);
        if (decision_cliente == CAMBIAR_CLAVE){
            decision_cliente = ABRIR_PUERTA;
            atender_llamada_usuario("ABRIR PUERTA", true, brillo_actual);
        }
        else if (decision_cliente == ABRIR_PUERTA){
            decision_cliente = CAMBIAR_CLAVE;
            atender_llamada_usuario("DIFF CLAVE", true, brillo_actual);
        }
        break;
    case GIRO_IZQ:
    	timerRestart(id_sleep);
        if (decision_cliente == CAMBIAR_CLAVE){
            decision_cliente = ABRIR_PUERTA;
            atender_llamada_usuario("ABRIR PUERTA", true, brillo_actual);
        }
        else if (decision_cliente == ABRIR_PUERTA){
            decision_cliente = CAMBIAR_CLAVE;
            atender_llamada_usuario("DIFF CLAVE", true, brillo_actual);
        }
        break;
    }
}



void giro_encoder_opciones (void){
	switch (update_encoder()){

	case GIRO_DER:
		timerRestart(id_sleep);
		if (decision_a_tomar == ADD_USER){
			decision_a_tomar = DEL_USER;
			atender_llamada_usuario("DEL USER", true, brillo_actual);
		}
		else if (decision_a_tomar == DEL_USER){
			decision_a_tomar = OPEN_DOOR;
			atender_llamada_usuario("ABRIR DOOR", true, brillo_actual);
		}
		else if (decision_a_tomar == OPEN_DOOR){
			decision_a_tomar = SUNSET;
			atender_llamada_usuario("BRILLO", true, brillo_actual);
		}
		else if (decision_a_tomar == SUNSET){
			decision_a_tomar = ADD_USER;
			atender_llamada_usuario("ADD USER", true, brillo_actual);
		}
		break;
	case GIRO_IZQ://add->del->open->brillo
		timerRestart(id_sleep);
		if (decision_a_tomar == ADD_USER){
			decision_a_tomar = SUNSET;
			atender_llamada_usuario("BRILLO", true, brillo_actual);
		}
		else if (decision_a_tomar == DEL_USER){
			decision_a_tomar = ADD_USER;
			atender_llamada_usuario("ADD USER", true, brillo_actual);
		}
		else if (decision_a_tomar == OPEN_DOOR){
			decision_a_tomar = DEL_USER;
			atender_llamada_usuario("DEL USER", true, brillo_actual);
		}
		else if (decision_a_tomar == SUNSET){
			decision_a_tomar = OPEN_DOOR;
			atender_llamada_usuario("ABRIR DOOR", true, brillo_actual);
		}
		break;

	}
}

int atender_llamado_tarjeta_magnetica(void) {
	//Estado Inicio. Hay 2 eventos, tarjeta y botón de encoder.
	//Se llama a la función del driver tarjeta para preguntar si alguien pasó alguna.
	punt_dataLM = recibiendo_data();
	//Si alguien pasó la tarjeta se devuelve el evento PASO_TARJETA
	if (punt_dataLM != NULL)
	{
		timerRestart(id_sleep);
//		if (punt_dataLM->error == true)	//Si alguien no paso la tarjeta o hubo un error se devuelve el evento ERROR_TARJETA.
//		{
//			punt_dataLM = NULL;
//			return ERROR_TARJETA;
//		}
		return PASO_TARJETA;
	}

	return VACIO;
}

void ir_a_id_encoder(void)
{
	reset_encoder();
	atender_llamada_usuario("ACCESS ID", true, brillo_actual);
	bloquear_boton=true;
}
/*
int atender_llamado_encoder(void) {
	if (evento_global == APRETO_BOTON)
	{
		evento_global = VACIO;
		reset_encoder();
		return APRETO_BOTON;
	}
	return VACIO;
}*/


//Para llegar acá ya se tuvo q haber apretado el botón.

//digito_activo vale 4 --> aux_id hay q copiarle digito_acitov - CANT_DISPLAYS - 1 y vas copinado hasrta que



void giro_encoder(void)
{
	switch (update_encoder())
	{
	case GIRO_DER:
		bloquear_boton=false;
		flag_derecha = true;
		if (flag_izquierda == true){
			valor_digito_encoder +=2 ;
			flag_izquierda = false;
		}
		if(correccion==true)
		{
			valor_digito_encoder++;
			correccion=false;
		}
		timerRestart(id_sleep); //Reinicia el timer de 10 segundos por ya que la persona hizo una actividad. Entonces, hay que empezar a contar
		//Cuando la persona hace un giro a la derecha, se incrementa el número y además se imprime en la pantalla.
		if (valor_digito_encoder <= '9') {
			data_encoder[digito_activo]=valor_digito_encoder;
			if(digito_activo<CANTIDAD_DISPLAYS){
			if(p2state==agregar_pin_usuario || p2state==nueva_contrasenia ||p2state== pin_completo)
			{
				update_cover_pin();
				atender_llamada_usuario(covered_pin, false, brillo_actual);
			}
			else
				atender_llamada_usuario(data_encoder, false, brillo_actual);
			}//¿Para mi está mal, no hay que poner data_encoder, hay que poner valor_digito_encoder?. Data_encoder hay que llamarlo una vez que se aprete el botón
			else if (digito_activo>=CANTIDAD_DISPLAYS)
			{
				int j=0;
				for(int i=digito_activo - CANTIDAD_DISPLAYS +1; i <= digito_activo && j<CANTIDAD_DISPLAYS; i++ )
				{
					id_aux[j]=data_encoder[i];
					j++;
				}
				if(p2state==agregar_pin_usuario || p2state==nueva_contrasenia ||p2state== pin_completo)
				{
					update_cover_pin2();
					atender_llamada_usuario(covered_pin, false, brillo_actual);
				}
				else
					atender_llamada_usuario(id_aux, false, brillo_actual);

			}

			++valor_digito_encoder; //¿Si a un char le haces ++ funciona bien?
		}
		//Si ya dio la vuelta completa, que vuelva a 0.
		else {
			valor_digito_encoder = '0'; //¿No habría que imprimir este número también?
			data_encoder[digito_activo] = valor_digito_encoder;
			if(digito_activo<CANTIDAD_DISPLAYS){
				if(p2state==agregar_pin_usuario || p2state==nueva_contrasenia ||p2state== pin_completo)
				{
					update_cover_pin();
					atender_llamada_usuario(covered_pin, false, brillo_actual);
				}
				else
					atender_llamada_usuario(data_encoder, false, brillo_actual);
				//¿Para mi está mal, no hay que poner data_encoder, hay que poner valor_digito_encoder?. Data_encoder hay que llamarlo una vez que se aprete el botón
			}
			else if (digito_activo>=CANTIDAD_DISPLAYS)
			{
				int j=0;
				for(int i=digito_activo - CANTIDAD_DISPLAYS +1; i <= digito_activo && j<CANTIDAD_DISPLAYS; i++ )
				{
					id_aux[j]=data_encoder[i];
					j++;
				}
				if(p2state==agregar_pin_usuario || p2state==nueva_contrasenia ||p2state== pin_completo)
				{
					update_cover_pin2();
					atender_llamada_usuario(covered_pin, false, brillo_actual);
				}
				else
					atender_llamada_usuario(id_aux, false, brillo_actual);

			}

			valor_digito_encoder++; //¿Si a un char le haces ++ funciona bien?
		}
		break;
	case GIRO_IZQ:
		bloquear_boton=false;
		if (flag_derecha == true){
			valor_digito_encoder-= 2;
			flag_derecha = false;
		}
		flag_izquierda = true;

		if(correccion==true)
		{
			valor_digito_encoder--;
			correccion=false;
		}

		timerRestart(id_sleep);
		//Cuando la persona hace un giro a la izquierda, se incrementa el número y además se imprime en la pantalla.
		if (valor_digito_encoder >= '0') {
			data_encoder[digito_activo]=valor_digito_encoder;
			if(digito_activo<CANTIDAD_DISPLAYS){
				if(p2state==agregar_pin_usuario || p2state==nueva_contrasenia || p2state==pin_completo)
				{
					update_cover_pin();
					atender_llamada_usuario(covered_pin, false, brillo_actual);
				}
				else
					atender_llamada_usuario(data_encoder, false, brillo_actual); //¿Para mi está mal, no hay que poner data_encoder, hay que poner valor_digito_encoder?. Data_encoder hay que llamarlo una vez que se aprete el botón
			}
			else if (digito_activo>=CANTIDAD_DISPLAYS)
			{
				int j=0;
				for(int i=digito_activo - CANTIDAD_DISPLAYS +1; i <= digito_activo && j<CANTIDAD_DISPLAYS; i++ )
				{
					id_aux[j]=data_encoder[i];
					j++;
				}
				if(p2state==agregar_pin_usuario || p2state==nueva_contrasenia ||p2state== pin_completo)
				{
					update_cover_pin2();
					atender_llamada_usuario(covered_pin, false, brillo_actual);
				}
				else
					atender_llamada_usuario(id_aux, false, brillo_actual);

			}
			valor_digito_encoder--;
		}
		else {
			valor_digito_encoder = '9'; //¿No habría que imprimir este número también?
			data_encoder[digito_activo] = valor_digito_encoder;
			if(digito_activo<CANTIDAD_DISPLAYS)
			{
				if(p2state==agregar_pin_usuario || p2state==nueva_contrasenia ||p2state== pin_completo)
				{
					update_cover_pin();
					atender_llamada_usuario(covered_pin, false, brillo_actual);
				}
				else
					atender_llamada_usuario(data_encoder, false, brillo_actual); //¿Para mi está mal, no hay que poner data_encoder, hay que poner valor_digito_encoder?. Data_encoder hay que llamarlo una vez que se aprete el botón
			}
			else if (digito_activo>=CANTIDAD_DISPLAYS)
			{
				int j=0;
				for(int i=digito_activo - CANTIDAD_DISPLAYS +1; i <= digito_activo && j<CANTIDAD_DISPLAYS; i++ )
				{
					id_aux[j]=data_encoder[i];
					j++;
				}
				if(p2state==agregar_pin_usuario || p2state==nueva_contrasenia ||p2state== pin_completo)
				{
					update_cover_pin2();
					atender_llamada_usuario(covered_pin, false, brillo_actual);
				}
				else
					atender_llamada_usuario(id_aux, false, brillo_actual);

			}
			valor_digito_encoder--; //¿Si a un char le haces ++ funciona bien? si

		}
		break;
	default:
		break;
	}
}
// Acá hay algo raro, porque estos valor_digito_encoder son solamente si se giro en encoder. Pero, para saber si de verdad es el valor que quería,
// hay que ver si se apreto el botón con la función de abajo. Por eso, estos valor_digito_encoder no es lo que hay que imprimir.
// Nunca se imprime correctamente data_encoder.

void giro_encoder_opciones_brillito (void) {
    switch (update_encoder()) {

    case GIRO_DER:
    	timerRestart(id_sleep);
        if (decision_a_tomar == BRILLO_MUY_BAJO) {
            decision_a_tomar = BRILLO_BAJO;
            atender_llamada_usuario("2", false, BRILLO_BAJO);
        }
        else if (decision_a_tomar == BRILLO_BAJO) {
            decision_a_tomar = BRILLO_REGULAR;
            atender_llamada_usuario("3", false, BRILLO_REGULAR);
        }
        else if (decision_a_tomar == BRILLO_REGULAR) {
            decision_a_tomar = BRILLO_ALTO;
            atender_llamada_usuario("4", false, BRILLO_ALTO);
        }
        else if (decision_a_tomar == BRILLO_ALTO) {
            decision_a_tomar = BRILLO_MUY_ALTO;
            atender_llamada_usuario("5", false, BRILLO_MUY_ALTO);
        }
        else if (decision_a_tomar == BRILLO_MUY_ALTO) {
            decision_a_tomar = BRILLO_MUY_BAJO;
            atender_llamada_usuario("1", false, BRILLO_MUY_BAJO);
        }
        break;
    case GIRO_IZQ:
    	timerRestart(id_sleep);
        if (decision_a_tomar == BRILLO_MUY_BAJO) {
            decision_a_tomar = BRILLO_MUY_ALTO;
            atender_llamada_usuario("5", false, BRILLO_MUY_ALTO);
        }
        else if (decision_a_tomar == BRILLO_BAJO) {
            decision_a_tomar = BRILLO_MUY_BAJO;
            atender_llamada_usuario("1", false, BRILLO_MUY_BAJO);
        }
        else if (decision_a_tomar == BRILLO_REGULAR) {
            decision_a_tomar = BRILLO_BAJO;
            atender_llamada_usuario("2", false, BRILLO_BAJO);
        }
        else if (decision_a_tomar == BRILLO_ALTO) {
            decision_a_tomar = BRILLO_REGULAR;
            atender_llamada_usuario("3", false, BRILLO_REGULAR);
        }
        else if (decision_a_tomar == BRILLO_MUY_ALTO) {
            decision_a_tomar = BRILLO_ALTO;
            atender_llamada_usuario("4", false, BRILLO_ALTO);
        }
        break;
    }
}


int revisar_id(char id[])
{
	for(int j=0; j<MAX_USERS; j++)
	{
		if (usuarios[j].no_espacio_disponible == false) {
			continue;
		}
		int cant_char_ok=0;
		for(int i=0; i<CANT_DIGITOS_ID && (id[i]==usuarios[j].id[i]); i++)
		{
				cant_char_ok++;

		}
		if(cant_char_ok== (CANT_DIGITOS_ID))
		{
			 //El ID es valido y j es el indice de usario usado
			atender_llamada_usuario("TRUE", false , brillo_actual);
			OS_ERR os_err;
			OSTimeDlyHMSM(0u, 0u, 2u, 0u, OS_OPT_TIME_HMSM_STRICT, &os_err);
			return j;

		}
	}
	return NO_VALIDO_ID;
}

int revisar_pin(char pin[])
{
	static int cant_intentos=0;
	int cant_char_ok = 0;
	for(int i=0; i<tamanio; i++)
	{
		if(pin[i]!= usuarios[num_id_activo].pin[i])
		{
			cant_intentos++;
			if(0 < cant_intentos && cant_intentos < 3) //agregué que cant_intentos sea mayor que 0
			{
				return NO_VALIDO_PIN; //faltaría hacer el retardo!
			}
			else
			{
				cant_intentos=0;
				return BLOQUEAR;
			}
		}
		else
		{
			cant_char_ok++;
		}
	}
	if (cant_char_ok == tamanio) { //agregu[e esto
		cant_intentos=0;
		return VALIDO_PIN;
	}
	return VACIO;
}

void led_azul_prendido(void)
{
	atender_llamada_usuario("HOLA", true, brillo_actual);
	gpioWrite(PIN_LED_BLUE,LOW);
	OS_ERR os_err;
	OSTimeDlyHMSM(0u, 0u, 2u, 0u, OS_OPT_TIME_HMSM_STRICT, &os_err);
	gpioWrite(PIN_LED_BLUE,HIGH);

}


void reset_encoder(void)
{
	digito_activo = 0;
	valor_digito_encoder = '0';
	for (int i = 0; i < MAX_DIGITOS; i++)
		data_encoder[i] = 0;
	encoderReset();
}

void inicio_a_id_completo(void)
{
	for (int i = 0; i < CANT_DIGITOS_ID; i++)
	{
		data_encoder[i] = punt_dataLM->PAN[i];
	}

	//reset_datos_tarjeta();
	gpioIRQ(CLOCK_TARJETA_MAGNETICA, GPIO_IRQ_MODE_DISABLE, &guardar_dato_serie);

}
void reiniciar(void)
{
	atender_llamada_usuario("HOLA", false, brillo_actual);
	bloquear_boton = false;
	evento_a_devolver=VACIO;
	decision_tomada = false;
	punt_dataLM = NULL;
	num_id_activo=0;
	reset_datos_tarjeta();
	gpioIRQ(CLOCK_TARJETA_MAGNETICA, GPIO_IRQ_MODE_FALLING_EDGE, &guardar_dato_serie);
	//App_Init();
	//evento_global=VACIO;
}
void ir_pin_encoder(void)
{
	tamanio=0;
	for(int i=0;usuarios[num_id_activo].pin[i]!=0 && i<CANT_DIGITOS_PIN;i++)
	{
		tamanio++;
	}
	atender_llamada_usuario(PASSWORD, true, brillo_actual); //imprimis la segunda bienvenida.
	reset_encoder();
	//evento_global=VACIO;
	bloquear_boton=false;
}

void pin_invalido(void)
{
	atender_llamada_usuario("CODE ERROR", true, brillo_actual);
	reset_encoder();
	bloquear_boton=true;
}

void bloquear_id(void)
{
	for(int i=0; i<CANT_DIGITOS_ID;i++)
		usuarios[num_id_activo].id[i]=0;

	for(int i=0; i<CANT_DIGITOS_PIN;i++)
		usuarios[num_id_activo].pin[i]=0;

	usuarios[num_id_activo].no_espacio_disponible = false;

	atender_llamada_usuario("USUARIO BLOQUEADO", true, brillo_actual);
	gpioIRQ(CLOCK_TARJETA_MAGNETICA, GPIO_IRQ_MODE_FALLING_EDGE, &guardar_dato_serie);
}

void borrar_id(int usuario)
{
	for(int i=0; i<CANT_DIGITOS_ID;i++)
		usuarios[usuario - 1].id[i]=0;

	for(int i=0; i<CANT_DIGITOS_PIN;i++)
		usuarios[usuario - 1].pin[i]=0;

	usuarios[usuario].no_espacio_disponible = false;

	atender_llamada_usuario("USUARIO BORRADO", true, brillo_actual);
}

void reset_boton(void){
	//evento_global=VACIO;
	gpioIRQ(CLOCK_TARJETA_MAGNETICA, GPIO_IRQ_MODE_FALLING_EDGE, &guardar_dato_serie);
}

void id_invalido(void)
{
	atender_llamada_usuario("ID ERROR", true, brillo_actual);
	OS_ERR os_err;
	OSTimeDlyHMSM(0u, 0u, 5u, 0u, OS_OPT_TIME_HMSM_STRICT, &os_err);
	reset_encoder();
	reiniciar();
	bloquear_boton=true;
	//evento_global=VACIO;
	//gpioIRQ(CLOCK_TARJETA_MAGNETICA, GPIO_IRQ_MODE_FALLING_EDGE, &guardar_dato_serie);
}

void sleep_reset(void)
{
	reiniciar();
	p2state=inicio;
}

void ir_a_estado_cliente(void)
{
	reset_encoder();
	decision_cliente=ABRIR_PUERTA;
	atender_llamada_usuario("ABRIR PUERTA", true, brillo_actual);
	//evento_global=VACIO;
}

void ir_a_estado_cliente2(void)
{
	for(int i=0; i<decision_pin;i++)
	{
	usuarios[num_id_activo].pin[i]=data_encoder[i];
	}

	reset_encoder();
	//evento_global=VACIO;
	decision_cliente=ABRIR_PUERTA;
	atender_llamada_usuario("ABRIR PUERTA",true, brillo_actual);
}
void update_cover_pin(void)
{
	for(int i=0; i<digito_activo;i++)
	{
		covered_pin[i]='-';
	}
	covered_pin[digito_activo]=data_encoder[digito_activo];
	covered_pin[digito_activo+1]=0;
}

void update_cover_pin2(void)
{
	for(int i=0; i<digito_activo-2;i++)
	{
		covered_pin[i]='-';
	}
	covered_pin[digito_activo-1]=id_aux[digito_activo-1];
	covered_pin[digito_activo]=0;
}

void ir_a_estado_administrador(void)
{
	decision_a_tomar=ADD_USER;
	atender_llamada_usuario("ADD USER", true, brillo_actual);
	reset_encoder();

}
void ingrese_id (void)
{
	atender_llamada_usuario("ADD ID", true, brillo_actual);
	reset_encoder();
	bloquear_boton=true;
	gpioIRQ(CLOCK_TARJETA_MAGNETICA, GPIO_IRQ_MODE_FALLING_EDGE, &guardar_dato_serie);
	//evento_global=VACIO;
}


void puerta_timer(void)
{
	OS_ERR os_err;
	prender_ledcitos(1);
	atender_llamada_usuario("ABIERTO", true, brillo_actual);
	OSTimeDlyHMSM(0u, 0u, 5u, 0u, OS_OPT_TIME_HMSM_STRICT, &os_err);
	prender_ledcitos(0);
	reiniciar();

}
void ir_a_cant_dig_pin(void)
{
	reset_encoder();
	decision_pin=DIG4;
	//evento_global=VACIO;
	atender_llamada_usuario("DIG4", false, brillo_actual);
	bloquear_boton=true;
}

void ir_a_nueva_contrasenia(void)
{
	reset_encoder();
	//evento_global=VACIO;
	atender_llamada_usuario("ACCESS CODE", true, brillo_actual);
	bloquear_boton=true;
}

void ir_a_brillo(void)
{
	reset_encoder();
	decision_a_tomar=brillo_actual;
	char arr_aux[2];
	arr_aux[0]=brillo_actual+'0';
	arr_aux[1]=0;
	atender_llamada_usuario(arr_aux, false, brillo_actual);

}

void ir_a_borrar_usuario(void)
{
	atender_llamada_usuario("DEL ID", true, brillo_actual);
	reset_encoder();
	bloquear_boton=true;
}

/*******************************************************************************
 *******************************************************************************
			FUNCIONES QUE SE LLAMAN EN EL CAMBIO DE ESTADOS
 *******************************************************************************
 ******************************************************************************/

/*******************************************************************************
 ******************************************************************************/

void cambiar_estado_Boton (bool boton_encoder_pres){

	if (boton_encoder_pres == true){
		timerRestart(id_sleep);
		flag_derecha = false;
		flag_izquierda = false;

		if(p2state==inicio)
		{
			no_avanzar=true;
		}
		else no_avanzar=false;

//		if(p2state == agregar_id_usuario)
//		{
//			no_avanzar_1=true;
//		}
//		else no_avanzar_1 =false;

		//evento_global=APRETO_BOTON;

		if(flag_clean == true){
		id_boton_encoder=timerStart(3000, TIM_MODE_SINGLESHOT_CRITICAL, fun_timer_encoder); //no nos importa si es 501 ms o 499ms
		//g no tiene sentido que se reinicie cuando se aprieta el botón porque siempre queda en el mismo estado 11.
		}
		timer_active_encoder = true;

	}
	else{
		timerRestart(id_sleep);
		if (flag_clean == false){
			prender_ledcitos(TURN_OFF_ALL_LEDS);
		}
		flag_clean = true;
		if(timer_active_encoder){
			timerStop(id_boton_encoder);
			timer_active_encoder = false;
			if(p2state==administrador||p2state==ajustar_brillo||p2state==cliente||p2state==cant_dig_pin||p2state==decision_longitud_pin||p2state==inicio||p2state==agregar_id_usuario)
			{
				decision_tomada = true;
			}
			else decision_tomada = false;
			if(no_avanzar==false && bloquear_boton==false)
				desplazar_digito_derecha();
		}
	}
	//OS_ERR os_err;
	//OSSemPost(&sem_enc_boton, OS_OPT_POST_1, &os_err);

}
//si vos mantenes apretado el boton, se resta
void fun_timer_encoder (void){
	if(no_avanzar==false && bloquear_boton==false)
		desplazar_digito_izquierda(); //implica que se va a borrar un digito, el que pusiste, o sea el de la izquierda
	timer_active_encoder = false;
	prender_ledcitos(PRENDER_TERCER_LED);
	flag_clean = false;
}
void desplazar_digito_izquierda(void){
	if(digito_activo>0)
	{
		data_encoder[digito_activo]=0;
		digito_activo--;
	}
	else
		data_encoder[digito_activo]='0';
	correccion=true;
	valor_digito_encoder = data_encoder[digito_activo];

	if(p2state==agregar_pin_usuario || p2state==nueva_contrasenia ||p2state== pin_completo)
	{
		update_cover_pin();
		atender_llamada_usuario(covered_pin, false, brillo_actual);
	}
	else
		atender_llamada_usuario(data_encoder, false, brillo_actual);
}
void desplazar_digito_derecha(void){
	digito_activo++;
	valor_digito_encoder = '0'; //cuando entras aca implica que se apreto el bot[on y se solto. Entonces, tenes q pasar al siguiente digito reiniciando el valor q tenias antes
}
void esperar (void)
{
//	timerStart(0.5, TIM_MODE_SINGLESHOT_NO_CRITICAL, ir_a_estado_administrador);

}
bool chequear_espacio_user(void)
{
	for(int i=0; i<MAX_USERS; i++)
	{
			if(usuarios[i].no_espacio_disponible==false)
			{
				for(int j=0; j<CANT_DIGITOS_ID; j++)
					usuarios[i].id[j]=data_encoder[j];

				usuarios[i].no_espacio_disponible=true;
				usuario_agregado = i;
				return true;
			}
	}
	return false;
}
void imprimir_pin(void)
{
	usuario_agregado=0;
	num_id_activo=0;
	gpioIRQ(CLOCK_TARJETA_MAGNETICA, GPIO_IRQ_MODE_FALLING_EDGE, &guardar_dato_serie);
	atender_llamada_usuario("SUCCESSFUL", false, brillo_actual);
}

/*******************************************************************************
 ******************************************************************************/
