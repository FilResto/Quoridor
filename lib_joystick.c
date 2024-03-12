/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           joystick.h
** Last modified Date:  2018-12-30
** Last Version:        V1.00
** Descriptions:        Atomic joystick init functions
** Correlated files:    lib_joystick.c, funct_joystick.c
**--------------------------------------------------------------------------------------------------------       
*********************************************************************************************************/

#include "lpc17xx.h"
#include "joystick.h"

/*----------------------------------------------------------------------------
  Function that initializes joysticks and switch them off
 *----------------------------------------------------------------------------*/

void joystick_init(void) {

	/* joystick Select functionality */
	LPC_PINCON->PINSEL3 &= ~(3 << 18); // PIN mode GPIO (00b value per P1.25)
	LPC_GPIO1->FIODIR &= ~(1 << 25);   // P1.25 Input (joysticks on PORT1 defined as Input)
	
	/* joystick Down functionality */
  LPC_PINCON->PINSEL3 &= ~(3<<20);	//PIN mode GPIO (00b value per P1.26) // in
	LPC_GPIO1->FIODIR   &= ~(1<<26);	//P1.26 Input (joysticks on PORT1 defined as Input) in input sto modificando il pin 26

	/* joystick left functionality */
	LPC_PINCON->PINSEL3 &= ~(3 << 22); // PIN mode GPIO (00b value per P1.26)
	LPC_GPIO1->FIODIR &= ~(1 << 27);   // P1.27Input (joysticks on PORT1 defined as Input)

	/* joystick right functionality */
	LPC_PINCON->PINSEL3 &= ~(3 << 24); // PIN mode GPIO (00b value per P1.26)
	LPC_GPIO1->FIODIR &= ~(1 << 28);   // P1.28Input (joysticks on PORT1 defined as Input)
	
	/* joystick up functionality */
	LPC_PINCON->PINSEL3 &= ~(3 << 29); // PIN mode GPIO (00b value per P1.26)
	LPC_GPIO1->FIODIR &= ~(1 << 29);   // P1.29Input (joysticks on PORT1 defined as Input)
	
	
	
	//se vogliamo configurare tutto il joistick allora devo aggiungere le alte inizializzazioni per i pulsanti, guardare sul manuale e a lezione per capire i pin
}
