
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "structs.h"
#include "gurobi_c.h"
#include "game.h"
#include "mainAux.h"
#include "parser.h"
#include "solver.h"



/* Constraint number 1: every value between 0-size must appear exactly one time for every block */
static int BlockConstraint(int* ind, double* val, GRBmodel *model, int size, int num_row, int num_col)
{
	int i, j, var,count, row , col;
	for (var = 0; var < size; var++)
	{
		for (i = 0; i < num_row; i++)
		{
			for (j = 0; j < num_col; j++)
			{
				count = 0;
				for (row = i*num_col; row < (i + 1)*num_col; row++)
				{
					for (col = j*num_row; col < (j + 1)*num_row; col++)
					{
						ind[count] = row*size*size + col*size + var;
						val[count] = 1.0;
						count++;
					}
				}

				if (GRBaddconstr(model, size, ind, val, GRB_EQUAL, 1.0, NULL))
					return 1;
			}
		}
	}
	return 0;
}

/*Constraint number 2: every value between 0-size must appear exactly one time for every row */
static int RowConstraint(int* ind, double* val, GRBmodel *model, int size)
{
	int i, j, var;
	for (var = 0; var < size; var++)
	{
		for (j = 0; j < size; j++)
		{
			for (i = 0; i < size; i++)
			{
				ind[i] = i*size*size + j*size + var;
				val[i] = 1.0;
			}

			if (GRBaddconstr(model, size, ind, val, GRB_EQUAL, 1.0, NULL))
				return 1;

		}
	}
	return 0;
}

/*Constraint number 3: every value between 0-size must appear exactly one time for every col */
static int ColConstraint(int* ind, double* val, GRBmodel *model, int size)
{
	int i, j, var;
	for (var = 0; var < size; var++)
	{
		for (i = 0; i < size; i++)
		{
			for (j = 0; j < size; j++)
			{
				ind[j] = i*size*size + j*size + var;
				val[j] = 1.0;
			}

			if (GRBaddconstr(model, size, ind, val, GRB_EQUAL, 1.0, NULL))
				return 1;
		}
	}
	return 0;
}

/* Constraint number 4: for every cell there is exactly one value */
static int CellConstraint(int* ind, double* val, GRBmodel *model, int size)
{
	int i, j, var;
	for (i = 0; i < size; i++)
	{
		for (j = 0; j < size; j++)
		{
			for (var = 0; var < size; var++)
			{
				ind[var] = i*size*size + j*size + var;
				val[var] = 1.0;
			}

			if (GRBaddconstr(model, size, ind, val, GRB_EQUAL, 1.0, NULL))
				return 1;

		}
	}
	return 0;
}

/*The main function of the constraints*/
static int AddConstraints(int *ind, double *val, GRBmodel *model, int row, int col)
{
	int error;

	error = CellConstraint(ind, val, model, row*col);
	if (error)
		return error;
	error = RowConstraint(ind, val, model, row*col);
	if (error)
		return error;
	error = ColConstraint(ind, val, model, row*col);
	if (error)
		return error;
	error = BlockConstraint(ind, val, model, row*col,row, col);
	if (error)
		return error;
	return error;

}

/*Intilaize vtype by the given Sudoku board*/
void LoadState(double* solution, char* vtype, cell ** board, int size)
{
	int i, j, var;
	for (i = 0; i < size; i++)
	{
		for (j = 0; j < size; j++)
		{
			for (var = 0; var < size ; var++)
			{
				if (board[i][j].value == var + 1){
					solution[(i*size*size) + (j*size) + var] = 1;
				}
				else{
					solution[(i*size*size) + (j*size) + var] = 0;
				}
				vtype[(i*size*size) + (j*size) + var] = GRB_BINARY;
			}
		}
	}

}


/*Change the currentBoard by the ILP solution that found*/
static void SetSolution(double* solution, cell ** board, int size){
	int line, col, var;
	for (line = 0; line < size; line++)
	{
		for (col = 0; col < size; col++)
		{
			for (var = 0; var < size; var++)
			{
				if (solution[line*size*size + col*size + var] == 1)
				{
					board[line][col].value = (var + 1);
				}
			}
		}
	}
}

/*The function that runs the solution finding*/
int ILPSolver(cell ** board, int size, int n, int m)
{
	int optimstatus, error = 0, ret;
	int* ind;
	double* val;
	double* solution;
	char* vtype;
	GRBenv *env   = NULL;
	GRBmodel *model = NULL;
	ind = (int*)malloc(size * sizeof(int));
	val = (double*)malloc(size * sizeof(double));
	solution = (double*)malloc(size*size*size * sizeof(double));
	vtype = (char*)malloc(size*size*size * sizeof(char));
	if (ind == NULL || val == NULL || solution == NULL || vtype == NULL) {
		printf("The allocation's memory failed\n");
		return 1;
	}

	LoadState(solution, vtype, board, size);
	error = GRBloadenv(&env, "FinalSudoku.log");
		if (error) {
			goto exit;
		}

	error = GRBsetintparam(env, GRB_INT_PAR_LOGTOCONSOLE, 0);
	if (error) {
		printf("ERROR: %s\n", GRBgeterrormsg(env));
		GRBfreeenv(env);
		ret=1;
	}

	error = GRBnewmodel(env, &model, "FinalSudoku", (size*size*size) , NULL, solution, NULL, vtype, NULL);
	if (error)
		goto exit;

	error = AddConstraints(ind, val, model, n, m);
	if (error)
		goto exit;

	error = GRBoptimize(model);
	if (error)
		goto exit;
	error = GRBgetintattr(model, GRB_INT_ATTR_STATUS, &optimstatus);
	if (error)
		goto exit;
	if (optimstatus != GRB_OPTIMAL) {
		ret = 1;
		goto exit;
	}
	error = GRBgetdblattrarray(model, GRB_DBL_ATTR_X, 0,(size*size*size), solution);
	if (error)
		goto exit;
	SetSolution(solution, board, size);
	ret = 0;

exit:

	if (error) {
		printf("ERROR: %s\n", GRBgeterrormsg(env));
		ret = -1;
	}

	GRBfreemodel(model);
	free(solution);
	free(val);
	free(vtype);
	free(ind);
	GRBfreeenv(env);

	return ret;
}
