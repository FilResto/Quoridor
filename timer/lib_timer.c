/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           lib_timer.h
** Last modified Date:  2014-09-25
** Last Version:        V1.00
** Descriptions:        atomic functions to be used by higher sw levels
** Correlated files:    lib_timer.c, funct_timer.c, IRQ_timer.c
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
#include "lpc17xx.h"
#include "timer.h"

/******************************************************************************
** Function name:		enable_timer
**
** Descriptions:		Enable timer
**
** parameters:			timer number: 0 or 1
** Returned value:		None
**
******************************************************************************/
void enable_timer(uint8_t timer_num)
{
	if (timer_num == 0)
	{
		LPC_TIM0->TCR = 1;
	}
	else if (timer_num == 1)
	{
		LPC_TIM1->TCR = 1;
	}
	else if (timer_num == 2)
	{
		LPC_TIM2->TCR = 1;
	}
	else
	{
		LPC_TIM3->TCR = 1;
	}
	return;
}

/******************************************************************************
** Function name:		disable_timer
**
** Descriptions:		Disable timer
**
** parameters:			timer number: 0 or 1
** Returned value:		None
**
******************************************************************************/
void disable_timer( uint8_t timer_num )
{
  if ( timer_num == 0 )
  {
	LPC_TIM0->TCR = 0;
  }
  else
  {
	LPC_TIM1->TCR = 0;
  }
  return;
}

/******************************************************************************
** Function name:		reset_timer
**
** Descriptions:		Reset timer
**
** parameters:			timer number: 0 or 1
** Returned value:		None
**
******************************************************************************/
void reset_timer( uint8_t timer_num )
{
  uint32_t regVal;

  if ( timer_num == 0 )
  {
	regVal = LPC_TIM0->TCR;
	regVal |= 0x02;
	LPC_TIM0->TCR = regVal;
  }
  else if (timer_num == 1)
  {
	regVal = LPC_TIM1->TCR;
	regVal |= 0x02;
	LPC_TIM1->TCR = regVal;
  }
	else if (timer_num == 2)
	{
		regVal = LPC_TIM2->TCR;
		regVal |= 0x02;
		LPC_TIM2->TCR = 0;
	}
	else
	{
		regVal = LPC_TIM3->TCR;
		regVal |= 0x02;
		LPC_TIM3->TCR = 0;
	}
  return;
}

