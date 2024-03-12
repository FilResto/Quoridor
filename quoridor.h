/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           quoridor.h
** Last modified Date:  2014-09-25
** Last Version:        V1.00
** Descriptions:        
** Correlated files:    
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
#ifndef __QUORIDOR_H
#define __QUORIDOR_H

#include "stdint.h" // for uint8_t, uint16_t types

// Function prototypes
void InitQuoridorGame(void);
void DrawToken(uint8_t player, uint16_t Col, uint16_t y);
void PlaceBarrier(uint8_t player, uint16_t row, uint16_t column, uint16_t orientation);
void StartMatch(void);
void PlaceInitialTokens(void);
void InitQuoridorGame(void);
void SwitchTurn(void);
void ConfirmAction(void);
void MoveToken(uint8_t player, int newX, int newY);
void ClearToken(uint8_t player, int Row, int Col);
void HandleCursorMovement(void);
void MoveCursorRight(void);
void MoveCursorLeft(void);
void MoveCursorUp(void);
void MoveCursorDown(void);
//variables
extern volatile int cursorCol_Player1, cursorRow_Player1;
extern volatile int cursorCol_Player2, cursorRow_Player2;
extern volatile int currentPlayer;
extern volatile int countdown;
extern volatile int matchStarted; //when P1 reaches row0 or P2 row6 I put it to 0
typedef enum {
    MODE_MOVE_TOKEN,
    MODE_PLACE_BARRIER
} OperationMode;
extern volatile OperationMode currentMode;
void PlaceBarrierConfirmed(void);
extern volatile int barrierRow, barrierCol, barrierOrientation;
void DrawPreviewBarrier(uint16_t row, uint16_t column, uint16_t orientation);
void ClearPreviewBarrier(void);
void UpdatePreviewBarrier(uint16_t row, uint16_t column, uint16_t orientation);
extern volatile int gameBoard7[7][7];
extern volatile int gameBoard13[13][13];
void RedrawAllBarriers(void);
int CheckWinner(void);
void EndMatch(int winner);
extern volatile uint8_t barrierP1;
extern volatile uint8_t barrierP2;
void DisplayNoWallsAvailableMessage(void);
void ClearTokenPreview(void);
void RedrawOverlappedBarriers(uint16_t row, uint16_t column, uint16_t orientation);
void DrawHalfBarrierLeft(uint16_t row, uint16_t column);

void DrawHalfBarrierRight(uint16_t row, uint16_t column); 

void DrawHalfBarrierUp(uint16_t row, uint16_t column);

void DrawHalfBarrierDown(uint16_t row, uint16_t column); 
void DrawFullBarrierHorizontal(uint16_t row, uint16_t column);
void DrawFullBarrierVertical(uint16_t row, uint16_t column);
void RedrawOverlappedBarriers13x13(int barrierRow, int barrierCol, int orientation);
int CheckPathForPlayer(int currentRow, int currentCol, int barrierPreviewRow, int barrierPreviewCol,int barrierOrientation, int goalRow);
void ClearPreviewBarrier(void);
uint32_t encodeMove(uint8_t playerID, uint8_t moveWallPlacement, uint8_t verticalHorizontal, uint8_t y, uint8_t x);
void ClearNoWallsAvailableMessage(void);
extern volatile int downKey1;
extern volatile int downKey2;

#endif 
