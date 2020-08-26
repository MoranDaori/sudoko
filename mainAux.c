#include "mainAux.h"
#include "ILP.h"
#include "structs.h"
#include <stdio.h>
#include <stdlib.h>

int** copyMoves(cell ** newBoard, cell** copyBoard, int size, int * numOfCommands)
{
	int line, col;
	int ** setCommands = (int**)malloc(sizeof(int*) * size * size);
	if (setCommands==NULL){
		printf("Allocation failed\n");
		return NULL;
	}
	for (line = 0; line < size; line++)
	{
		for (col = 0; col < size; col++)
		{
			if (newBoard[line][col].value != copyBoard[line][col].value)
			{
				setCommands[*numOfCommands] = (int*)malloc(sizeof(int) * 3);
				setCommands[*numOfCommands][0] = line + 1;
				setCommands[*numOfCommands][1] = col + 1;
				setCommands[*numOfCommands][2] = newBoard[line][col].value;
				(*numOfCommands)++;
			}
		}
	}
	return setCommands;
}

/*initialize a empty move list*/
List * initMoves() 
{
	List *moves;
	moves = (List*)malloc(sizeof(List));
	if (moves==NULL){
		return NULL;
	}
	moves->current = NULL;
	moves->tail = NULL;
	moves->head = NULL;
	return moves;
}

/*initialize a deafult data for Sudoku board*/
void deafultBoardSize(Sudoku * board) 
{
	board->boardSize = 9;
	board->n = 3;
	board->m = 3;
}

/*initialize a empty Sudoku board with deafult_board_size*/
Sudoku * init(Sudoku *board) 
{
	board = (Sudoku*)malloc(sizeof(Sudoku));
	if (board==NULL){
		return NULL;
	}
	board->currentBoard = NULL;
	board->moves = initMoves();
	if (board->moves==NULL){
		return NULL;
	}
	deafultBoardSize(board);
	board->markErrors = 1;
	board->gameStatus = 0;
	return board;
}

/*checks if the sudoku board have errors*/
void findErrors(Sudoku *board)
{
	int line = 0, coll = 0, index, i = 0, j = 0, lineBox = 0, collBox = 0, optionalValue = 0;
	int size = board->boardSize;
	for (line = 0; line < size; line++)
	{
		for (coll = 0; coll < size; coll++)
		{
			optionalValue = board->currentBoard[line][coll].value;
			board->currentBoard[line][coll].error = 0;
			if (optionalValue != 0)
			{
				for (index = 0; index < size; index++)
				{
					if (board->currentBoard[line][index].value == optionalValue && index != coll)
					{
						board->currentBoard[line][coll].error = 1;
					}
					else if (board->currentBoard[index][coll].value == optionalValue && index != line)
					{
						board->currentBoard[line][coll].error = 1;
					}
				}
				lineBox = line / board->m;
				lineBox *= board->m;
				collBox = coll / board->n;
				collBox *= board->n;
				for (i = lineBox; i < board->m + lineBox; i++)
				{
					for (j = collBox; j < board->n + collBox; j++) 
					{
						if (board->currentBoard[i][j].value == optionalValue)
						{
							if (line == i && coll == j)
							{
								continue;
							}
							board->currentBoard[line][coll].error = 1;
						}
					}
				}

			}
		}
	}
}

int newGame(char * file_path, Sudoku * board)
{
	Move* move;
	cell **newBoard;
	FILE* loadFile = NULL;
	if ((board->gameStatus == 1 && file_path != NULL) || board->gameStatus == 2)
	{
		loadFile = fopen(file_path, "r");
		if (loadFile == NULL)
		{
			printf("Error: File doesn't exist\n");
			return -1;
		}

		newBoard = loadBoard(board, loadFile);
		if (!newBoard)
		{
			fclose(loadFile);
			return -1;

		}
	}
	else
	{
		deafultBoardSize(board);
		newBoard = createNewBoard(board->boardSize);
		if (!newBoard){
			printf("Allocation failed\n");
			return -1;
		}
	}
	deleteOldMoves(board);
	move = newMove(0, NULL);
	if (move==NULL){
		printf("Allocation failed\n");
		return -1;
	}
	move->board = newBoard;
	board->currentBoard = newBoard;
	makingMove(board, move);
	return 1;
}

/*deleting all the moves of the game*/
void deleteOldMoves(Sudoku *board)
{
	Move *temp;
	int i, j;
	while (board->moves->tail != NULL)
	{
		for (i = 0; i < board->boardSize; i++)
		{
			free(board->moves->tail->board[i]);
		}
		free(board->moves->tail->board);
		if (board->moves->tail->numOfSetCommands != NULL)
		{
			for (j = 0; j < board->moves->tail->numOfCommands; j++)
			{
				free(board->moves->tail->numOfSetCommands[j]);
			}
			free(board->moves->tail->numOfSetCommands);
		}
		temp = board->moves->tail->prev;
		free(board->moves->tail);
		board->moves->tail = temp;
	}
	board->moves->current = NULL;
	board->moves->head = NULL;
}

