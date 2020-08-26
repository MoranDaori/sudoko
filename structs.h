#pragma once
#ifndef STRUCTS
#define STRUCTS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#define FALSE 0
#define TRUE !FALSE

#define STRLEN 256
#define MAX_BORAD_SIZE 99

typedef struct 
{
	int value;
	int error;
	int fixed;
} cell;

typedef struct Move 
{
	cell** board;
	int numOfCommands;
	int** numOfSetCommands;
	struct Move* next;
	struct Move* prev;
}Move;

typedef struct 
{
	int line;
	int coll;
	int val;
}StackNode;

typedef struct 
{
	StackNode *stackArray;
}Stack;

typedef struct 
{
	Move *current;
	Move *head;
	Move *tail;
} List;

typedef struct 
{
	int boardSize;
	int n;
	int m;
	int markErrors;
	int gameStatus;
	cell** currentBoard;
	List *moves;
} Sudoku;



#endif
