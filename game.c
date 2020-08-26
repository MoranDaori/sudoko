#include "game.h"
#include "mainAux.h"
#include "structs.h"
#include "LP.h"
#include "ILP.h"
#include "solver.h"
#include <stdio.h>
#include <stdlib.h>

/*print a empty Sudoku board game*/
void printBoard(Sudoku *board)
{
	int index, j, i, x = 0, y, n, m, size, sizeLine, sizeColl, sizeColl1;
	n = board->n;
	m = board->m;
	size = board->boardSize;
	sizeLine = size + n + 1;
	sizeColl = size + m + 1;
	sizeColl1 = 4 * size + m + 1;
	for (index = 0; index < sizeLine; index++)
	{
		if (index % (m + 1) == 0)
		{
			for (j = 0; j < sizeColl1; j++)
			{
				printf("-");
			}
		}
		else
		{
			for (y = 0, i = 0; i < sizeColl; i++, y++)
			{
				if (i % (n + 1) == 0)
				{
					if (!((size == 1) && (i != 0)))
						printf("|");
					y--;
				}
				else if (board->currentBoard[x][y].value == 0)
				{
					printf("    ");
				}
				else
				{
					printf(" %2d", board->currentBoard[x][y].value);
					if (board->currentBoard[x][y].fixed == 1)
					{
						printf(".");
					}
					else if ((board->currentBoard[x][y].error == 1) && (board->gameStatus == 1 || board->markErrors == 1))
					{
						printf("*");
					}
					else
					{
						printf(" ");
					}
				}
			}
			x++;
		}
		printf("\n");
	}
	return;
}
/*get to edit mode and start new game*/
void editMode(char * file_path, Sudoku * board)
{
	int mode=board->gameStatus;
	board->gameStatus = 1;
	if (newGame(file_path, board)==1)
	{
		findErrors(board);
		printBoard(board);
	}
	else{
		board->gameStatus = mode;
	}
}
/*get to solve mode and start new game*/
void solveMode(char *file_path, Sudoku *board) 
{
	int mode = board->gameStatus;
	board->gameStatus = 2;
	if (newGame(file_path, board)==1)
	{
		findErrors(board);
		printBoard(board);
	}
	else
	{
		board->gameStatus = mode;
	}
}

/*sets the value of the cell <col,row> to value*/
int set(int line, int coll, int value, Sudoku *board)
{
	int *numOfSetCommands;
	int **setOfSetCommands;
	cell **copyBoard;
	Move *createMove;
	int size;
	size = board->boardSize;
	numOfSetCommands = (int*)malloc(sizeof(int) * 3);
	setOfSetCommands = (int**)malloc(sizeof(int*));
	if ((numOfSetCommands == NULL)||(setOfSetCommands==NULL)){
		printf("Allocation failed\n");
		return -1;
	}
	if ((board->gameStatus == 2) && (board->currentBoard[line - 1][coll - 1].fixed == 1)) 
	{
		printf("Error: the required cell is fixed\n");
		free(numOfSetCommands);
		free(setOfSetCommands);
		return -1;
	}
	numOfSetCommands[0] = line;
	numOfSetCommands[1] = coll;
	numOfSetCommands[2] = value;
	setOfSetCommands[0] = numOfSetCommands;
	createMove = newMove(1, setOfSetCommands);
	if (createMove==NULL){
		printf("Allocation failed\n");
		free(numOfSetCommands);
		free(setOfSetCommands);
		return -1;
	}
	copyBoard = CopyBoard(board->currentBoard, size);
	if (copyBoard==NULL){
		printf("Allocation failed\n");
		free(numOfSetCommands);
		free(setOfSetCommands);
		free (createMove);
		return -1;
	}
	copyBoard[line - 1][coll - 1].value = value;
	createMove->board = copyBoard;
	board->currentBoard = copyBoard;
	makingMove(board, createMove);
	findErrors(board);
	printBoard(board);
	if (board->gameStatus == 2) 
	{
		gameOver(board);
	}
	return 1;
}

