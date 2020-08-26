#ifndef MAIN_AUX
#define MAIN_AUX

#include "structs.h"

int** copyMoves(cell ** newBoard, cell** copyBoard, int size, int * numOfCommands);

List * initMoves();

void deafultBoardSize(Sudoku * board);

Sudoku * init(Sudoku *board);

void findErrors(Sudoku *board);

int newGame(char *file_path, Sudoku *board);

void deleteOldMoves(Sudoku *board);

cell** loadBoard(Sudoku *board, FILE *load_file);

cell** createNewBoard(int size);

Move* newMove(int size, int** set_commands);

void makingMove(Sudoku *board, Move *move);

int errorStatus(Sudoku *board);

cell** CopyBoard(cell **board, int size);

int numOfEmptyCells(Sudoku *board);

int legalInputL(Sudoku* board, cell** boardCell, int line, int col, int value);

int legalInputC(Sudoku* board, cell** boardCell, int line, int col, int value);

int legalInputB(Sudoku* board, cell** boardCell, int line, int col, int value);

int legalInput(Sudoku* board, cell** boardCell, int line, int col, int value);

int legalBoardToSave(Sudoku *board, FILE * file);

int finishBoard(Sudoku *board);

int availableInputs(Sudoku* board, cell ** cellBoard, int line, int coll, int *availableVals);

int gameOver(Sudoku *board);

int legalRange(char *str, int size);

#endif 
