#include "gpio.h"
#include "MK64F12.h"
#include "MK64F12_features.h"
#include "board.h"
#include "hardware.h"
#include "OS.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
#define MASKALL1S 0xFFFFFFFF		//cada F son 4 unos asi son 32 bits
#define MASKPIN(a) ((uint32_t)(1u << ((PIN2NUM(a)))))
#define RISING_EDGE_IRQ 0b1001U
#define FALLING_EDGE_IRQ 0b1010U
#define EITHER_EDGE_IRQ 0b1011U

#define NULL ((void *)0)
#define TRUE 1
#define FALSE 0

/* ============================================================================
 * Module internal variables
 * ==========================================================================*/

static GPIO_Type * const gpio[] = GPIO_BASE_PTRS; //const hace q no se escriba en la ram y si en la flash y static hace q no lo puedan ver otras funciones
 PORT_Type * const port[] = PORT_BASE_PTRS;

static uint8_t PortA_pines_irq [32] = {0};
static uint8_t PortB_pines_irq [32] = {0};
static uint8_t PortC_pines_irq [32] = {0};
static uint8_t PortD_pines_irq [32] = {0};
static uint8_t PortE_pines_irq [32] = {0};

static unsigned int contadorA = 0;
static unsigned int contadorB = 0;
static unsigned int contadorC = 0;
static unsigned int contadorD = 0;
static unsigned int contadorE = 0;

/*Prototipos de funciones internas*/
static pinIrqFun_t irqHandler[160] = {0}; //arreglo de punteros a callbacks para interrupciones por pin
//los primeros 3 bits son del puerto (va de 0 a 4) y los otros 5 son del nro de pin, el nro maximo es 159 asi que entra bien


void gpioClear_ISF (pin_t pin);

/**
 * @brief Configures the specified pin to behave either as an input or an output
 * @param pin the pin whose mode you wish to set (according PORTNUM2PIN)
 * @param mode INPUT, OUTPUT, INPUT_PULLUP or INPUT_PULLDOWN.
 */
void gpioMode (pin_t pin, uint8_t mode)
{
	//primero ponemos el pin para que sea gpio
	port[PIN2PORT(pin)]->PCR[PIN2NUM(pin)] = PORT_PCR_MUX(1)| PORT_PCR_DSE(1);

	if (mode==INPUT_PULLDOWN)
	{
		port[PIN2PORT(pin)]->PCR[PIN2NUM(pin)] |= (PORT_PCR_PE(1) | PORT_PCR_PS(0)); //seteo el bit 0 y 1 como enable y pulldown, el enable es para poder poner el pulldown

	}
	if(mode==INPUT_PULLUP)
	{
		port[PIN2PORT(pin)]->PCR[PIN2NUM(pin)] |= (PORT_PCR_PE(1) | PORT_PCR_PS(1));
	}

	if (mode==INPUT || mode==INPUT_PULLDOWN || mode== INPUT_PULLUP)
	{
		gpio[PIN2PORT(pin)]->PDDR &= MASKALL1S-(MASKPIN(pin));    		//gpio es un array de punteros a estructuras donde hay 5 structs (los puertos)
																			//shiftea el 1 la cantidad de pin y dsp lo resta con maskall1s para cambiar solo ese pin
	}
	else
	{
		gpio[PIN2PORT(pin)]->PDDR |= MASKPIN(pin);
	}
}


/**
 * @brief Configures how the pin reacts when an IRQ event ocurrs
 * @param pin the pin whose IRQ mode you wish to set (according PORTNUM2PIN)
 * @param irqMode disable, risingEdge, fallingEdge or bothEdges
 * @param irqFun function to call on pin event
 * @return Registration succeed
 */
