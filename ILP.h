#include "game.h"
#include "gurobi_c.h"

GRBenv *InitEnv();

void LoadState(double* solution, char* vtype, cell ** board, int size);

int ILPSolver(cell ** board, int size, int n, int m);