/*Validates the current board using ILP, ensuring it is solvable*/
int validate(Sudoku *board)
{
	cell** copyBoard;
	int size, ret, i;
	size = board->boardSize;
	copyBoard = CopyBoard(board->currentBoard, size);
	if (copyBoard==NULL){
		printf("Allocation failed\n");
		return -1;
	}

	if (errorStatus(board))
	{
		printf("Error: the board is erroneous\n");
		if (copyBoard)
		{
			for (i = 0; i < board->boardSize ; i++)
				free(copyBoard[i]);
			free(copyBoard);
		}
		return -1;
	}
	ret=ILPSolver(copyBoard, size, board->n, board->m);
	if (ret==0)
	{
		printf("Validation passed: the board is solvable\n");
	}
	else
	{
		printf("Validation failed: the board is unsolvable\n");
	}
	if (copyBoard)
	{
		for (i = 0; i < board->boardSize ; i++)
			free(copyBoard[i]);
		free(copyBoard);
	}
	return ret;
}

/*Undo a previous move done by the user.*/
int undo(Sudoku *board)
{
	int command[STRLEN][4];
	int i, j, line, coll, newValue, oldValue, counter;
	Move *lastMove;
	if (board->moves->head == board->moves->current)
	{
		printf("Error: there are no moves to undo\n");
		return -1;
	}
	counter = 0;
	for (i = 0; i < board->moves->current->numOfCommands; i++)
	{
		line = board->moves->current->numOfSetCommands[i][0];
		coll = board->moves->current->numOfSetCommands[i][1];
		newValue = board->moves->current->numOfSetCommands[i][2];
		lastMove = board->moves->current->prev;
		oldValue = lastMove->board[line - 1][coll - 1].value;
		command[i][0] = line;
		command[i][1] = coll;
		command[i][2] = newValue;
		command[i][3] = oldValue;
		counter++;
	}
	board->moves->current = (board->moves->current->prev);
	board->currentBoard = board->moves->current->board;
	for (j = 0; j < counter; j++)
	{
		if (!command[j][2])
		{
			printf("Undo <%d,%d>: from _ to %d\n", command[j][1], command[j][0], command[j][3]);
		}
		else if (!command[j][3])
		{
			printf("Undo <%d,%d>: from %d to _\n", command[j][1], command[j][0], command[j][2]);
		}
		else
			printf("Undo <%d,%d>: from %d to %d\n", command[j][1], command[j][0], command[j][2], command[j][3]);
	}
	return 1;
}

/*Redo a move previously undone by the user*/
int redo(Sudoku *board)
{
	int command[STRLEN][4];
	int i, j, line, coll, newValue, oldValue, count;
	Move *nextMove;
	if (board->moves->current->next == NULL)
	{
		printf("Error: there are no moves to redo\n");
		return -1;
	}
	count = 0;
	nextMove = board->moves->current->next;
	for (i = 0; i < nextMove->numOfCommands; i++) 
	{
		line = nextMove->numOfSetCommands[i][0];
		coll = nextMove->numOfSetCommands[i][1];
		oldValue = board->currentBoard[line - 1][coll - 1].value;
		newValue = nextMove->board[line - 1][coll - 1].value;
		command[i][0] = line;
		command[i][1] = coll;
		command[i][2] = oldValue;
		command[i][3] = newValue;
		count++;
	}
	board->moves->current = nextMove;
	board->currentBoard = nextMove->board;
	for (j = 0; j < count; j++)
	{
		if (!command[j][2])
		{
			printf("Redo <%d,%d>: from _ to %d\n", command[j][1], command[j][0], command[j][3]);
		}
		else if (!command[j][3])
		{
			printf("Redo <%d,%d>: from %d to _\n", command[j][1], command[j][0], command[j][2]);
		}
		else
			printf("Redo <%d,%d>: from %d to %d\n", command[j][1], command[j][0], command[j][2], command[j][3]);
	}
	return 1;
}