cell** loadBoard(Sudoku *board, FILE *load_file)
{
	cell **newBoard;
	int i, j, x, tempn, tempm, tempValue;
	char letter;

	x = fscanf(load_file, "%d %d\n", &tempm, &tempn);
	if (x != 2 || tempm * tempn > MAX_BORAD_SIZE)
	{
		printf("Bad file format\n");
		return NULL;
	}

	board->m = tempm;
	board->n = tempn;
	board->boardSize = board->n * board->m;
	newBoard = createNewBoard(board->boardSize);
	if (newBoard==NULL){
		printf("Allocation failed\n");
		return NULL;
	}

	for (i = 0; i < board->boardSize; i++)
	{
		for (j = 0; j < board->boardSize; j++)
		{
			x = fscanf(load_file, "%d", &tempValue);
			if (tempValue < 0 || tempValue > board->boardSize || x != 1)
			{
				printf("Bad file format\n");
				return NULL;
			}
			newBoard[i][j].value = tempValue;

			letter = fgetc(load_file);
			if (letter == '.')
			{
				if (board->gameStatus == 2 && newBoard[i][j].value != 0)
				{
					newBoard[i][j].fixed = 1;
				}
				fgetc(load_file);
			}
		}
	}
	return newBoard;
}

/*create a new board initialize with 0*/
cell** createNewBoard(int size)
{
	cell** newBoard;
	int i, j;
	newBoard = (cell**)malloc(sizeof(cell*) * size);
	if (newBoard==NULL){
		return NULL;
	}
	for (i = 0; i < size; i++)
	{
		newBoard[i] = (cell*)malloc(sizeof(cell) * size);
		if (newBoard[i]==NULL){
			return NULL;
		}
		for (j = 0; j < size; j++)
		{
			newBoard[i][j].value = 0;
			newBoard[i][j].fixed = 0;
			newBoard[i][j].error = 0;
		}
	}
	return newBoard;
}

/*create a new move initialize to the deafult*/
Move* newMove(int num, int** pointernumOfSetCommands)
{
	Move *move;
	move = (Move*)malloc(sizeof(Move));
	if (move==NULL){
		return NULL;
	}
	move->numOfCommands = num;
	move->numOfSetCommands = pointernumOfSetCommands;
	move->prev = NULL;
	move->next = NULL;
	return move;
}

/*update the moves list*/
void makingMove(Sudoku *board, Move *move)
{
	int i, j;
	Move *temp;
	if (board->moves->head == NULL)
	{
		board->moves->head = move;
		board->moves->current = move;
		board->moves->tail = move;
		return;
	}
	while (board->moves->current != board->moves->tail)
	{
		for (i = 0; i < board->boardSize; i++)
			free(board->moves->tail->board[i]);
		free(board->moves->tail->board);
		if (board->moves->tail->numOfSetCommands != NULL)
			for (j = 0; j < board->moves->tail->numOfCommands; j++)
				free(board->moves->tail->numOfSetCommands[j]);
		free(board->moves->tail->numOfSetCommands);
		temp = board->moves->tail->prev;
		free(board->moves->tail);
		board->moves->tail = temp;
	}
	board->moves->tail->next = move;
	move->prev = board->moves->tail;
	board->moves->tail = move;
	board->moves->current = move;
}

/*update if the cell became to cell with error*/
int errorStatus(Sudoku *board)
{
	int i = 0, j = 0;
	findErrors(board);
	for (i = 0; i < board->boardSize; i++)
		for (j = 0; j < board->boardSize; j++)
			if (board->currentBoard[i][j].error == TRUE)
				return 1;
	return 0;
}

/*making a copy of the board*/
cell** CopyBoard(cell **board, int size)
{
	cell **newBoard;
	int i = 0, j = 0;
	newBoard = createNewBoard(size);
	if (newBoard==NULL){
		return NULL;
	}
	for (i = 0; i < size; i++)
	{
		for (j = 0; j < size; j++)
		{
			newBoard[i][j].value = board[i][j].value;
			newBoard[i][j].fixed = board[i][j].fixed;
			newBoard[i][j].error = board[i][j].error;
		}
	}
	return newBoard;
}

/*returns the number of the empty cells*/
int numOfEmptyCells(Sudoku *board)
{
	int i, j, size, count;
	count = 0;
	size = board->boardSize;
	for (i = 0; i < size; i++)
	{
		for (j = 0; j < size; j++)
		{
			count += !board->currentBoard[i][j].value ? 1 : 0;
		}
	}
	return count;
}

