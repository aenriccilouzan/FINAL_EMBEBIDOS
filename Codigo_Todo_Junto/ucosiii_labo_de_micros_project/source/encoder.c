/*
 * encoder.c
 *
 * Modified: Interrupt-based for Micrium RTOS
 */

#include "gpio.h"
#include "board.h"
#include "encoder.h"
#include "timer.h"
#include "os.h" // Librería Micrium

/* ============================================================================
 * Variables Internas
 * ==========================================================================*/
static bool state_A;
static bool state_B;
static int indicador_suma_resta = 0;
static bool current_state_A;
static bool current_state_B;
void cambiar_estado_A ();
void timer_fun(void);

// Variables del botón (Se mantienen con Timer)
static tim_id_t id_encoder_btn;
static tim_id_t id_encoder_1;

static enc_callback_t usuario_fun;
static bool boton_presionado;
bool estado_boton=false;

// --- MICRIUM ---
OS_SEM sem_enc_giro;
OS_SEM sem_enc_boton;
static OS_ERR os_err;



#define ENCODER_ACTIVE LOW
#define A_ENCODER_PIN (gpioRead(A_ENCODER))
#define B_ENCODER_PIN (gpioRead(B_ENCODER))

/* Prototipos */
void isr_encoder(void); // Nueva función de interrupción
void timer_btn_fun(void); // Timer solo para el botón

bool enconderInit (enc_callback_t fun){

    // 1. Crear Semáforo
    //OSSemCreate(&semaforo_encoder, "Sem Encoder", 0, &os_err);

    // 2. Configurar Pines
    gpioMode (BOTON_ENCODER, INPUT);
    // Configurar A y B como entradas (y pullups si tu placa lo requiere externamente, sino INPUT_PULLUP)
    gpioMode (A_ENCODER, INPUT);
    gpioMode (B_ENCODER, INPUT);

    usuario_fun = fun;

    // Estado inicial
    boton_presionado = (gpioRead(BOTON_ENCODER) == ENCODER_ACTIVE);
    state_A = (A_ENCODER_PIN == ENCODER_ACTIVE);
    state_B = (B_ENCODER_PIN == ENCODER_ACTIVE);

    // 3. ACTIVAR INTERRUPCIONES DE GPIO (Hardware)
    // Esto asegura que NO perdamos pasos aunque el RTOS esté durmiendo.
    // Usamos BOTH_EDGES para máxima precisión.
//    gpioIRQ(A_ENCODER, GPIO_IRQ_MODE_BOTH_EDGES, &isr_encoder);
//    gpioIRQ(B_ENCODER, GPIO_IRQ_MODE_BOTH_EDGES, &isr_encoder);

    // 4. Timer SOLO para el botón (10ms está bien para debounce)
    id_encoder_btn = timerStart(10, TIM_MODE_PERIODIC_CRITICAL, timer_btn_fun);
	timerStart(1, TIM_MODE_PERIODIC_CRITICAL , cambiar_estado_A);
    return boton_presionado;
}
void cambiar_estado_A (){
	static bool hubo_cambio=false;

	current_state_A = !A_ENCODER_PIN;
	current_state_B = !B_ENCODER_PIN;

	if(current_state_A != state_A){
		state_A = current_state_A;

		if((state_A == true) && (state_B == true)){ //ingresas desde el estado 11 y vas al 01
			indicador_suma_resta++;

		}
		else if((state_A == false) && (state_B == true)){ //ingresas desde el estado 01 y vas al 11
			indicador_suma_resta--;

		}
		else if((state_A == true) && (state_B == false)){ //ingresas desde el estado 10 y vas al estado 00
			indicador_suma_resta--;

		}
		else if((state_A == false) && (state_B == false)){ // ingresas desde el estado 00 y vas al 10
			indicador_suma_resta++;
		}
		hubo_cambio = true;	}
	if(current_state_B != state_B)
	{
		state_B=current_state_B;
		if((state_A == true) && (state_B == true)){ //ingresas desde el estado 11 y vas al 10
				indicador_suma_resta--;
			}
			else if((state_A == true) && (state_B == false)){ //ingresas desde el estado 10 y vas al 11
				indicador_suma_resta++;
			}
			else if((state_A == false) && (state_B == true)){ //ingresas desde el estado 01 y vas al estado 00
				indicador_suma_resta++;
			}
			else if((state_A == false) && (state_B == false)){ // ingresas desde el estado 00 y vas al 01
				indicador_suma_resta--;
			}
		hubo_cambio = true;
	}
	if (hubo_cambio) {
	        if (indicador_suma_resta >= 4 || indicador_suma_resta <= -4) {
	             OSSemPost(&sem_enc_giro, OS_OPT_POST_1, &os_err);
	        }
	    }
	hubo_cambio=false;
}

