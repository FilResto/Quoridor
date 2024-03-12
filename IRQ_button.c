#include "button.h"
#include "lpc17xx.h"
//#include "/timer/timer.h"
#include "quoridor.h"
#define HORIZONTAL 1
#define VERTICAL 2
#define PLAYER1 			0
#define PLAYER2 			1
//metti quando premo key1 and key2 nel rit. tipo down1 per key 1 abd down2 per key2. quando li premo li incremento e se sono != 0 nel rito faccio quello che devo fare
//cioè mettere la barriera e metterla in verticale, 12pin per key2 e 11 per key1
volatile int matchStarted=0; //flag to know if a match is started or ended
volatile int downKey1=0;
volatile int downKey2=0;
void EINT0_IRQHandler (void)	  	/* INT0														 */
{
	StartMatch();
	matchStarted=1;
	LPC_SC->EXTINT &= (1 << 0);     /* clear pending interrupt         */
}


void EINT1_IRQHandler (void)	  	/* KEY1														 */
{
	if(matchStarted){
		if(currentMode==MODE_MOVE_TOKEN){
			ClearTokenPreview();
			if((currentPlayer==PLAYER1 && barrierP1>0) || (currentPlayer == PLAYER2 && barrierP2 > 0)){
			currentMode=MODE_PLACE_BARRIER;
			DrawPreviewBarrier(barrierRow, barrierCol, barrierOrientation);
		} else{
				DisplayNoWallsAvailableMessage();
			}
	}else{
			currentMode=MODE_MOVE_TOKEN;
			//ClearPreviewBarrier(barrierRow, barrierCol, barrierOrientation);
		}
	}
	LPC_SC->EXTINT &= (1 << 1);     /* clear pending interrupt         */
}


void EINT2_IRQHandler (void)	  	/* KEY2														 */
{
	if(matchStarted){
		if(currentMode==MODE_PLACE_BARRIER){
			ClearPreviewBarrier();
			RedrawOverlappedBarriers13x13(barrierRow, barrierCol, barrierOrientation);
			barrierOrientation=(barrierOrientation==HORIZONTAL) ? VERTICAL : HORIZONTAL; //if horizontal put vertical, and viceversa, didnt want to #define the same data again
			UpdatePreviewBarrier(barrierRow, barrierCol, barrierOrientation);
		}
}
	
  LPC_SC->EXTINT &= (1 << 2);     /* clear pending interrupt         */    
}