/*Saves the current game board to the specified file, where the input includes a full or relative path to the file*/
void saveBoard(char * file_path, Sudoku * board)
{
	cell** copyBoard;
	int i = 0, j = 0, size;
	FILE* saveFile;
	copyBoard = CopyBoard(board->currentBoard, board->boardSize);
	if (copyBoard==NULL){
		printf("Allocation failed\n");
		return;
	}
	size = board->boardSize;
	saveFile=NULL;
	saveFile = fopen(file_path, "w");
	if (!legalBoardToSave(board, saveFile))
	{
		return;
	}
	board->currentBoard = copyBoard;
	fprintf(saveFile, "%d %d\n", board->m, board->n);
	for (i = 0; i < size; i++)
	{
		for (j = 0; j < size; j++)
		{
			fprintf(saveFile, "%d", board->currentBoard[i][j].value);
			if ((board->currentBoard[i][j].fixed == 1) || ((board->gameStatus == 1) && (board->currentBoard[i][j].value != 0)))
			{
				fprintf(saveFile, ".");
			}
			fprintf(saveFile, " ");
		}
		fprintf(saveFile, "\n");
	}
	fclose(saveFile);
	printf("Saved to: %s\n", file_path);
	return;
}

/*Give a hint to the user by showing the solution of a single cell X,Y*/
int hint(Sudoku* board, int line, int coll)
{
	cell** copyBoard;
	int hint, size, i;
	size = board->boardSize;
	copyBoard = CopyBoard(board->currentBoard, size);
	if (copyBoard==NULL){
		printf("Allocation failed\n");
		return -1;
	}
	if (board->currentBoard[line - 1][coll - 1].fixed == 1)
	{
		printf("Error: the cell <%d,%d> is fixed\n", coll, line);
		if (copyBoard)
		{
			for (i = 0; i < board->boardSize ; i++)
				free(copyBoard[i]);
			free(copyBoard);
		}
		return -1;
	}
	else if (board->currentBoard[line - 1][coll - 1].value != 0)
	{
		printf("Error: the cell <%d,%d> already contains a value\n", coll, line);
		if (copyBoard)
		{
			for (i = 0; i < board->boardSize ; i++)
				free(copyBoard[i]);
			free(copyBoard);
		}
		return -1;
	}
	else if (errorStatus(board))
	{
		printf("Error: the board contains erroneous cells\n");
		if (copyBoard)
		{
			for (i = 0; i < board->boardSize ; i++)
				free(copyBoard[i]);
			free(copyBoard);
		}
		return -1;
	}

	hint = ILPSolver(copyBoard, size, board->n, board->m);
	if (!hint)
	{
		printf("Hint: set cell <%d,%d> to %d\n", coll, line, copyBoard[line - 1][coll - 1].value);
	}
	else if (hint)
	{
		printf("the board is unsolvable\n");
	}
	if (copyBoard)
	{
		for (i = 0; i < board->boardSize ; i++)
			free(copyBoard[i]);
		free(copyBoard);
	}
	return hint;
}

/*Print the number of solutions for the current board*/
void numOfSolutions(Sudoku *board)
{
	int size, i, count;
	cell** copyBoard;
	size = board->boardSize;
	copyBoard = CopyBoard(board->currentBoard, size);
	if (copyBoard==NULL){
		printf("Allocation failed\n");
		return;
	}
	if (errorStatus(board))
	{
		printf("Error: the board contains erroneous cells\n");
		if (copyBoard)
		{
			for (i = 0; i < board->boardSize ; i++)
				free(copyBoard[i]);
			free(copyBoard);
		}
		return;
	}
	count=exhaustiveSolver(board);
	if (count!=-1){
		printf("The number of solutions found by the exhaustive backtracking algorithm: %d\n", exhaustiveSolver(board));}
	board->currentBoard = copyBoard;
}