// --- ESTA ES LA INTERRUPCIÓN (Se ejecuta "sola" al mover la perilla) ---
//void isr_encoder(void){
//    // Avisamos a Micrium que estamos en una ISR (Importante para contexto)
//    OSIntEnter();
//
//    bool current_state_A = (A_ENCODER_PIN == ENCODER_ACTIVE);
//    bool current_state_B = (B_ENCODER_PIN == ENCODER_ACTIVE);
//    bool hubo_cambio = false;
//
//    // Máquina de estados para detectar dirección
//    if(current_state_A != state_A){
//        state_A = current_state_A;
//        if((state_A == true) && (state_B == true)) indicador_suma_resta++;
//        else if((state_A == false) && (state_B == true)) indicador_suma_resta--;
//        else if((state_A == true) && (state_B == false)) indicador_suma_resta--;
//        else if((state_A == false) && (state_B == false)) indicador_suma_resta++;
//        hubo_cambio = true;
//    }
//
//    if(current_state_B != state_B){
//        state_B = current_state_B;
//        if((state_A == true) && (state_B == true)) indicador_suma_resta--;
//        else if((state_A == true) && (state_B == false)) indicador_suma_resta++;
//        else if((state_A == false) && (state_B == true)) indicador_suma_resta++;
//        else if((state_A == false) && (state_B == false)) indicador_suma_resta--;
//        hubo_cambio = true;
//    }
//
//    // Si completamos un "click" (4 pasos lógicos), avisamos al RTOS
//    if (hubo_cambio) {
//        if (indicador_suma_resta >= 4 || indicador_suma_resta <= -4) {
//             OSSemPost(&sem_enc_giro, OS_OPT_POST_1, &os_err);
//        }
//    }
//
//    OSIntExit();
//}

// Función Timer para el botón (Polling 10ms)
void timer_btn_fun(void){
    static int contador = 0;
    if(contador > 0) contador--;

    if(contador == 0)
    {
        bool estado_actual = (gpioRead(BOTON_ENCODER) == ENCODER_ACTIVE);
        if(estado_actual != boton_presionado)
        {
            boton_presionado = estado_actual;
            //usuario_fun(boton_presionado);
            contador = 5; // Debounce simple
            estado_boton= boton_presionado;
            // Opcional: Avisar al semáforo si quieres despertar la tarea al pulsar
            OSSemPost(&sem_enc_boton, OS_OPT_POST_1, &os_err);
        }
    }
}

// Devuelve el evento y resetea el contador parcial
int update_encoder (void){
    // Sección crítica recomendada pero opcional si solo esta tarea lee
    // CPU_SR_ALLOC(); CPU_CRITICAL_ENTER();

    int evento = 0;
    if (indicador_suma_resta >= 4){
        indicador_suma_resta -= 4;
        evento = GIRO_DER;
    }
    else if (indicador_suma_resta <= -4){
        indicador_suma_resta += 4;
        evento = GIRO_IZQ;
    }

    // CPU_CRITICAL_EXIT();
    return evento;
}

void encoderReset(void)
{
    indicador_suma_resta = 0;
}

// Función No Bloqueante para consultar desde App_Run




































