/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           quoridor.c
** Last modified Date:  2014-09-25
** Last Version:        V1.00
** Descriptions:        
** Correlated files:    
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
#include "quoridor.h"
#include "GLCD/GLCD.h" // Include your LCD header
#include "timer/timer.h"
#include "RIT.h"

#define PLAYER1_COLOR Blue
#define PLAYER2_COLOR Red
#define BARRIER_COLOR Black
#define PLAYER1 			0
#define PLAYER2 			1
#define MAX_BARRIERS 	8
#define BARRIER_LENGTH 29*2+4
#define GRID_SIZE     7
#define GRID_SIZE13   13
#define BARRIER_SPACE 4  // spazio barriere
#define CELL_HEIGHT      29
#define CELL_WIDTH       29
#define TOKEN_SIZE 15
#define BARRIER_NONE 0
#define BARRIER_MIDDLE 1
#define BARRIER_END 2
#define BARRIER_PRESENT 1
#define HORIZONTAL 1
#define VERTICAL 2
#define BOTH 3 //meaning that in that position there is a vertical and horizontal barrier
#define BARRIER_THICKNESS 3
#define PREVIEW_COLOR Green
#define MOVE_NONE  0
#define MOVE_UP    1
#define MOVE_DOWN  2
#define MOVE_LEFT  3
#define MOVE_RIGHT 4
#define HALF_BARRIER_SIZE (CELL_WIDTH + BARRIER_SPACE / 2)


volatile uint8_t barrierP1=MAX_BARRIERS;
volatile uint8_t barrierP2=MAX_BARRIERS;

volatile int currentPlayer=PLAYER1;

//cursorX_Player1 would represent the column (horizontal position) of Player 1's token
//cursorY_Player1 would represent the row (vertical position) of Player 1's token.
volatile int cursorCol_Player1 = 3, cursorRow_Player1 = 6; // Player 1 starts at row 6, col 3
volatile int cursorCol_Player2 = 3, cursorRow_Player2 = 0; // Player 2 starts at row 0, col 3
volatile int previewActive=0;

// Variable to keep track of the last movement direction
volatile int lastMoveDirection = MOVE_NONE;

volatile OperationMode currentMode = MODE_MOVE_TOKEN;//or MODE_PLACE_BARRIER
volatile int barrierRow=3, barrierCol=3, barrierOrientation=HORIZONTAL;
volatile int gameBoard7[7][7]={BARRIER_NONE};
volatile int gameBoard13[13][13];
volatile int previewBarrierActive=0;
int visited[GRID_SIZE13][GRID_SIZE13];
volatile uint32_t lastMoveEncoded=0;
uint32_t tokenOrBarrier=0; //0 player movement | 1 barrier placed
uint32_t finalBarrierOrientation=0;
uint32_t x, y;
int available_message=0;

//initialize the board
void InitQuoridorGame(void){
	DrawQuoridorBoard();
	DrawBarrierCounter(PLAYER1,barrierP1);
	DrawBarrierCounter(PLAYER2,barrierP2);
	PlaceInitialTokens();
	DrawTurnIndicator(currentPlayer);
	NVIC_DisableIRQ(EINT1_IRQn); //disabling interrupt from key1
	NVIC_DisableIRQ(EINT2_IRQn); // and key2
	//DrawFullBarrierHorizontal(5,5);
	//DrawFullBarrierVertical(2,2);
}


//draw player
// Call this function at the beginning of the game to place both tokens
void PlaceInitialTokens() {
    // Player 1 (White) starts at the bottom center of the grid
    DrawToken(PLAYER1, cursorRow_Player1, cursorCol_Player1); // Last row, center column

    // Player 2 (Red) starts at the top center of the grid
    DrawToken(PLAYER2, cursorRow_Player2, cursorCol_Player2); // First row, center column
}
void DrawToken(uint8_t player, uint16_t row, uint16_t column){
		uint16_t color = (player == PLAYER1) ? PLAYER1_COLOR : PLAYER2_COLOR;

    // Calculate the top-left corner of the cell where the token should be drawn
    uint16_t cellCol = BARRIER_SPACE + column * (CELL_WIDTH + BARRIER_SPACE) + (CELL_WIDTH - TOKEN_SIZE) / 2;
    uint16_t cellRow = BARRIER_SPACE + row * (CELL_HEIGHT + BARRIER_SPACE) + (CELL_HEIGHT - TOKEN_SIZE) / 2;

    // Assuming FillRectangle function fills the area including the starting point but not the ending point
    FillRectangle(cellCol, cellRow, cellCol + TOKEN_SIZE, cellRow + TOKEN_SIZE, color);
}
void DrawTokenWithColor(uint16_t row, uint16_t column, uint16_t color){

    // Calculate the top-left corner of the cell where the token should be drawn
    uint16_t cellCol = BARRIER_SPACE + column * (CELL_WIDTH + BARRIER_SPACE) + (CELL_WIDTH - TOKEN_SIZE) / 2;
    uint16_t cellRow = BARRIER_SPACE + row * (CELL_HEIGHT + BARRIER_SPACE) + (CELL_HEIGHT - TOKEN_SIZE) / 2;

    // Assuming FillRectangle function fills the area including the starting point but not the ending point
    FillRectangle(cellCol, cellRow, cellCol + TOKEN_SIZE, cellRow + TOKEN_SIZE, color);
}


// Start a new match
void StartMatch(void) {
	currentPlayer=PLAYER1;
  init_timer_complete(0, 0, 0, 3, 500000); 						    /* TurnTimer = 1 secondo */
	//init_timer_complete(0, 0, 0, 3, 25000000);//timer scheda
	enable_timer(0);
	init_RIT(0x004C4B40);
	//init_RIT(0x00BEBC20); //rit scheda
	enable_RIT();
	NVIC_EnableIRQ(EINT1_IRQn);//enable interrupt from key1
	NVIC_EnableIRQ(EINT2_IRQn);//and key2
	
	
}
void SwitchTurn(void){
	if(previewActive){
			ClearTokenPreview();
			lastMoveEncoded=encodeMove(currentPlayer,0,1,y,x);
	}
	if(previewBarrierActive){
			ClearPreviewBarrier();
		lastMoveEncoded=encodeMove(currentPlayer,0,1,y,x);
			previewBarrierActive=0;
			barrierCol=3;
			barrierOrientation=HORIZONTAL;
			barrierRow=3;
		if(currentMode==MODE_PLACE_BARRIER)
			currentMode=MODE_MOVE_TOKEN;
		}
	lastMoveEncoded=encodeMove(currentPlayer, tokenOrBarrier,finalBarrierOrientation,x,y);

	if(currentPlayer==PLAYER1)
		currentPlayer=PLAYER2;
	else
		currentPlayer=PLAYER1;
		
		DrawTurnIndicator(currentPlayer);
	  LPC_TIM0->TC=0;//reset the timer counter to 0
		enable_timer(0); //restart the timer0
		countdown=20;
		updateLCDTimer(countdown);
		if(available_message==1)
				ClearNoWallsAvailableMessage();
}

