
#include "LP.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "structs.h"
#include "gurobi_c.h"
#include "game.h"
#include "mainAux.h"
#include "parser.h"
#include "solver.h"

/*returns the value of the cell with the max score*/
static int MaxValArray(double* scores, int size){
	int i;
	double max_score;
	max_score=scores[0];
	for (i = 1; i < size; i++)
	{
		if (scores[i] > max_score)
		{
			max_score  = scores[i];
		}
	}
	while (1){
		i=rand()%size;
		if (scores[i] == max_score)
		{return i+1;}}
	return 0;
}

/*return the max score in the array scores*/
static double MaxScore(double* scores, int size){
	int i;
	double max_score;
	max_score=scores[0];
	for (i = 1; i < size; i++)
	{
		if (scores[i] > max_score)
		{
			max_score  = scores[i];
		}
	}
	return max_score;
}


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

/* Constraint number 5- for every fixed cell */
static int fixedConstraint(int* ind, double* val, GRBmodel *model, cell** board, int size){
	int i, j, var;
	for (var = 0; var < size; var++) {
		for (i = 0; i < size; i++) {
			for (j = 0; j < size; j++) {
				if (board[i][j].value == var + 1) {
					ind[0] = (i * size * size) + (j * size) + var;
					val[0] = 1.0;
					if (GRBaddconstr(model, 1.0, ind, val, GRB_EQUAL, 1.0, NULL))
						return 1;
				}
			}
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
void LoadStateLP(double* solution, double *obj, char* vtype,  cell ** board, int size)
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
				obj[(i*size*size) + (j*size) + var] = rand()%size;
				vtype[(i*size*size) + (j*size) + var] = GRB_CONTINUOUS;
			}
		}
	}

}


/*The function that runs the solution finding*/
int LPSolver(cell ** board, int size, int n, int m, double X, int prints_for_guess_hint, int line, int coll, Sudoku *sudboard)
{
	int optimstatus, ret,start ,error = 0;
	int* ind;
	double* val;
	double* solution;
	double* obj;
	char* vtype;
	double* scores;
	int printer,guess_i,guess_j,max;
	GRBenv *env   = NULL;
	GRBmodel *model = NULL;
	ind = (int*)malloc(size * sizeof(int));
	obj = (double*)malloc(size*size*size * sizeof(double));
	val = (double*)malloc(size * sizeof(double));
	solution = (double*)malloc(size*size*size * sizeof(double));
	vtype = (char*)malloc(size*size*size * sizeof(char));

	if(X>0.0){
	}

	if (ind == NULL || val == NULL || solution == NULL || vtype == NULL || obj == NULL) {
		printf("The allocation's memory failed\n");
		return 1;
	}

	LoadStateLP(solution ,obj, vtype, board, size);
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

	error = GRBaddvars(model, (size*size*size), 0, NULL, NULL, NULL, obj, NULL, NULL, vtype, NULL);
	if (error) {
		printf("ERROR %d GRBaddvars(): %s\n", error, GRBgeterrormsg(env));
		return -1;
	}
	error = GRBoptimize(model);
	if (error)
		goto exit;
	error = GRBgetintattr(model, GRB_INT_ATTR_STATUS, &optimstatus);
	if (error)
		goto exit;
	if (optimstatus == GRB_OPTIMAL) {
		ret=0;
	}
	else if (optimstatus == GRB_INF_OR_UNBD) {
		if (error)
			goto exit;
	}
	else {
		if (error)
			goto exit;
	}
	error = GRBgetdblattrarray(model, GRB_DBL_ATTR_X, 0,(size*size*size), solution);
	if (error)
		goto exit;
	error = fixedConstraint(ind, val, model, board, size);
	if (error)
		goto exit;

	error = GRBgetdblattrarray(model, GRB_DBL_ATTR_X, 0, 0.0, solution);
	if (error) {
		goto exit;
	}

	if (prints_for_guess_hint==1){
		start=(size*size*(line-1))+size*(coll-1);
		for (printer=start;printer<(start+size);printer++){
			if (solution[printer]>0){
				printf("The scores of value %d of cell <%d,%d> is %f\n",((printer% size)+1) ,coll, line,solution[printer]);}
		}
	}
	if(prints_for_guess_hint==0){
		for (guess_i = 0; guess_i < size; guess_i++) {
			for (guess_j = 0; guess_j < size; guess_j++) {
				if (board[guess_i][guess_j].value == 0){
					start=(size*size*(guess_i))+size*(guess_j);
					scores = (double*)malloc(size * sizeof(double));
					if (scores == NULL) {
							printf("The allocation's memory failed\n");
							return 1;
						}
					for (printer=start;printer<(start+size);printer++){
						if (legalInput(sudboard ,board ,guess_i, guess_j, printer%size+1)){
							scores[printer%size]=solution[printer];}
						else{
							scores[printer%size]=0;
						}
					}
					max=MaxValArray(scores,size);
					if ((X< MaxScore(scores, size))|| (X==MaxScore(scores, size))){
						board[guess_i][guess_j].value=max;}
				}
			}
		}
		free (scores);
	}

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
	free(obj);
	GRBfreeenv(env);

	return ret;
}
