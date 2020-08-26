#include "game.h"
#include "gurobi_c.h"

void LoadStateLP(double* solution, double *obj, char* vtype,  cell ** board, int size);
GRBenv *InitEnv();

int LPSolver(cell ** board, int size, int n, int m, double X, int prints_for_guess_hint, int line, int coll, Sudoku *sudboard);