int CheckWinner(){
	if(cursorRow_Player1==0)//P1 won
			return 2;
	else if(cursorRow_Player2==6)
		return 3;
	return 0;
}
void EndMatch(int winner){
	//disabling all and showing the winner
  NVIC_DisableIRQ(EINT0_IRQn);
  NVIC_DisableIRQ(EINT1_IRQn);
  NVIC_DisableIRQ(EINT2_IRQn);
  disable_RIT();
	disable_timer(0);
	//LCD_Clear(White);
	if(winner==2)
		GUI_Text(75,120,(uint8_t *)"Player 1 Wins!", Blue, White);
	else if(winner==3)
		GUI_Text(75,150,(uint8_t *)"Player 2 Wins!", Red, White);
	matchStarted=0;//just to make sure if in some sort of way the button interrupts have not been disabled, this will make sure they dont work
	
}

void ConfirmAction() {
    if (previewActive) {
			int newRow, newCol; //variables for only move down and up for checking if i can jump the other player
			int winner;
			switch(lastMoveDirection){
						case MOVE_UP:
								newRow = (currentPlayer == PLAYER1) ? cursorRow_Player1 - 1 : cursorRow_Player2 - 1;
								newCol = (currentPlayer == PLAYER1) ? cursorCol_Player1 : cursorCol_Player2;
						//check on if I jump the other player
								if ((currentPlayer == PLAYER1 && cursorRow_Player2 == newRow && cursorCol_Player2 == newCol) ||
                    (currentPlayer == PLAYER2 && cursorRow_Player1 == newRow && cursorCol_Player1 == newCol)){
											newRow--;//Move up another row to jump over
										}
                MoveToken(currentPlayer, newRow, newCol);
										x=newRow;
										y=newCol;
										tokenOrBarrier=0;
										finalBarrierOrientation=0;
                break;
            case MOVE_DOWN:
								newRow = (currentPlayer == PLAYER1) ? cursorRow_Player1 + 1 : cursorRow_Player2 + 1;
                newCol = (currentPlayer == PLAYER1) ? cursorCol_Player1 : cursorCol_Player2;
                if ((currentPlayer == PLAYER1 && cursorRow_Player2 == newRow && cursorCol_Player2 == newCol) ||
                    (currentPlayer == PLAYER2 && cursorRow_Player1 == newRow && cursorCol_Player1 == newCol)) {
                    newRow++; // Move down another row to jump over
                }
                MoveToken(currentPlayer, newRow, newCol);
								x=newRow;
								y=newCol;
								tokenOrBarrier=0;
								finalBarrierOrientation=0;
                break;
            case MOVE_LEFT:
                MoveToken(currentPlayer, (currentPlayer == PLAYER1) ? cursorRow_Player1 : cursorRow_Player2, 
                                      (currentPlayer == PLAYER1) ? cursorCol_Player1 - 1 : cursorCol_Player2 - 1);
						x=newRow;
						y=newCol;
						tokenOrBarrier=0;
						finalBarrierOrientation=0;
                break;
            case MOVE_RIGHT:
                MoveToken(currentPlayer, (currentPlayer == PLAYER1) ? cursorRow_Player1 : cursorRow_Player2, 
                                      (currentPlayer == PLAYER1) ? cursorCol_Player1 + 1 : cursorCol_Player2 + 1);
						tokenOrBarrier=0;
						x=newRow;
						y=newCol;
						finalBarrierOrientation=0;
                break;
            default:
                // Handle no movement or invalid direction if necessary
                break;
			}
        
        previewActive = 0; // Reset preview flag
				lastMoveDirection = MOVE_NONE;
				//check for the winner
				winner=CheckWinner();
				if(winner==2 || winner==3)
					EndMatch(winner);//match ended
				else	
					SwitchTurn(); // Switch turns
    }
}
void CancelMove() {//never used, maybe use it somehow
    if (previewActive) {
        if (currentPlayer == PLAYER1) {
            DrawToken(PLAYER1, cursorRow_Player1, cursorCol_Player1); // Redraw the original token
        } else {
            DrawToken(PLAYER2, cursorRow_Player2, cursorCol_Player2); // Redraw the original token
        }
        previewActive = 0; // Reset preview flag
    }
}

// Function to clear a token (essentially redraw the cell without the token)
void ClearToken(uint8_t player, int Row, int Col) {
    // Calculate the top-left corner of the cell
		uint16_t cellRow = BARRIER_SPACE + Row * (CELL_HEIGHT + BARRIER_SPACE);
    uint16_t cellCol = BARRIER_SPACE + Col * (CELL_WIDTH + BARRIER_SPACE);

    // Redraw the cell
    // Assuming DrawCell function draws an empty cell
    DrawCell(cellRow, cellCol);//swappare prima row e poi col
}
// Move a player's token to a new location
void MoveToken(uint8_t player, int newRow, int newCol) {//metteer a posto
    // Clear token from old position
	if(player==PLAYER1){
		ClearToken(player, cursorRow_Player1, cursorCol_Player1);
		// Update token position
		cursorRow_Player1=newRow;
		cursorCol_Player1=newCol;
		// Draw token at new position
    DrawToken(player, newRow, newCol);
	} 
	else if(player==PLAYER2){
		ClearToken(player, cursorRow_Player2, cursorCol_Player2);
		// Update token position
		cursorRow_Player2=newRow;
		cursorCol_Player2=newCol;
		// Draw token at new position
    DrawToken(player, newRow, newCol);
	}
}

