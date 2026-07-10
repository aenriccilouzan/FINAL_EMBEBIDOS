/*
 * tarjeta_magnetica.c
 *
 *  Created on: 31 ago. 2025
 *      Author: andrea kpa
 */
#include "tarjeta_magnetica.h"
#include "gpio.h"
#include "board.h"
#include "os.h"
#include "timer.h"

/* ============================================================================
 * Module internal variables
 * ==========================================================================*/
// --- VARIABLES MICRIUM ---
OS_Q   queue_tarjeta;
static OS_ERR os_err;           // Variable para errores


static uint8_t digito_actual = 0;             /**< Acumula los bits del carácter en curso */
static uint8_t contador_bits = 0;             /**< Cuenta los bits recibidos de un carácter */
static bool columna_acumulada[CANTIDAD_DATOS_TARJETA] = {0}; /**< Verificación de paridad por columna */
static bool lectura_completa=false;                 /**< Indica si se completó la lectura de una tarjeta */
struct dataLM datos_tarjeta;                  /**< Estructura global con los datos decodificados */
static bool trama_iniciada = false;           /**< Señal de inicio de trama */

/* Prototipos de funciones internas */
static void guardar_caracter(void);
void reset_datos_tarjeta(void);



void inicializar_tarjeta(void)
{
	//OSSemCreate(&semaforo_tarjeta, "Sem Tarjeta", 0, &os_err);
	gpioMode(ENABLE_TARJETA_MAGNETICA, INPUT);
	gpioMode(DATA_TARJETA, INPUT);
	gpioMode(CLOCK_TARJETA_MAGNETICA, INPUT);
	reset_datos_tarjeta();
	gpioIRQ(CLOCK_TARJETA_MAGNETICA, GPIO_IRQ_MODE_FALLING_EDGE, &guardar_dato_serie);
}

void guardar_dato_serie(void){ //contador empieza en 0, recibo el primer bit lo guardo en la primera posicion

	OSIntEnter();
	//timerRestart(id_sleep);
	static bool contador_de_unos=0; //HACER UNA XOR QUE COMPARA PARIDAD CONTADORUNOS XOR= BIT ACTUAL
	bool bit_actual;

	if (gpioRead(ENABLE_TARJETA_MAGNETICA)==LOW)
	{
		bit_actual= !(gpioRead(DATA_TARJETA));//estamos negando pq la entrada está negada
		if(bit_actual==HIGH && trama_iniciada==false)
		{
			trama_iniciada=true;
		}
		if(trama_iniciada==true)
		{
			if(contador_bits < CANTIDAD_DATOS_TARJETA)
			{
				columna_acumulada[contador_bits] ^= bit_actual; //te guarda si las columnas son pares o impares
				digito_actual |= (bit_actual << contador_bits);
				contador_de_unos^= bit_actual; //para saber si es par o impar
				contador_bits++;
			}
			else
			{
				if((contador_de_unos==IMPAR && bit_actual==PAR) ||(contador_de_unos==PAR && bit_actual==IMPAR))
				{
					guardar_caracter();
				}
				else
				{
					datos_tarjeta.error=true;
				}
				contador_bits=0;
				contador_de_unos=0;
				digito_actual=0;

			}
		}

	}
	else
	{

	}
	OSIntExit();
}

void guardar_caracter(void)
{
    static uint8_t campo_actual = SS;
    static uint8_t indice_campo = 0;
    char caracter_ascii = digito_actual + '0';

    switch(campo_actual)
    {
    case SS:
        if(caracter_ascii == ';')
        {
            campo_actual = PAN;
            indice_campo = 0;
            reset_datos_tarjeta();
        }
        break;

    case PAN:
        if(caracter_ascii != '=')
        {
            if(indice_campo < SIZE_PAN)
            {
                datos_tarjeta.PAN[indice_campo] = caracter_ascii;
                indice_campo++;
            }
            else
            {
                datos_tarjeta.error = true;
                indice_campo = 0;
            }
        }
        else
        {
            campo_actual = AD;
            indice_campo = 0;
        }
        break;

    case AD:
        if(indice_campo < SIZE_AD)
        {
            datos_tarjeta.AD[indice_campo] = caracter_ascii;
            indice_campo++;
        }
        else
        {
            indice_campo = 1;
            datos_tarjeta.DD[0] = caracter_ascii;
            campo_actual = DD;
        }
        break;

    case DD:
        if(caracter_ascii != '?')
        {
            if(indice_campo < SIZE_DD)
            {
                datos_tarjeta.DD[indice_campo] = caracter_ascii;
                indice_campo++;
            }
            else
            {
                datos_tarjeta.error = true;
                indice_campo = 0;
            }
        }
        else
        {
            campo_actual = LRC;
            indice_campo = 0;
        }
        break;

    case LRC:
        trama_iniciada = false;
        contador_bits = 0;
        digito_actual = 0;
        indice_campo = 0;

        // ¡CORRECCIÓN 1! Sacamos el "static" para que se reinicie en cada lectura
        bool datos_incorrectos = false;

        // ¡CORRECCIÓN 2! Validamos solo los dígitos que realmente guardamos (hasta encontrar un 0x00)
        for(int i = 0; i < SIZE_PAN; i++)
        {
            if (datos_tarjeta.PAN[i] == 0) {
                break; // Terminamos de revisar los números reales, salimos del for
            }
            if (datos_tarjeta.PAN[i] > '9' || datos_tarjeta.PAN[i] < '0')
            {
                datos_incorrectos = true;
                break; // Apenas encontramos un error, dejamos de buscar
            }
        }

        //gpioToggle(PIN_LED_RED);

        // Además verificamos que no haya saltado la bandera de error durante la lectura de campos
        if(datos_incorrectos == false && datos_tarjeta.error == false)
        {
            OS_ERR err;
            OSQPost(&queue_tarjeta,
                    (void *)datos_tarjeta.PAN,
                    sizeof(datos_tarjeta.PAN),
                    OS_OPT_POST_FIFO,
                    &err);

            campo_actual = SS;
        }
        else
        {
            reset_datos_tarjeta();
            campo_actual = SS; // Reiniciamos la máquina de estados por si hubo error
        }
        break;
    }
}
void reset_datos_tarjeta(void)
{
	for(int i = 0; i < SIZE_PAN; i++) datos_tarjeta.PAN[i] = 0;
	for(int i = 0; i < SIZE_AD; i++) datos_tarjeta.AD[i] = 0;
	for(int i = 0; i < SIZE_DD; i++) datos_tarjeta.DD[i] = 0;
	datos_tarjeta.error = false;

}



// puede seguir escribiendo, preguntar si no es mas grande del tamaño, si si ERROR
// campo siguiente es cuando llega el igual