/*Automatically fill "obvious" values – cells which contain a single legal value*/
int autofill(Sudoku *board)
{
	cell **copyBoard;
	Move *move;
	int  val = 0, i, j, k, numOfLegalVals, changed, size;
	int numOfCommands = 0;
	int **setCommands;
	if (errorStatus(board))
	{
		printf("Error: the board contains erroneous cells\n");
		return 0;
	}

	size = board->boardSize;
	changed = 0;
	setCommands = (int **)malloc(sizeof(int*) * board->boardSize * board->boardSize);
	if (setCommands==NULL){
		printf("Allocation failed\n");
		return -1;
	}
	copyBoard = CopyBoard(board->currentBoard, size);
	if (copyBoard==NULL){
		printf("Allocation failed\n");
		return -1;
	}
	for (i = 0; i < size; i++)
	{
		for (j = 0; j < size; j++)
		{
			numOfLegalVals = 0;
			if (board->currentBoard[i][j].value != 0){
				continue;
			}
			for (k = 1; k <= size; k++)
			{
				if (numOfLegalVals >= 2)
					break;
				if (legalInput(board, board->currentBoard, i, j, k))
				{
					numOfLegalVals++;
					val = k;
				}
			}
			if (numOfLegalVals == 1)
			{
				changed = 1;
				copyBoard[i][j].value = val;
				setCommands[numOfCommands] = (int*)malloc(sizeof(int) * 3);
				setCommands[numOfCommands][0] = i + 1;
				setCommands[numOfCommands][1] = j + 1;
				setCommands[numOfCommands][2] = val;
				numOfCommands++;
				printf("Cell <%d,%d> set to %d\n", j + 1, i + 1, val);
			}

		}
	}

	if (changed == 0)
	{
		return 1;
	}
	board->currentBoard = copyBoard;
	if (numOfCommands > 0) 
	{
		move = newMove(numOfCommands, setCommands);
		if (move==NULL){
			printf("Allocation failed\n");
			return -1;
		}
		move->board = copyBoard;
		makingMove(board, move);
		gameOver(board);
	}
	return 1;
}

/*Guesses a solution to the current board using LP, with threshold X,returns 1 if the command been done*/
int guess (Sudoku* board, double X){
	cell** copyBoard;
	Move * move;
	int size, i,hint;
	int numOfCommands = 0;
	int ** setCommands = NULL;
	size = board->boardSize;
	copyBoard = CopyBoard(board->currentBoard, size);
	if (copyBoard==NULL){
		printf("Allocation failed\n");
		return 0;
	}
	if (errorStatus(board))
	{
		printf("Error: the board contains erroneous cells\n");
		if (copyBoard)
		{
			for (i = 0; i < board->boardSize ; i++)
				free(copyBoard[i]);
			free(copyBoard);
		}
		return 0;
	}
	hint=LPSolver(copyBoard, size, board->n, board->m, X , 0, 1, 1, board);
	if (hint!=0){
		printf ("Error: the board is unsolvable\n");
		for (i = 0; i < board->boardSize ; i++)
			free(copyBoard[i]);
		free(copyBoard);
		return 0;
	}
	setCommands = copyMoves(copyBoard, board->currentBoard, size, &numOfCommands);
	if (setCommands==NULL){
		printf("Allocation failed\n");
		for (i = 0; i < board->boardSize ; i++)
			free(copyBoard[i]);
		free(copyBoard);
		return 0;
	}
	board->currentBoard=copyBoard;
	printBoard(board);
	if (numOfCommands>0){
		move = newMove(numOfCommands, setCommands);
		if (move==NULL){
			printf("Allocation failed\n");
			for (i = 0; i < board->boardSize ; i++)
				free(copyBoard[i]);
			free(copyBoard);
			return 0;
		}
		move->board = copyBoard;
		makingMove(board, move);
		gameOver(board);
	}
	return 1;
}

/*Generates a puzzle by randomly filling X empty cells with legal values,
  running ILP to solve the board, and then clearing all but Y random cells*/