void DrawPreviewToken(uint8_t player, uint16_t row, uint16_t column) {
    uint16_t color = PREVIEW_COLOR;
    uint16_t cellCol = BARRIER_SPACE + column * (CELL_WIDTH + BARRIER_SPACE) + (CELL_WIDTH - TOKEN_SIZE) / 2;
    uint16_t cellRow = BARRIER_SPACE + row * (CELL_HEIGHT + BARRIER_SPACE) + (CELL_HEIGHT - TOKEN_SIZE) / 2;
    FillRectangle(cellCol, cellRow, cellCol + TOKEN_SIZE, cellRow + TOKEN_SIZE, color);
}
void ClearTokenPreview() {//not complete
    if (previewActive) {
        uint16_t currentRow = (currentPlayer == PLAYER1) ? cursorRow_Player1 : cursorRow_Player2;
        uint16_t currentCol = (currentPlayer == PLAYER1) ? cursorCol_Player1 : cursorCol_Player2;

        // Adjust the row and column based on the last move direction
        switch (lastMoveDirection) {
            case MOVE_UP:
                currentRow = (currentRow > 0) ? currentRow - 1 : currentRow;
								//se tipo sono p1 e ora current row ha la stesso valore di rowp2, allora devo fare current row ancora meno1
								if(currentPlayer==PLAYER1 && currentRow==cursorRow_Player2)
										currentRow--;
								else if(currentPlayer==PLAYER2 && currentRow==cursorRow_Player1)
										currentRow--;
                break;
            case MOVE_DOWN:
                currentRow = (currentRow < GRID_SIZE - 1) ? currentRow + 1 : currentRow;
								if(currentPlayer==PLAYER1 && currentRow==cursorRow_Player2)
										currentRow++;
								else if(currentPlayer==PLAYER2 && currentRow==cursorRow_Player1)
										currentRow++;
                break;
                break;
            case MOVE_LEFT:
                currentCol = (currentCol < GRID_SIZE - 1) ? currentCol - 1 : currentCol;
                break;
            case MOVE_RIGHT:
                currentCol = (currentCol > 0) ? currentCol +1 : currentCol;
                break;
            default:
                // No movement or invalid direction, no need to adjust the position
                break;
        }

        // Clear the preview token for the current player
        ClearToken(currentPlayer, currentRow, currentCol);
        
        // Reset previewActive flag and lastMoveDirection
        previewActive = 0;
        lastMoveDirection = MOVE_NONE;
    }
}


void MoveCursorRight(void) {
		if(previewActive)
				ClearTokenPreview();
    if (!previewActive) {
        // Calculate the next column in the 13x13 matrix
				int nextCol_Player1 = (cursorCol_Player1 + 1) * 2;
        int nextCol_Player2 = (cursorCol_Player2 + 1) * 2;
        int row1 = cursorRow_Player1 * 2;
        int row2 = cursorRow_Player2 * 2;
				int barrierCol_Player1 = (cursorCol_Player1 * 2) + 1; // The odd column index to the right
        int barrierCol_Player2 = (cursorCol_Player2 * 2) + 1; // The odd column index to the right
        if (currentPlayer == PLAYER1 && nextCol_Player1 < GRID_SIZE13) {
            // Check for a vertical barrier on the right side of the current position
            if (gameBoard13[row1][barrierCol_Player1] != BARRIER_END && gameBoard13[row1][barrierCol_Player1] != BARRIER_MIDDLE) {
                // Show preview token at the calculated position
                DrawPreviewToken(PLAYER1, cursorRow_Player1, cursorCol_Player1+1); // Show preview at the 7x7 position
                lastMoveDirection = MOVE_RIGHT;
                previewActive = 1;
            }
        } else if (currentPlayer == PLAYER2 && nextCol_Player2 < GRID_SIZE13) {
            // Check for a vertical barrier on the right side of the current position
            if (gameBoard13[row2][barrierCol_Player2] != BARRIER_END && gameBoard13[row2][barrierCol_Player2] != BARRIER_MIDDLE) {
                // Show preview token at the calculated position
                DrawPreviewToken(PLAYER2, cursorRow_Player2, cursorCol_Player2+1); // Show preview at the 7x7 position
                lastMoveDirection = MOVE_RIGHT;
                previewActive = 1;
            }
        }
    }
}




void MoveCursorLeft(void) {
		if(previewActive)
				ClearTokenPreview();
    if (!previewActive) {
        // Calculate the previous column in the 13x13 matrix
        int prevCol_Player1 = (cursorCol_Player1 - 1) * 2;
        int prevCol_Player2 = (cursorCol_Player2 - 1) * 2;
        int row1 = cursorRow_Player1 * 2;
        int row2 = cursorRow_Player2 * 2;
        int barrierCol_Player1 = (cursorCol_Player1 * 2) - 1; // The odd column index to the left
        int barrierCol_Player2 = (cursorCol_Player2 * 2) - 1; // The odd column index to the left
        if (currentPlayer == PLAYER1 && prevCol_Player1 >= 0) {
            // Check for a vertical barrier on the left side of the current position
            if (gameBoard13[row1][barrierCol_Player1] != BARRIER_END && gameBoard13[row1][barrierCol_Player1] != BARRIER_MIDDLE) {
                DrawPreviewToken(PLAYER1, cursorRow_Player1, cursorCol_Player1-1 ); // Show preview at 7x7 position
                lastMoveDirection = MOVE_LEFT;
                previewActive = 1;
            }
        } else if (currentPlayer == PLAYER2 && prevCol_Player2 >= 0) {
            // Check for a vertical barrier on the left side of the current position
            if (gameBoard13[row2][barrierCol_Player2] != BARRIER_END && gameBoard13[row2][barrierCol_Player2] != BARRIER_MIDDLE ) {
                DrawPreviewToken(PLAYER2, cursorRow_Player2, cursorCol_Player2-1 ); // Show preview at 7x7 position
                lastMoveDirection = MOVE_LEFT;
                previewActive = 1;
            }
        }
    }
}


