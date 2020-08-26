#ifndef SOLVER
#define SOLVER

#include "structs.h"
#include "mainAux.h"

int exhaustiveSolver(Sudoku* board);

int numberOfCellsFilled(Sudoku *board);

int findEmptyCell(Sudoku* board, int* line, int* coll);

void push(Stack* stack, int line, int coll, int value, int index);

void pop(Stack* stack, int* line, int* coll, int* value, int index);

#endif