void generate(Sudoku* board, int X, int Y)
{
	Move * move;
	cell** copyBoard;
	int val = 0, valIndex, solve, line, coll, iterations, cellsToFill, cellsToKeep, size, i, j, avaliableInputLen, flag = 1;
	cell** Xcells = NULL;
	cell** Ycells = NULL;
	int* availableVals = NULL;
	int numOfCommands = 0;
	int ** setCommands = NULL;
	size = board->boardSize;
	iterations = 0;
	availableVals= (int*)malloc(size * sizeof(int));
	copyBoard = CopyBoard(board->currentBoard, size);
	Xcells = CopyBoard(board->currentBoard, size);
	Ycells = CopyBoard(board->currentBoard, size);

	if (availableVals == NULL) {
		printf("Allocation failed\n");
		return;
	}
	if (ILPSolver(copyBoard, size, board->n, board->m)!=0){
		printf("Error: the board is unsolvable\n");
		return;
	}
	copyBoard = CopyBoard(board->currentBoard, size);
	if ((copyBoard==NULL)||(Xcells==NULL)||(Ycells==NULL)){
		printf("Allocation failed\n");
		free(availableVals);
		return;
	}
	if (numOfEmptyCells(board) < X)
	{
		printf("Error: there is not enough empty cells in the board\n");
		free(availableVals);
		for (i=0; i<size; i++){
			free(copyBoard[i]);
			free(Xcells[i]);
			free(Ycells[i]);
		}
		free(copyBoard);
		free(Xcells);
		free(Ycells);
		return;
	}
	for (iterations = 0; iterations < MAXITER; iterations++) {
		for (i=0; i<size; i++){
			for (j=0; j<size; j++){
				Xcells[i][j].value=0;
				Ycells[i][j].value=0;
			}
		}
		cellsToFill = 0;
		cellsToKeep = 0;
		while (cellsToFill < X)
		{
			line = rand() % size;
			coll = rand() % size;
			if ((copyBoard[line][coll].value == 0)&&(Xcells[line][coll].value!=1))
			{
				Xcells[line][coll].value=1;
				cellsToFill++;
			}
		}

		while (cellsToKeep < Y)
		{
			line = rand() % size;
			coll = rand() % size;
			if (Ycells[line][coll].value!=1){
				Ycells[line][coll].value=1;
				cellsToKeep++;
			}
		}
		for (i=0; i<size; i++){
			for (j=0; j<size; j++){
				if (Xcells[line][coll].value==1){
					avaliableInputLen = availableInputs(board, copyBoard, line, coll, availableVals);
					while (avaliableInputLen>0){
						valIndex = rand() % (avaliableInputLen - 1);
						val = availableVals[valIndex];
						if (!(legalInput(board, copyBoard, line, coll, val))){
							memmove(&availableVals[valIndex], &availableVals[valIndex + 1], (avaliableInputLen - valIndex - 1) * sizeof(int));
							avaliableInputLen--;
						}
						else if (legalInput(board, copyBoard, line, coll, val)){
							copyBoard[line][coll].value=val;
							break;
						}
					}
					if (avaliableInputLen == 0)
					{
						copyBoard = CopyBoard(board->currentBoard, size);
						flag=0;
						break;
					}
				}
			}
		}
		if (flag == 0)
		{
			flag = 1;
			continue;
		}
		solve=ILPSolver(copyBoard, size, board->n, board->m);
		if (solve==0){
			for (line = 0; line < size; line++)
			{
				for (coll = 0; coll < size; coll++)
				{
					if (Ycells[line][coll].value == 0)
					{
						copyBoard[line][coll].value = 0;
					}
				}
			}
			setCommands = copyMoves(copyBoard, board->currentBoard, size, &numOfCommands);
			if (setCommands==NULL){
				printf("Allocation failed\n");
				free(availableVals);
				for (i=0; i<size; i++){
					free(copyBoard[i]);
					free (Xcells[i]);
					free (Ycells[i]);}
				free(copyBoard);
				free (Ycells);
				free(Xcells);
				return;
			}
			board->currentBoard=copyBoard;
			if (numOfCommands>0){
				move = newMove(numOfCommands, setCommands);
				if (move==NULL){
					printf("Allocation failed\n");
					free(availableVals);
					for (i=0; i<size; i++){
						free(copyBoard[i]);
						free (Xcells[i]);
						free (Ycells[i]);}
					free(copyBoard);
					free (Ycells);
					free(Xcells);
					return;
				}
				move->board = copyBoard;
				makingMove(board, move);
				gameOver(board);
			}
			free(availableVals);
			for (i=0; i<size; i++)
			{
				free (Xcells[i]);
				free (Ycells[i]);
			}
			free (Ycells);
			free(Xcells);
			return;
		}
		else if (solve==-1)
		{
			free(availableVals);
			for (i=0; i<size; i++){
				free (Xcells[i]);
				free (Ycells[i]);
			}
			free (Ycells);
			free(Xcells);
			return;
		}
		else
		{
			copyBoard = CopyBoard(board->currentBoard, size);
			continue;
		}
		printf("Error: puzzle generator failed\n");
		free(availableVals);
		for (i=0; i<size; i++){
			free (Xcells[i]);
			free (Ycells[i]);
		}
		free (Ycells);
		free(Xcells);
		return;
	}
}