//BARRIER_NONE 0
//HORIZONTAL 1
//VERTICAL 2
//barrier is positioned horizontally in the same cell of the player, so for the Down we have to check the next cell, for Up only the current one
void MoveCursorUp(void) {
		if(previewActive)
				ClearTokenPreview();
    if (!previewActive) {
        // Convert current row and col to 13x13 grid coordinates
        int currentRow_Player1 = cursorRow_Player1 * 2 ;
        int currentRow_Player2 = cursorRow_Player2 * 2 ;
        int currentCol_Player1 = cursorCol_Player1 * 2 ;
        int currentCol_Player2 = cursorCol_Player2 * 2 ;

        // Calculate positions to check for barriers and players
        int rowAbove_Player1 = currentRow_Player1 - 2; // The row above Player 1
        int rowAbove_Player2 = currentRow_Player2 - 2; // The row above Player 2
        int twoRowsAbove_Player1 = currentRow_Player1 - 4; // Two rows above Player 1
        int twoRowsAbove_Player2 = currentRow_Player2 - 4; // Two rows above Player 2

        if (currentPlayer == PLAYER1 && rowAbove_Player1 >= 0) {
            if (twoRowsAbove_Player1>=0 && rowAbove_Player1==currentRow_Player2 && currentCol_Player1==currentCol_Player2 &&
                gameBoard13[currentRow_Player1-1][currentCol_Player1] != BARRIER_END && gameBoard13[currentRow_Player1-1][currentCol_Player1] != BARRIER_MIDDLE &&
                gameBoard13[currentRow_Player1-3][currentCol_Player1] != BARRIER_END && gameBoard13[currentRow_Player1-3][currentCol_Player1] != BARRIER_MIDDLE) {
                // Double jump over Player 2
                DrawPreviewToken(PLAYER1, twoRowsAbove_Player1 /2, currentCol_Player1 / 2 );
                lastMoveDirection = MOVE_UP;
            } else if (gameBoard13[currentRow_Player1-1][currentCol_Player1] != BARRIER_END && gameBoard13[currentRow_Player1-1][currentCol_Player1] != BARRIER_MIDDLE && rowAbove_Player1==currentRow_Player2 && currentCol_Player1==currentCol_Player2) {
                // case barrier over P2, so cant jump or normal jump
                
            }else if(gameBoard13[currentRow_Player1-1][currentCol_Player1] != BARRIER_END && gameBoard13[currentRow_Player1-1][currentCol_Player1] != BARRIER_MIDDLE){
							DrawPreviewToken(PLAYER1, rowAbove_Player1 / 2 , currentCol_Player1 / 2 );
              lastMoveDirection = MOVE_UP;
						}
        } else if (currentPlayer == PLAYER2 && rowAbove_Player2 >= 0) {
            if (twoRowsAbove_Player2>=0 && rowAbove_Player2==currentRow_Player1 && currentCol_Player2==currentCol_Player1 &&
                gameBoard13[currentRow_Player2-1][currentCol_Player2] != BARRIER_END && gameBoard13[currentRow_Player2-1][currentCol_Player2] != BARRIER_MIDDLE &&
                gameBoard13[currentRow_Player2-3][currentCol_Player2] != BARRIER_END && gameBoard13[currentRow_Player2-3][currentCol_Player2] != BARRIER_MIDDLE) {
                // Double jump over Player 1
                DrawPreviewToken(PLAYER2, twoRowsAbove_Player2 /2, currentCol_Player2 / 2 );
                lastMoveDirection = MOVE_UP;
            } else if (gameBoard13[currentRow_Player2-1][currentCol_Player2] != BARRIER_END && gameBoard13[currentRow_Player2-1][currentCol_Player2] != BARRIER_MIDDLE && rowAbove_Player2==currentRow_Player1 && currentCol_Player2==currentCol_Player1) {
                // case barrier over P1, so cant jump or normal jump
                
            } else if (gameBoard13[currentRow_Player2-1][currentCol_Player2] != BARRIER_END && gameBoard13[currentRow_Player2-1][currentCol_Player2] != BARRIER_MIDDLE) {
                // Normal jump
                DrawPreviewToken(PLAYER1, rowAbove_Player2 / 2 , currentCol_Player2 / 2 );
                lastMoveDirection = MOVE_UP;
            }
        }
        previewActive = (lastMoveDirection == MOVE_UP);
    }
}



void MoveCursorDown(void) {
		if(previewActive)
				ClearTokenPreview();
    if (!previewActive) {
        // Convert current row and col to 13x13 grid coordinates
        int currentRow_Player1 = cursorRow_Player1 * 2 ;
        int currentRow_Player2 = cursorRow_Player2 * 2 ;
        int currentCol_Player1 = cursorCol_Player1 * 2 ;
        int currentCol_Player2 = cursorCol_Player2 * 2 ;

        // Calculate positions to check for barriers and players
        int rowUnder_Player1 = currentRow_Player1 + 2; // The row above Player 1
        int rowUnder_Player2 = currentRow_Player2 + 2; // The row above Player 2
        int twoRowsUnder_Player1 = currentRow_Player1 + 4; // Two rows above Player 1
        int twoRowsUnder_Player2 = currentRow_Player2 + 4; // Two rows above Player 2

        if (currentPlayer == PLAYER1 && rowUnder_Player1 < GRID_SIZE13) {
            if (twoRowsUnder_Player1<GRID_SIZE13 && rowUnder_Player1==currentRow_Player2 && currentCol_Player2==currentCol_Player1 &&
                gameBoard13[currentRow_Player1+1][currentCol_Player1] != BARRIER_END && gameBoard13[currentRow_Player1+1][currentCol_Player1] != BARRIER_MIDDLE &&
                gameBoard13[currentRow_Player1+3][currentCol_Player1] != BARRIER_END && gameBoard13[currentRow_Player1+3][currentCol_Player1] != BARRIER_MIDDLE) {
                // Double jump under Player 1
                DrawPreviewToken(PLAYER1, twoRowsUnder_Player1 /2, currentCol_Player1 / 2 );
                lastMoveDirection = MOVE_DOWN;
            } else if (gameBoard13[currentRow_Player1+1][currentCol_Player1] != BARRIER_END && gameBoard13[currentRow_Player1+1][currentCol_Player1] != BARRIER_MIDDLE && rowUnder_Player1==currentRow_Player2 && currentCol_Player1==currentCol_Player2) {
                // case barrier under P1, so cant jump or normal jump
                
            }else if(gameBoard13[currentRow_Player1+1][currentCol_Player1] != BARRIER_END && gameBoard13[currentRow_Player1+1][currentCol_Player1] != BARRIER_MIDDLE){
							DrawPreviewToken(PLAYER1, rowUnder_Player1 / 2 , currentCol_Player1 / 2 );
              lastMoveDirection = MOVE_DOWN;
						}
        } else if (currentPlayer == PLAYER2 && rowUnder_Player2 <GRID_SIZE13) {
            if (twoRowsUnder_Player2<GRID_SIZE13 && rowUnder_Player2==currentRow_Player1 && currentCol_Player2==currentCol_Player1 &&
                gameBoard13[currentRow_Player2+1][currentCol_Player2] != BARRIER_END && gameBoard13[currentRow_Player2+1][currentCol_Player2] != BARRIER_MIDDLE &&
                gameBoard13[currentRow_Player2+3][currentCol_Player2] != BARRIER_END && gameBoard13[currentRow_Player2+3][currentCol_Player2] != BARRIER_MIDDLE) {
                // Double jump under Player 2
                DrawPreviewToken(PLAYER2, twoRowsUnder_Player2 /2, currentCol_Player2 / 2 );
                lastMoveDirection = MOVE_DOWN;
            }else if (gameBoard13[currentRow_Player2+1][currentCol_Player2] != BARRIER_END && gameBoard13[currentRow_Player2+1][currentCol_Player2] != BARRIER_MIDDLE && rowUnder_Player2==currentRow_Player1 && currentCol_Player2==currentCol_Player1) {
                // case barrier under P2, so cant jump or normal jump
                
            } else if (gameBoard13[currentRow_Player2+1][currentCol_Player2] != BARRIER_END && gameBoard13[currentRow_Player2+1][currentCol_Player2] != BARRIER_MIDDLE ) {
                // Normal jump
                DrawPreviewToken(PLAYER1, rowUnder_Player2 / 2 , currentCol_Player2 / 2 );
                lastMoveDirection = MOVE_DOWN;
            }
        }
        previewActive = (lastMoveDirection == MOVE_DOWN);
    }
}