/*if value is a legal input of the cell <col,row> in a row and col returns 1*/
int legalInputL(Sudoku* board, cell** boardCell, int line, int col, int value)
{
	int i, size;
	size = board->boardSize;
	if (value != 0){
		for (i = 0; i < size; i++)
		{
			if (i==col){
				continue;
			}
			else if (boardCell[line][i].value == value)
			{
				return 0;
			}
		}
	}
	return 1;
}

/*if value is a legal input of the cell <col,row> in a row and col returns 1*/
int legalInputC(Sudoku* board, cell** boardCell, int line, int col, int value)
{
	int i, size;
	size = board->boardSize;
	if (value != 0){
		for (i = 0; i < size; i++)
		{
			if (i==line){
				continue;
			}
			else if (boardCell[i][col].value == value)
			{
				return 0;
			}
		}
	}
	return 1;
}


/*if value is a legal input of the cell <col,row> in a block returns 1*/
int legalInputB(Sudoku* board, cell** boardCell, int line, int col, int value)
{
	int i, j, BOARDLINE, BOARDCOL, lineBlock, colBlock;
	BOARDLINE = board->n;
	BOARDCOL = board->m;
	lineBlock = (line / BOARDCOL)*BOARDCOL;
	colBlock = (col / BOARDLINE)*BOARDLINE;
	for (i = 0; i < BOARDCOL; i++) {
		for (j = 0; j < BOARDLINE; j++) {
			if (((lineBlock + i)==line)&&((colBlock + j)==col)){
				continue;
			}
			else if (boardCell[lineBlock + i][colBlock + j].value == value)
			{
				return 0;
			}
		}
	}
	return 1;
}


/*check if value is a legal input of the cell*/
int legalInput(Sudoku* board, cell** boardCell, int line, int col, int value)
{
	int legalL, legelC, legalB;
	legalL = legalInputL(board, boardCell, line, col, value);
	legelC = legalInputC(board, boardCell, line, col, value);
	legalB = legalInputB(board, boardCell, line, col, value);
	return (legalL == 1 && legelC == 1 && legalB == 1);
}

/*check what the status's game and print the save errors*/
int legalBoardToSave(Sudoku * board, FILE * file)
{
	int i;
	cell ** copyBoard;
	copyBoard = CopyBoard(board->currentBoard, board->boardSize);
	if (copyBoard==NULL){
		printf("Allocation failed\n");
		return 0;
	}
	if (file == NULL)
	{
		printf("Error: File cannot be created\n");
		if (copyBoard)
		{
			for (i = 0; i < board->boardSize ; i++)
				free(copyBoard[i]);
			free(copyBoard);
		}
		return 0;
	}
	if (board->gameStatus == 1 && errorStatus(board))
	{
		fclose(file);
		printf("Error: board contains erroneous cells\n");
		if (copyBoard)
		{
			for (i = 0; i < board->boardSize ; i++)
				free(copyBoard[i]);
			free(copyBoard);
		}
		return 0;
	}
	if ((board->gameStatus == 1) && (ILPSolver(copyBoard, board->boardSize, board->n, board->m) != 0))
	{
		fclose(file);
		printf("Error: board not validate \n");
		if (copyBoard)
		{
			for (i = 0; i < board->boardSize ; i++)
				free(copyBoard[i]);
			free(copyBoard);
		}
		return 0;
	}
	return 1;
}

/*check if every cell in the board is full*/
int finishBoard(Sudoku *board) {
	int i, j, size;
	size = board->boardSize;
	for (i = 0; i < size; i++) {
		for (j = 0; j < size; j++) {
			if (board->currentBoard[i][j].value == 0)
				return 0;
		}
	}
	return 1;
}

/*returns a array of all available input of cell*/
int availableInputs(Sudoku* board, cell ** cellBoard, int line, int coll, int *availableVals)
{
	int i, flag, size, index = 0;
	size=board->boardSize;
	for (i = 1; i <= size; i++)
	{
		flag=legalInput(board, cellBoard, line, coll, i);
		if (flag)
		{
			availableVals[index] = i;
			index++;
		}
	}
	return index;
}

/*check if the board got full and there is no errors*/
int gameOver(Sudoku *board)
{
	int error;
	int boarsStatus;
	error = errorStatus(board);
	boarsStatus = finishBoard(board);
	if ((boarsStatus == 1) && (error == 1)) {
		printf("ERROR: Puzzle solution erroneous\n");
		return -1;
	}
	else if ((boarsStatus == 1) && (error == 0)) {
		printf("Puzzle solved successfully!\n");
		board->gameStatus = 0;
		return 1;
	}
	else {
		return 0;
	}
}

/*checks if the input is a legal char*/
int legalRange(char *str, int size)
{
	int i = 0;
	while (str[i])
	{
		if (str[i] - '0' < 0 || str[i] - '0' > 9)
		{
			return 0;
		}
		i++;
	}
	if (atoi(str) > size)
	{
		return 0;
	}
	return 1;
}