/******************************************************************************
** Function name:		init_timer
**
** Descriptions:		Init timer
**
** parameters:			
** Descriptions2:	Sets up a timer with a given TimerInterval, that will be loadeed into MatchRegister0 by default,
**									When the timer's counter matches the value of MatchReg0 it interrups.
										It only uses Match Register 0 for the timing event, and it's fixed to reset the timer and generate an interrupt when the match occurs
										It enables the timer's interrupt in the Nested Vectored Interrupt Controller (NVIC) and sets the priority
**
******************************************************************************/
uint32_t init_timer ( uint8_t timer_num, uint32_t TimerInterval )
{
  if ( timer_num == 0 )
  {
	LPC_TIM0->MR0 = TimerInterval;//prende l intervallo di temporizazione che ho scritto e lo salva nel match register 0, quindi non considera nessuno degli altri 3 match register


	LPC_TIM0->MCR = 3;

	NVIC_EnableIRQ(TIMER0_IRQn);//qui abilita le interruzioni del timer0
	//NVIC_SetPriority(TIMER0_IRQn, 0);		/* more priority than buttons */
	return (1);
  }
  else if ( timer_num == 1 )
  {
	LPC_TIM1->MR0 = TimerInterval;	
	LPC_TIM1->MCR = 3;				

	NVIC_EnableIRQ(TIMER1_IRQn);
	//NVIC_SetPriority(TIMER1_IRQn, 5);	/* less priority than buttons and timer0*/
	return (1);
  }
	else if ( timer_num == 2 )
  {
	LPC_TIM2->MR0 = TimerInterval;	
	LPC_TIM2->MCR = 3;				

	NVIC_EnableIRQ(TIMER2_IRQn);
	//NVIC_SetPriority(TIMER2_IRQn, 0);//max prio per timer2	
	return (1);
  }
	else if ( timer_num == 3 )
  {
	LPC_TIM3->MR0 = TimerInterval;	
	LPC_TIM3->MCR = 3;				

	NVIC_EnableIRQ(TIMER3_IRQn);
	//NVIC_SetPriority(TIMER3_IRQn, 1);	//meno prio di timer2 per timer3
	return (1);
  }
  return (0);
}
//In this version you can choose which match register you can use
/******************************************************************************
** Function name:		init_timer_complete
**
** Descriptions:		Init timer compete version
**
** parameters:			
** Descriptions2:	This function provides a more granular control over the timer's configuration.
									Prescaler: The prescaler value is used to divide the input clock before it's counted by the timer. 
														 This effectively slows down the timer's counting rate, allowing for longer intervals or finer resolution depending on the clock source.
									MatchReg:  This allows the user to select which match register (MR0, MR1, MR2, or MR3) to use.
														 This is useful when you want to have multiple timing events triggered from the same timer.
									SRImatchReg: This parameter sets the behavior when the timer matches the value in the selected match register. 
															 It can control whether an interrupt is generated (Interrupt on Match), whether the timer should reset (Reset on Match), and whether the timer should stop (Stop on Match).
									The << 3 * MatchReg operation is shifting the control bits to the correct position in the Match Control Register (MCR) corresponding to the selected match register.
**
******************************************************************************/
uint32_t init_timer_complete(uint8_t timer_num, uint32_t Prescaler, uint8_t MatchReg, uint8_t SRImatchReg, uint32_t TimerInterval)
{//example of call init_timer_complete(1, 0, 2, 3, 0x017D7840), timer 1 is configured, prescaler is disabled, match register2 is the one to use
	//SRImatchReg 3 is a bit mask 011 that means:
									//- Interrupt on Match (MRxI): is the I,If set, an interrupt is generated when the timer matches this register.
									//- Reset on Match (MRxR): is the R, If set, the timer counter will reset to 0 on a match.
									//- Stop on Match (MRxS): is the S, If set, the timer will stop on a match.
									// in mi case SRI, so i reset and interrupt at each match
									//						011
									//- TimerInterval (0x017D7840): This is the value to be set in the specified match register

	if (timer_num == 0)
	{
		LPC_TIM0->PR = Prescaler;

		if (MatchReg == 0)
		{
			LPC_TIM0->MR0 = TimerInterval;
			LPC_TIM0->MCR |= SRImatchReg << 3 * MatchReg;
		}
		else if (MatchReg == 1)
		{
			LPC_TIM0->MR1 = TimerInterval;
			LPC_TIM0->MCR |= SRImatchReg << 3 * MatchReg;
		}
		else if (MatchReg == 2)
		{
			LPC_TIM0->MR2 = TimerInterval;
			LPC_TIM0->MCR |= SRImatchReg << 3 * MatchReg;
		}
		else if (MatchReg == 3)
		{
			LPC_TIM0->MR3 = TimerInterval;
			LPC_TIM0->MCR |= SRImatchReg << 3 * MatchReg;
		}


		NVIC_EnableIRQ(TIMER0_IRQn);

		NVIC_SetPriority(TIMER0_IRQn, 1); 
		return (0);
	}
	/*else if (timer_num == 1)
	{

	NVIC_EnableIRQ(TIMER0_IRQn);
	
	NVIC_SetPriority(TIMER0_IRQn, 0);		// more priority than buttons 
	return (0);
  }
*/
  else if ( timer_num == 1 )
  {

		LPC_TIM1->PR = Prescaler;

		if (MatchReg == 0)
		{
			LPC_TIM1->MR0 = TimerInterval;
			LPC_TIM1->MCR |= SRImatchReg << 3 * MatchReg;
		}
		else if (MatchReg == 1)
		{
			LPC_TIM1->MR1 = TimerInterval;
			LPC_TIM1->MCR |= SRImatchReg << 3 * MatchReg;
		}
		else if (MatchReg == 2)
		{
			LPC_TIM1->MR2 = TimerInterval;
			LPC_TIM1->MCR |= SRImatchReg << 3 * MatchReg;
		}
		else if (MatchReg == 3)
		{
			LPC_TIM1->MR3 = TimerInterval;
			LPC_TIM1->MCR |= SRImatchReg << 3 * MatchReg;
		}
		NVIC_EnableIRQ(TIMER1_IRQn);
		
		NVIC_SetPriority(TIMER1_IRQn, 1); /* more priority than buttons */
		return (0);
	}
	else if (timer_num == 2)
	{
		LPC_TIM2->PR = Prescaler;

		if (MatchReg == 0)
		{
			LPC_TIM2->MR0 = TimerInterval;
			LPC_TIM2->MCR |= SRImatchReg << 3 * MatchReg;
		}
		else if (MatchReg == 1)
		{
			LPC_TIM2->MR1 = TimerInterval;
			LPC_TIM2->MCR |= SRImatchReg << 3 * MatchReg;
		}
		else if (MatchReg == 2)
		{
			LPC_TIM2->MR2 = TimerInterval;
			LPC_TIM2->MCR |= SRImatchReg << 3 * MatchReg;
		}
		else if (MatchReg == 3)
		{
			LPC_TIM2->MR3 = TimerInterval;
			LPC_TIM2->MCR |= SRImatchReg << 3 * MatchReg;
		}
		NVIC_EnableIRQ(TIMER2_IRQn);
		// NVIC_SetPriority(TIMER1_IRQn, 4);		/* less priority than buttons */
		NVIC_SetPriority(TIMER2_IRQn, 0); /* more priority than buttons */
		return (0);
	}
	else if (timer_num == 3)
	{
		LPC_TIM3->PR = Prescaler;

		if (MatchReg == 0)
		{
			LPC_TIM3->MR0 = TimerInterval;
			LPC_TIM3->MCR |= SRImatchReg << 3 * MatchReg;
		}
		else if (MatchReg == 1)
		{
			LPC_TIM3->MR1 = TimerInterval;
			LPC_TIM3->MCR |= SRImatchReg << 3 * MatchReg;
		}
		else if (MatchReg == 2)
		{
			LPC_TIM3->MR2 = TimerInterval;
			LPC_TIM3->MCR |= SRImatchReg << 3 * MatchReg;
		}
		else if (MatchReg == 3)
		{
			LPC_TIM3->MR3 = TimerInterval;
			LPC_TIM3->MCR |= SRImatchReg << 3 * MatchReg;
		}
		NVIC_EnableIRQ(TIMER3_IRQn);
		// NVIC_SetPriority(TIMER3_IRQn, 4);		/* less priority than buttons */
		NVIC_SetPriority(TIMER3_IRQn, 0); /* more priority than buttons */
		return (0);
	}
	return (1);
}

/******************************************************************************
**                            End Of File
******************************************************************************/