// Function to copy the game board and add the preview barrier

// Recursive function to check if there's a path for the player
/*int FindPath(int row, int col, int goalRow, int visited[GRID_SIZE13][GRID_SIZE13]) {
    // Check bounds
    if (row < 0 || row >= GRID_SIZE13 || col < 0 || col >= GRID_SIZE13) {
        return 0;
    }
    // Check if the current cell has already been visited
    if (visited[row][col]) {
        return 0;
    }
    // Mark the current cell as visited
    visited[row][col] = 1;
    // Check if the goal row is reached
    if (row == goalRow) {
        return 1;
    }
    
    // Check all four possible movements (up, down, left, right)
    // Adjusted to check the cell adjacent to the player for barriers
    if (row - 2 >= 0 && (col % 2 == 0 || gameBoard13[row - 1][col] != BARRIER_MIDDLE) && FindPath(row - 2, col, goalRow, visited)) { // Up
        return 1;
    }
    if (row + 2 < GRID_SIZE13 && (col % 2 == 0 || gameBoard13[row + 1][col] != BARRIER_MIDDLE) && FindPath(row + 2, col, goalRow, visited)) { // Down
        return 1;
    }
    if (col - 2 >= 0 && (row % 2 == 0 || gameBoard13[row][col - 1] != BARRIER_MIDDLE) && FindPath(row, col - 2, goalRow, visited)) { // Left
        return 1;
    }
    if (col + 2 < GRID_SIZE13 && (row % 2 == 0 || gameBoard13[row][col + 1] != BARRIER_MIDDLE) && FindPath(row, col + 2, goalRow, visited)) { // Right
        return 1;
    }
    
    // Unmark the current cell before backtracking
    visited[row][col] = 0;
    // If no path is found, return 0
    return 0;
}
*/


int FindPath(int row, int col, int goalRow, int visited[GRID_SIZE13][GRID_SIZE13]) {
    // Base Case: Check bounds
    if (row < 0 || row >= GRID_SIZE13 || col < 0 || col >= GRID_SIZE13) {
        return 0;
    }

    // Check if the current cell is a barrier or already visited
    if (visited[row][col]) {
        return 0;
    }

    // Mark as visited
    visited[row][col] = 1;

    // Base Case: Check if the goal row is reached
    if (row == goalRow) {
        return 1;
    }

    // Recursive Case: Check all four directions (considering barriers)
    if ((row - 2 >= 0 && gameBoard13[row-1][col] == BARRIER_NONE && FindPath(row - 2, col, goalRow, visited)) || // Up
        (row + 2 < GRID_SIZE13 && gameBoard13[row+1][col] == BARRIER_NONE && FindPath(row + 2, col, goalRow, visited)) || // Down
        (col - 2 >= 0 && gameBoard13[row][col-1] == BARRIER_NONE && FindPath(row, col - 2, goalRow, visited)) || // Left
        (col + 2 < GRID_SIZE13 && gameBoard13[row][col+1] == BARRIER_NONE && FindPath(row, col + 2, goalRow, visited))) { // Right
        return 1;
    }

    // No path found from this cell, backtrack by resetting visited state
    //visited[row][col] = 0;
    return 0;
}


// Pseudocode for BFS path checking
int CheckPathForPlayer(int currentRow, int currentCol, int barrierPreviewRow, int barrierPreviewCol,int barrierOrientation, int goalRow) {
    //adding the previewbarrier into the gameboeard13x13 new
		int i,j;
		for(i=0;i<GRID_SIZE13 ;i++){
			for(j=0;j<GRID_SIZE13 ;j++){
				visited[i][j]=0;
			}
		}
		
    // If the goalRow is not reached, return 0 (false)
    return FindPath(currentRow,currentCol,goalRow,visited);
}

int IsValidBarrierPosition(int gridRow, int gridCol, int orientation) {
	//gridrow and gridcol are already in 13x13
	int pathExistsP1, pathExistsP2, p1Row, p1Col, p2Row, p2Col;
	// Convert 7x7 grid coordinates to 13x13 grid coordinates
    //gridRow = (gridRow * 2) - 1;
    //gridCol = (gridCol * 2) - 1;
		
    if (orientation == HORIZONTAL) {
        // Horizontal placement. Check the positions immediately adjacent to the barrier ends.
        if ((gridCol > 0 && gameBoard13[gridRow][gridCol - 1] == BARRIER_MIDDLE) ||  // Check left side of the barrier
            (gridCol + 3 < GRID_SIZE13 && gameBoard13[gridRow][gridCol + 4] == BARRIER_MIDDLE)||
						(gridCol + 2 < GRID_SIZE13 && gameBoard13[gridRow][gridCol + 2] == BARRIER_MIDDLE)||
						(gridCol + 1 < GRID_SIZE13 && gameBoard13[gridRow][gridCol + 1] == BARRIER_MIDDLE)) {  // Check right side of the barrier
            return 0;  // Invalid position due to a potential '+' formation
        }
    } else {
        // Vertical placement. Check the positions immediately adjacent to the barrier ends.
        if ((gridRow > 0 && gameBoard13[gridRow - 1][gridCol] == BARRIER_MIDDLE) ||  // Check top side of the barrier
            (gridRow + 3 < GRID_SIZE13 && gameBoard13[gridRow + 4][gridCol] == BARRIER_MIDDLE)||
						(gridRow + 1 < GRID_SIZE13 && gameBoard13[gridRow + 1][gridCol] == BARRIER_MIDDLE) ||
						(gridRow + 2 < GRID_SIZE13 && gameBoard13[gridRow + 2][gridCol] == BARRIER_MIDDLE)) {  // Check bottom side of the barrier
            return 0;  // Invalid position due to a potential '+' formation
        }
    }
		p1Row = cursorRow_Player1*2;
		p1Col = cursorCol_Player1*2;
		p2Row = cursorRow_Player2*2;
		p2Col = cursorCol_Player1*2;
		if (barrierOrientation == HORIZONTAL) {
        gameBoard13[gridRow][gridCol] = BARRIER_END;
        gameBoard13[gridRow][gridCol + 1] = BARRIER_MIDDLE;
				gameBoard13[gridRow][gridCol + 2] = BARRIER_MIDDLE;
				gameBoard13[gridRow][gridCol + 3] = BARRIER_END;
    } else { // VERTICAL
        gameBoard13[gridRow][gridCol] = BARRIER_END;
        gameBoard13[gridRow + 1][gridCol] = BARRIER_MIDDLE;
				gameBoard13[gridRow+2][gridCol] = BARRIER_MIDDLE;
        gameBoard13[gridRow + 3][gridCol] = BARRIER_END;
    }
		// Check path for Player 1 from their current position to the goal row
    pathExistsP1 = CheckPathForPlayer(p1Row, p1Col,gridRow, gridCol, barrierOrientation, 0);
    
    //in order to save memory and time execution, I call it on P2 only if on P1 is succesfully done
		// Check path for Player 2 only if Player 1's path exists
    if (pathExistsP1) {
        pathExistsP2 = CheckPathForPlayer(p2Row, p2Col, gridRow, gridCol, barrierOrientation, 12);
    } else {
        pathExistsP2 = 0; // Since Player 1's path doesn't exist, don't need to check for Player 2
    }
		if (barrierOrientation == HORIZONTAL) {
        gameBoard13[gridRow][gridCol] = BARRIER_NONE;
        gameBoard13[gridRow][gridCol + 1] = BARRIER_NONE;
				gameBoard13[gridRow][gridCol + 2] = BARRIER_NONE;
				gameBoard13[gridRow][gridCol + 3] = BARRIER_NONE;
    } else { // VERTICAL
        gameBoard13[gridRow][gridCol] = BARRIER_NONE;
        gameBoard13[gridRow + 1][gridCol] = BARRIER_NONE;
				gameBoard13[gridRow+2][gridCol] = BARRIER_NONE;
        gameBoard13[gridRow + 3][gridCol] = BARRIER_NONE;
    }
    return pathExistsP1 && pathExistsP2;  // Valid position as no '+' formation is detected and if path is possible for both of them
}


