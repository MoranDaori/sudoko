#ifndef GAME
#define GAME

#include "structs.h"
#include <stdio.h>
#include <stdlib.h>

#define MAXITER 1000

void printBoard(Sudoku *board);

void editMode(char *file_path, Sudoku* board);

void solveMode(char *file_path, Sudoku *board);

int set(int row, int coll, int value, Sudoku *board);

int validate(Sudoku *board);

int undo(Sudoku *board);

int redo(Sudoku *board);

void saveBoard(char* file_path, Sudoku *board);

int hint(Sudoku* board, int x, int y);

void numOfSolutions(Sudoku *board);

int autofill(Sudoku *board);

int guess (Sudoku* board, double X);

void generate(Sudoku* board, int X, int Y);

int guessHint (Sudoku* board, int line, int coll);

void reset(Sudoku *board);

void freeGame(Sudoku *board);

#endif
