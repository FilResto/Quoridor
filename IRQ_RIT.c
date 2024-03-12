/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           IRQ_RIT.c
** Last modified Date:  2014-09-25
** Last Version:        V1.00
** Descriptions:        functions to manage T0 and T1 interrupts
** Correlated files:    RIT.h
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
#include "lpc17xx.h"
#include "RIT.h"
#include "quoridor.h"
//#include "../led/led.h", non usiamo led in scacchi
#define GRID_SIZE 7
#define PLAYER1 			0
#define PLAYER2 			1
#define HORIZONTAL 1
#define VERTICAL 2
/******************************************************************************
** Function name:		RIT_IRQHandler
**
** Descriptions:		REPETITIVE INTERRUPT TIMER handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/

/***
** SELECT PIN P1.25
** DOWN 	PIN P1.26
** LEFT		PIN P1.27
** RIGHT 	PIN P1.28
** UP 		PIN P1.29
***/
volatile int J_select=0, J_down=0, J_right=0, J_left=0, J_up=0;

void RIT_IRQHandler (void)
{	//voglio gestire la select e down
	if(currentMode==MODE_MOVE_TOKEN){
		if ((LPC_GPIO1->FIOPIN & (1 << 25)) == 0){ // select
			J_select++;
      if (J_select == 1) { // Confirm action only on the first press
          ConfirmAction();
        }
    } else {
        J_select = 0; // Reset counter when button is released
	}
	
	if((LPC_GPIO1->FIOPIN & (1<<26)) == 0){	//down
		J_down++;
		if(currentPlayer==PLAYER1){
			if(J_down > 0 && cursorRow_Player1 < GRID_SIZE - 1) {
			MoveCursorDown();
		}
	}else if (currentPlayer==PLAYER2){
		if(J_down > 0 && cursorRow_Player2 < GRID_SIZE - 1) {
			MoveCursorDown();
		}
	}
	}else{
		J_down=0;
	}
	
	if ((LPC_GPIO1->FIOPIN & (1 << 29)) == 0){ // up
		J_up++;
    if(currentPlayer==PLAYER1){
			if(J_up > 0 && cursorRow_Player1 > 0 ) {
			MoveCursorUp();
		}
	}else if (currentPlayer==PLAYER2){
		if(J_up > 0 && cursorRow_Player2 > 0) {
			MoveCursorUp();
		}
	}
    } else {
        J_up = 0;
		}
	
	if ((LPC_GPIO1->FIOPIN & (1 << 27)) == 0){ // left
		J_left++;
		if(currentPlayer==PLAYER1){
			if(J_left > 0 && cursorCol_Player2 > 0) {
			MoveCursorLeft();
		}
	}else if (currentPlayer==PLAYER2){
		if(J_left > 0 && cursorCol_Player2 > 0) {
			MoveCursorLeft();
		}
	}
    } else {
        J_left = 0;
	}
		
	if ((LPC_GPIO1->FIOPIN & (1 << 28)) == 0){ // right
		J_right++;
    if(currentPlayer==PLAYER1){
			if(J_right > 0 && cursorCol_Player1 < GRID_SIZE - 1) {
			MoveCursorRight();
		}
	}else if (currentPlayer==PLAYER2){
		if(J_right > 0 && cursorCol_Player2 < GRID_SIZE - 1) {
			MoveCursorRight();
		}
	}
    } else {
        J_right = 0;
	}
		
	
	
	
	}else if(currentMode==MODE_PLACE_BARRIER){//key1 11 e key2 12
			
			

			if ((LPC_GPIO1->FIOPIN & (1 << 25)) == 0){ // select
    					J_select++;
					if (J_select == 1) { // Confirm action only on the first press
							PlaceBarrierConfirmed();
					}
			} else {
					J_select = 0; // Reset counter when button is released
		}
		
		if((LPC_GPIO1->FIOPIN & (1<<26)) == 0){	//down
			J_down++;
			if(J_down > 0) {
				if((barrierOrientation == HORIZONTAL && barrierRow < GRID_SIZE - 1) || 
           (barrierOrientation == VERTICAL && barrierRow < GRID_SIZE-2)) {
							ClearPreviewBarrier();
							RedrawOverlappedBarriers13x13(barrierRow, barrierCol, barrierOrientation);
							barrierRow++;
							UpdatePreviewBarrier(barrierRow, barrierCol, barrierOrientation);
					 }	
					}
		}else{
			J_down=0;
		}
		
		if ((LPC_GPIO1->FIOPIN & (1 << 27)) == 0){ // left
			J_left++;
				if(J_left > 0 ) {
					if((barrierOrientation == HORIZONTAL && barrierCol > 0) || 
           (barrierOrientation == VERTICAL && barrierCol > 1)) {
							ClearPreviewBarrier();
					RedrawOverlappedBarriers13x13(barrierRow, barrierCol, barrierOrientation);
							barrierCol--;
							UpdatePreviewBarrier(barrierRow, barrierCol, barrierOrientation);
				}
			}
			} else {
					J_left = 0;
		}
			
		if ((LPC_GPIO1->FIOPIN & (1 << 28)) == 0){ // right 
			J_right++;
			if(J_right > 0 ) {
				if((barrierOrientation == HORIZONTAL && barrierCol < GRID_SIZE-2) || 
           (barrierOrientation == VERTICAL && barrierCol < GRID_SIZE-1)) {
							ClearPreviewBarrier();
				RedrawOverlappedBarriers13x13(barrierRow, barrierCol, barrierOrientation);
							barrierCol++;
							UpdatePreviewBarrier(barrierRow, barrierCol, barrierOrientation);
			}
		}
			} else {
					J_right = 0;
		}
			
		if ((LPC_GPIO1->FIOPIN & (1 << 29)) == 0){ // up
			J_up++;
			if(J_up > 0 ) {
				if((barrierOrientation == HORIZONTAL && barrierRow > 1) || 
           (barrierOrientation == VERTICAL && barrierRow > 0)) {
							ClearPreviewBarrier();
							RedrawOverlappedBarriers13x13(barrierRow, barrierCol, barrierOrientation);
							barrierRow--;
							UpdatePreviewBarrier(barrierRow, barrierCol, barrierOrientation);
					}
				}
			
			} else {
					J_up = 0;
			}
			//if ((LPC_GPIO2->FIOPIN & (1 << 12)) == 0)
				//downKey2++;
			//else
				//downKey2=0;
			
		}
	
	

	//HandleCursorMovement();
  LPC_RIT->RICTRL |= 0x1;	/* clear interrupt flag */
	LPC_RIT->RICOUNTER=0;//reset the RIT
	//reset_RIT();
	
  return;
}

/******************************************************************************
**                            End Of File
******************************************************************************/