int CanPlaceHorizontalBarrier(int gridRow, int gridCol) {
    // Check that the start position is either BARRIER_NONE or BARRIER_END
    // and that the middle positions are BARRIER_NONE
    return (gameBoard13[gridRow][gridCol] == BARRIER_NONE || gameBoard13[gridRow][gridCol] == BARRIER_END) &&
           gameBoard13[gridRow][gridCol + 1] == BARRIER_NONE &&//usuless, cuz this is the player col (we have a player on the up or down
           (gameBoard13[gridRow][gridCol + 2] == BARRIER_NONE || gameBoard13[gridRow][gridCol + 2] == BARRIER_END) &&
           (gameBoard13[gridRow][gridCol + 3] == BARRIER_NONE || gameBoard13[gridRow][gridCol + 3] == BARRIER_END);
}

int CanPlaceVerticalBarrier(int gridRow, int gridCol) {
    // Similar logic to horizontal barrier placement
    return (gameBoard13[gridRow][gridCol] == BARRIER_NONE || gameBoard13[gridRow][gridCol] == BARRIER_END || gameBoard13[gridRow][gridCol] == BARRIER_MIDDLE) &&
           gameBoard13[gridRow + 1][gridCol] == BARRIER_NONE &&//usuless, cuz this is the player row (we have a player on the right or left
           (gameBoard13[gridRow + 2][gridCol] == BARRIER_NONE || gameBoard13[gridRow + 2][gridCol] == BARRIER_END )&&
           (gameBoard13[gridRow + 3][gridCol] == BARRIER_NONE || gameBoard13[gridRow + 3][gridCol] == BARRIER_END || gameBoard13[gridRow+3][gridCol] == BARRIER_MIDDLE);
}
// Place a barrier on the board
void PlaceBarrier(uint8_t player, uint16_t row, uint16_t column, uint16_t orientation) {
    // Convert grid coordinates to pixel coordinates for the start of the barrier
    uint16_t startCol = BARRIER_SPACE + column * (CELL_WIDTH + BARRIER_SPACE);
    uint16_t startRow = BARRIER_SPACE + row * (CELL_HEIGHT + BARRIER_SPACE);
		// Adjust coordinates for the 13x13 grid
    
		int i;
    // Convert 7x7 grid coordinates to 13x13 grid coordinates
    int gridRow = (row * 2) - 1;
    int gridCol = (column * 2) - 1;

    // Adjust pixel coordinates for the start of the barrier on the LCD

    // Check if a barrier can be placed and draw it on the 13x13 grid and LCD
    if (orientation==HORIZONTAL && CanPlaceHorizontalBarrier(gridRow, gridCol) && gridCol < GRID_SIZE13 - 1  ) {
        // Check for horizontal barrier placement, forse mettere <gridsize13 -1 cosi arriva a 12
						gameBoard13[gridRow][gridCol] = BARRIER_END;
            gameBoard13[gridRow][gridCol + 1] = BARRIER_MIDDLE;
            gameBoard13[gridRow][gridCol + 2] = BARRIER_MIDDLE;
            gameBoard13[gridRow][gridCol + 3] = BARRIER_END;
            // Draw the barrier horizontally on the LCD
            startRow -= BARRIER_THICKNESS / 2;
            for ( i = 0; i < BARRIER_THICKNESS; i++) {
            LCD_DrawLine(startCol, startRow - i, startCol + BARRIER_LENGTH, startRow - i, BARRIER_COLOR);
						}
										// Update the barrier count for the player
						if (player == PLAYER1) {
								barrierP1--;
								DrawBarrierCounter(PLAYER1, barrierP1);
						} else if (player == PLAYER2) {
								barrierP2--;
								DrawBarrierCounter(PLAYER2, barrierP2);
						}
				previewBarrierActive = 0;
				x=barrierRow; //is for 7x7, for 13x13 use gridRow and gridCol
				y=barrierCol;
				barrierRow=3;
				barrierCol=3;
				barrierOrientation=HORIZONTAL;
        currentMode = MODE_MOVE_TOKEN; // Switch back to token movement mode
				SwitchTurn();
        
    } else if(orientation==VERTICAL){
        // Check for vertical barrier placement
        if (CanPlaceVerticalBarrier(gridRow, gridCol) && gridCol < GRID_SIZE13 - 1) {
						
            gameBoard13[gridRow][gridCol] = BARRIER_END;
            gameBoard13[gridRow + 1][gridCol] = BARRIER_MIDDLE;
            gameBoard13[gridRow + 2][gridCol] = BARRIER_MIDDLE;
            gameBoard13[gridRow + 3][gridCol] = BARRIER_END;
            // Draw the barrier vertically on the LCD
            startCol -= BARRIER_THICKNESS / 2;
            for ( i = 0; i < BARRIER_THICKNESS; i++) {
            LCD_DrawLine(startCol - i, startRow, startCol - i, startRow + BARRIER_LENGTH, BARRIER_COLOR);
						}
									// Update the barrier count for the player
					if (player == PLAYER1) {
							barrierP1--;
							DrawBarrierCounter(PLAYER1, barrierP1);
					} else if (player == PLAYER2) {
							barrierP2--;
							DrawBarrierCounter(PLAYER2, barrierP2);
					}
					previewBarrierActive = 0;
				x=barrierRow;
				y=barrierCol;
				barrierRow=3;
				barrierCol=3;
				barrierOrientation=HORIZONTAL;
        currentMode = MODE_MOVE_TOKEN; // Switch back to token movement mode
				SwitchTurn();
        }
				
    }

    

    // You might want to add code here to redraw the player's remaining barrier count on the screen
  }