/* Show a guess to the user for a single cell, returns 0 if the hint was not exposed*/
int guessHint (Sudoku* board, int line, int coll)
{
	int hint;
	cell** copyBoard;
	int size, i;
	size = board->boardSize;
	copyBoard = CopyBoard(board->currentBoard, size);
	if (copyBoard==NULL){
		printf("Allocation failed\n");
		return 0;
	}
	if (errorStatus(board))
	{
		printf("Error: the board contains erroneous cells\n");
		if (copyBoard)
		{
			for (i = 0; i < board->boardSize ; i++)
				free(copyBoard[i]);
			free(copyBoard);
		}
		return 0;
	}
	else if (board->currentBoard[line - 1][coll - 1].fixed == 1)
	{
		printf("Error: the cell <%d,%d> is fixed\n", coll, line);
		if (copyBoard)
		{
			for (i = 0; i < board->boardSize ; i++)
				free(copyBoard[i]);
			free(copyBoard);
		}
		return 0;
	}
	else if (board->currentBoard[line - 1][coll - 1].value != 0)
	{
		printf("Error: the cell <%d,%d> already contains a value\n", coll, line);
		if (copyBoard)
		{
			for (i = 0; i < board->boardSize ; i++)
				free(copyBoard[i]);
			free(copyBoard);
		}
		return 0;
	}

	hint=LPSolver(copyBoard, size, board->n, board->m,0.0 , 1,line,coll,board);
	if (hint!=0){
		printf ("Error: the board is unsolvable\n");
		for (i = 0; i < board->boardSize ; i++)
			free(copyBoard[i]);
		free(copyBoard);

		return 0;
	}
	return hint;
}

/*Undo all moves, reverting the board to its original loaded state. */
void reset(Sudoku *board)
{
	int i, j, size;
	Move *temp_move;
	board->moves->current = board->moves->head;
	size = board->boardSize;
	if (board->moves->head != NULL)
	{
		while (board->moves->current != board->moves->tail)
		{
			temp_move = board->moves->tail->prev;
			for (i = 0; i < size; i++) 
			{
				free(board->moves->tail->board[i]);
			}
			free(board->moves->tail->board);
			if (board->moves->tail->numOfCommands > 0)
			{
				for (j = 0; j < board->moves->tail->numOfCommands; j++)
				{
					free(board->moves->tail->numOfSetCommands[j]);
				}
			}
			free(board->moves->tail->numOfSetCommands);
			free(board->moves->tail);
			board->moves->tail = temp_move;
		}
	}
	else
	{
		board->moves->tail = NULL;
		board->moves->current = NULL;
	}
	printf("The board reset\n");
	board->moves->tail->next = NULL;
	board->currentBoard = board->moves->current->board;
	return;
}

/*exiting the game and free all the memory of the Sudoku board*/
void freeGame(Sudoku *board)
{
	int i;
	if (board->moves->current != board->moves->tail)
		deleteOldMoves(board);
	if (board->moves){
		free(board->moves);}
	if (board->currentBoard)
	{
		for (i = 0; i < board->boardSize ; i++)
			free(board->currentBoard[i]);
		free(board->currentBoard);
	}
	free(board);
}