bool gpioIRQ (pin_t pin, uint8_t irqMode, pinIrqFun_t irqFun) //irqFun lo manda el usuario (us). Se llama una sola vez, es para configurar pines de interrupci[on
{
	port[PIN2PORT(pin)]->PCR[PIN2NUM(pin)] &= (~PORT_PCR_IRQC_MASK);// Esta mascara es F00000 20 bits, 000F00000.
	switch (irqMode)
	{
		case GPIO_IRQ_MODE_DISABLE:
			return TRUE;
			break;
		case GPIO_IRQ_MODE_RISING_EDGE:
			port[PIN2PORT(pin)]->PCR[PIN2NUM(pin)] |= PORT_PCR_IRQC(RISING_EDGE_IRQ);
			break;
		case GPIO_IRQ_MODE_FALLING_EDGE:
			port[PIN2PORT(pin)]->PCR[PIN2NUM(pin)] |= PORT_PCR_IRQC(FALLING_EDGE_IRQ);
			break;
		case GPIO_IRQ_MODE_BOTH_EDGES:
			port[PIN2PORT(pin)]->PCR[PIN2NUM(pin)] |= PORT_PCR_IRQC(EITHER_EDGE_IRQ);
			break;
		default:
			return FALSE;
			break;
	}
	if ((irqFun != NULL) && ((irqMode == GPIO_IRQ_MODE_RISING_EDGE) || (irqMode == GPIO_IRQ_MODE_FALLING_EDGE) ||  (irqMode == GPIO_IRQ_MODE_BOTH_EDGES))){
		irqHandler[pin]=irqFun;
		switch (PIN2PORT(pin))
		{
			case PA:
				PortA_pines_irq[contadorA] = pin;
				contadorA++;
			break;
			case PB:
				PortB_pines_irq[contadorB] = pin;
				contadorB++;
			break;
			case PC:
				PortC_pines_irq[contadorC] = pin;
				contadorC++;
			break;
			case PD:
				PortD_pines_irq[contadorD] = pin;
				contadorD++;
			break;
			case PE:
				PortE_pines_irq[contadorE] = pin;
				contadorE++;
			break;
		}
		gpioClear_ISF(pin);
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/**
 * @brief Write a HIGH or a LOW value to a digital pin
 * @param pin the pin to write (according PORTNUM2PIN)
 * @param val Desired value (HIGH or LOW)
 */
void gpioWrite (pin_t pin, bool value)		//el pin tiene que ser un output, se cambia el pdor
											//no hace falta validar que es un output, xq sino es un registro q no se usa
{
	if (pin == PORTNUM2PIN(PC, 2))
	{
		while(1);
	}
		if(value==HIGH)
		{
			gpio[PIN2PORT(pin)]->PSOR |= MASKPIN(pin);
		}
		else
		{
			gpio[PIN2PORT(pin)]->PCOR |= MASKPIN(pin);	//CON EL CLEAR SE PONE EN 0!!!
		}
}

/**
 * @brief Toggle the value of a digital pin (HIGH<->LOW)
 * @param pin the pin to toggle (according PORTNUM2PIN)
 */
void gpioToggle (pin_t pin)
{
		gpio[PIN2PORT(pin)]->PTOR |= MASKPIN(pin);
}

/**
 * @brief Reads the value from a specified digital pin, either HIGH or LOW.
 * @param pin the pin to read (according PORTNUM2PIN)
 * @return HIGH or LOW
 */
bool gpioRead (pin_t pin)
{
		return ((gpio[PIN2PORT(pin)]->PDIR) & MASKPIN(pin));
}

bool gpioRead_ISF (pin_t pin)
{
	return (port[PIN2PORT(pin)]->PCR[PIN2NUM(pin)] & PORT_PCR_ISF_MASK); //aca solo
}

void gpioClear_ISF (pin_t pin)
{
    port[PIN2PORT(pin)]->ISFR = (1 << PIN2NUM(pin));
}

__ISR__ PORTA_IRQHandler() //el SW3 esta en el puerto A
{
	OSIntEnter();
	for (int i = 0; i < contadorA; i++){
		if (gpioRead_ISF(PortA_pines_irq[i])){
			 gpioClear_ISF(PortA_pines_irq[i]);
			 irqHandler[PA*32 + PIN2NUM(PortA_pines_irq[i])]();
			 break;
		}
	}
	OSIntExit();
}

__ISR__ PORTB_IRQHandler()
{
	OSIntEnter();

	gpioWrite(PIN_LED_EXAMEN1, HIGH);
	for (int i = 0; i < contadorB; i++){
		if (gpioRead_ISF(PortB_pines_irq[i])){
			 gpioClear_ISF(PortB_pines_irq[i]);
			 irqHandler[PB*32 + PIN2NUM(PortB_pines_irq[i])]();
			 break;
		}
	}
	gpioWrite(PIN_LED_EXAMEN1, LOW);
	OSIntExit();
}

__ISR__ PORTC_IRQHandler()
{
	OSIntEnter();

	if (gpioRead_ISF(PORTNUM2PIN(PC,2))){
		gpioClear_ISF(PORTNUM2PIN(PC,2));
		irqHandler[PC*32 + 2]();
	}
	OSIntExit();
}
	/*
	for (int i = 0; i < contadorC; i++){
		if (gpioRead_ISF(PortC_pines_irq[i])){
			 irqHandler[PC*32 + PIN2NUM(PortC_pines_irq[i])]();
			 gpioClear_ISF(PortC_pines_irq[i]);
			 break;
		}
	}
	*/



__ISR__ PORTD_IRQHandler()
{
	OSIntEnter();
	for (int i = 0; i < contadorD; i++){
		if (gpioRead_ISF(PortD_pines_irq[i])){
			 irqHandler[PD*32 + PIN2NUM(PortD_pines_irq[i])]();
			 gpioClear_ISF(PortD_pines_irq[i]);
			 break;
		}
	}
	OSIntExit();
}

__ISR__ PORTE_IRQHandler()
{
	OSIntEnter();
	for (int i = 0; i < contadorE; i++){
		if (gpioRead_ISF(PortE_pines_irq[i])){
			 irqHandler[PE*32 + PIN2NUM(PortE_pines_irq[i])]();
			 gpioClear_ISF(PortE_pines_irq[i]);
			 break;
		}
	}
	OSIntExit();
}
/******************************************************************************* */