// Check if the selected position for the barrier is valid without creating a '+' formation

void PlaceBarrierConfirmed(){
	if (currentMode == MODE_PLACE_BARRIER && previewBarrierActive ) {
        // Code to place barrier at the selected position
				int gridRow = (barrierRow * 2) - 1; // Convert to 13x13 grid
        int gridCol = (barrierCol * 2) - 1;
		if(currentPlayer==PLAYER1 && IsValidBarrierPosition(gridRow, gridCol, barrierOrientation)){
			if(barrierP1>0){
				PlaceBarrier(currentPlayer, barrierRow, barrierCol, barrierOrientation);
				//resetting for the next barrier placement
				
		
				//redraw all barriers in case some of them got overwritten
				//RedrawAllBarriers();
				previewBarrierActive=0;
				tokenOrBarrier=1;
				if(barrierOrientation==VERTICAL)
					finalBarrierOrientation=1;
				else if(barrierOrientation==HORIZONTAL)
					finalBarrierOrientation=0;
					
			}else{
				DisplayNoWallsAvailableMessage();
			}
		}else if(currentPlayer==PLAYER2 && IsValidBarrierPosition(gridRow, gridCol, barrierOrientation)){
			if(barrierP2>0){
				PlaceBarrier(currentPlayer, barrierRow, barrierCol, barrierOrientation);
				//resetting for the next barrier placement
				
		
				//redraw all barriers in case some of them got overwritten
				//RedrawAllBarriers();
				if(barrierOrientation==VERTICAL)
					finalBarrierOrientation=0;
				else if(barrierOrientation==HORIZONTAL)
					finalBarrierOrientation=1;
				tokenOrBarrier=1;	
				previewBarrierActive=0;
			}else{
				DisplayNoWallsAvailableMessage();
			}
		}    
    }
}



void DrawPreviewBarrier(uint16_t row, uint16_t column, uint16_t orientation) {
		// Convert grid coordinates to pixel coordinates for the start of the barrier
    uint16_t startCol = BARRIER_SPACE + column * (CELL_WIDTH + BARRIER_SPACE);
    uint16_t startRow = BARRIER_SPACE + row * (CELL_HEIGHT + BARRIER_SPACE);
		int i;
    // Adjust the starting point to the space between cells for the barrier
    if (orientation == HORIZONTAL) {
        startRow -= BARRIER_THICKNESS / 2;
        for ( i = 0; i < BARRIER_THICKNESS; i++) {
            LCD_DrawLine(startCol, startRow - i, startCol + BARRIER_LENGTH, startRow - i, Green);
        }
    } else { // VERTICAL
        // startX adjusted to center the barrier vertically between the blocks
        startCol -= BARRIER_THICKNESS / 2;
        for ( i = 0; i < BARRIER_THICKNESS; i++) {
            LCD_DrawLine(startCol - i, startRow, startCol - i, startRow + BARRIER_LENGTH, Green);
        }
    }
		previewBarrierActive=1;
}
void ClearPreviewBarrier() {//not used
    // Redraw the grid cells that were covered by the preview barrier
	uint16_t startCol, startRow;
	uint16_t backgroundColor = White;
	int i;
     if (barrierOrientation == HORIZONTAL) {
        startCol = BARRIER_SPACE + barrierCol * (CELL_WIDTH + BARRIER_SPACE);
        // Subtract 1 from barrierRow to get the row above the current barrier
        startRow = BARRIER_SPACE + ((barrierRow - 1) * (CELL_HEIGHT + BARRIER_SPACE)) + (CELL_HEIGHT+2) - (BARRIER_THICKNESS / 2);
        for (i = 0; i < BARRIER_THICKNESS; i++) {
            LCD_DrawLine(startCol, startRow + i, startCol + BARRIER_LENGTH, startRow + i, backgroundColor);
        }
    } else { // VERTICAL
        // Subtract 1 from barrierCol to get the column to the left of the current barrier
        startCol = BARRIER_SPACE + ((barrierCol - 1) * (CELL_WIDTH + BARRIER_SPACE)) + (CELL_WIDTH+2) - (BARRIER_THICKNESS / 2);
        startRow = BARRIER_SPACE + barrierRow * (CELL_HEIGHT + BARRIER_SPACE);
        for (i = 0; i < BARRIER_THICKNESS; i++) {
            LCD_DrawLine(startCol + i, startRow, startCol + i, startRow + BARRIER_LENGTH, backgroundColor);
        }
    }
}
void DrawHalfBarrierLeft(uint16_t row, uint16_t column) {
    uint16_t startCol = BARRIER_SPACE + column * (CELL_WIDTH + BARRIER_SPACE);
    uint16_t startRow = BARRIER_SPACE + row * (CELL_HEIGHT + BARRIER_SPACE) - (BARRIER_THICKNESS / 2);
	int i;
    for ( i = 0; i < BARRIER_THICKNESS; i++) {
        LCD_DrawLine(startCol, startRow - i, startCol + HALF_BARRIER_SIZE, startRow - i, BARRIER_COLOR);
    }
}


void DrawHalfBarrierRight(uint16_t row, uint16_t column) {
	int i;
    uint16_t startCol = BARRIER_SPACE + column * (CELL_WIDTH + BARRIER_SPACE) + CELL_WIDTH+3; // Start at half cell width
    uint16_t startRow = BARRIER_SPACE + row * (CELL_HEIGHT + BARRIER_SPACE) - (BARRIER_THICKNESS / 2);
    for ( i = 0; i < BARRIER_THICKNESS; i++) {
        LCD_DrawLine(startCol, startRow - i, startCol + HALF_BARRIER_SIZE, startRow - i, BARRIER_COLOR);
    }
}

void DrawHalfBarrierUp(uint16_t row, uint16_t column) {
	int i;
     uint16_t startCol = BARRIER_SPACE + column * (CELL_WIDTH + BARRIER_SPACE) - (BARRIER_THICKNESS / 2);
		 uint16_t startRow = BARRIER_SPACE + (row - 1) * (CELL_HEIGHT + BARRIER_SPACE) + (CELL_HEIGHT + BARRIER_SPACE / 2);
    for ( i = 0; i < BARRIER_THICKNESS; i++) {
        LCD_DrawLine(startCol - i, startRow, startCol - i, startRow + HALF_BARRIER_SIZE+ BARRIER_SPACE, BARRIER_COLOR);
    }
}

void DrawHalfBarrierDown(uint16_t row, uint16_t column) {
	int i;
    uint16_t startCol = BARRIER_SPACE + column * (CELL_WIDTH + BARRIER_SPACE) - (BARRIER_THICKNESS / 2);
    // Shift the starting row down by one cell
    uint16_t startRow = BARRIER_SPACE + row * (CELL_HEIGHT + BARRIER_SPACE)+CELL_HEIGHT+BARRIER_SPACE;
    for ( i = 0; i < BARRIER_THICKNESS; i++) {
        LCD_DrawLine(startCol - i, startRow, startCol - i, startRow + HALF_BARRIER_SIZE, BARRIER_COLOR);
    }
}

void DrawFullBarrierHorizontal(uint16_t row, uint16_t column){
	int startCol, startRow, i;
	startCol = BARRIER_SPACE + column * (CELL_WIDTH + BARRIER_SPACE);
  startRow = BARRIER_SPACE + row * (CELL_HEIGHT + BARRIER_SPACE) - (BARRIER_THICKNESS / 2);
  for (i = 0; i < BARRIER_THICKNESS; i++) {
     LCD_DrawLine(startCol, startRow - i, startCol + BARRIER_LENGTH, startRow - i, BARRIER_COLOR);
  }
}

void DrawFullBarrierVertical(uint16_t row, uint16_t column){
	int startCol, startRow, i;
	startCol = BARRIER_SPACE + column * (CELL_WIDTH + BARRIER_SPACE) - (BARRIER_THICKNESS / 2);
	startRow = BARRIER_SPACE + row * (CELL_HEIGHT + BARRIER_SPACE);
	for (i = 0; i < BARRIER_THICKNESS; i++) {
			LCD_DrawLine(startCol - i, startRow, startCol - i, startRow + BARRIER_LENGTH, BARRIER_COLOR);
	}
}
void RedrawOverlappedBarriers13x13(int barrierRow, int barrierCol, int orientation) {
    // Convert to 13x13 grid coordinates
    int gridRow = (barrierRow * 2) - 1;
    int gridCol = (barrierCol * 2) - 1;

    if(barrierOrientation==HORIZONTAL){
			// Check the left side for overlap on an even column in the 13x13 grid.
        if (gridCol < GRID_SIZE13 && (gameBoard13[gridRow][gridCol+3] == BARRIER_MIDDLE || gameBoard13[gridRow][gridCol+3] == BARRIER_END)) {
            DrawHalfBarrierRight(barrierRow, barrierCol);
				}
			// Check the right side for overlap on an even column in the 13x13 grid.
        if (gridCol < GRID_SIZE13 && (gameBoard13[gridRow][gridCol + 1] == BARRIER_MIDDLE || gameBoard13[gridRow][gridCol + 1] == BARRIER_END)) {
            DrawHalfBarrierLeft(barrierRow, barrierCol);
        }
		}else if (orientation == VERTICAL) {
			// Check the lower side for overlap on an even row in the 13x13 grid.
        if (gridRow < GRID_SIZE13  && (gameBoard13[gridRow + 3][gridCol] == BARRIER_MIDDLE || gameBoard13[gridRow + 3][gridCol] == BARRIER_END)) {
            // If we are on an even row and there is a barrier below, draw the top half of the barrier.
            DrawHalfBarrierDown(barrierRow, barrierCol);
        }
        // Check the upper side for overlap on an even row in the 13x13 grid.
        if (gridRow <GRID_SIZE13 && (gameBoard13[gridRow + 1][gridCol] == BARRIER_MIDDLE || gameBoard13[gridRow + 1][gridCol] == BARRIER_END)) {
            // If we are on an even row and there is a barrier above, draw the bottom half of the barrier.
            DrawHalfBarrierUp(barrierRow, barrierCol);
        }
        
		
		}
	}		

 void UpdatePreviewBarrier(uint16_t row, uint16_t column, uint16_t orientation) {
	 if(previewBarrierActive){
    DrawPreviewBarrier(row, column, orientation);
    barrierRow = row;
    barrierCol = column;
    barrierOrientation = orientation;
	 }
}
 
void DisplayNoWallsAvailableMessage() {
    if(currentPlayer==PLAYER1 && available_message==0){
			GUI_Text(30,237,(uint8_t *)"P1:no walls available", Blue, White);
			available_message=1;
		}
		else if(currentPlayer==PLAYER2 && available_message==0){
			GUI_Text(30,237,(uint8_t *)"P2:no walls available", Red, White);
			available_message=1;
		}
}
void ClearNoWallsAvailableMessage() {
    if(currentPlayer==PLAYER1 && available_message==1){
			GUI_Text(30,237,(uint8_t *)"P1:no walls available", White, White);
			available_message=0;
		}
		else if(currentPlayer==PLAYER2 && available_message==0){
			GUI_Text(30,237,(uint8_t *)"P2:no walls available", Red, White);
			available_message=0;
		}
}

void MapPlayerPositionsTo13x13() {//not used
    // Player 1
    gameBoard13[cursorRow_Player1 * 2][cursorCol_Player1 * 2] = PLAYER1;
    // Player 2
    gameBoard13[cursorRow_Player2 * 2][cursorCol_Player2 * 2] = PLAYER2;
}

uint32_t encodeMove(uint8_t playerID, uint8_t moveWallPlacement, uint8_t verticalHorizontal, uint8_t y, uint8_t x){
	uint32_t move = 0;
	move |= (uint32_t) playerID<<24;
	move |= (uint32_t) moveWallPlacement<<20;
	
	move |= (uint32_t) verticalHorizontal<<16;
	move |= (uint32_t) y<<8;
	move |= (uint32_t) x;
	return move;
}

void decodeMove(uint32_t move, uint8_t *playerID, uint8_t *moveWallPlacement, uint8_t *verticalHorizontal, uint8_t *y, uint8_t *x) {
    *playerID = (move >> 24) & 0xFF;
    *moveWallPlacement = (move >> 20) & 0x0F;
    *verticalHorizontal = (move >> 16) & 0x0F;
    *y = (move >> 8) & 0xFF;
    *x = move & 0xFF;
}


