#include "solver.h"


/*solve the sudoku using backtracking algorithm*/
int exhaustiveSolver(Sudoku* board)
{
	Stack stack;
	cell** tempBoard = board->currentBoard;
	int line = 0, coll = 0, counter = 0, value = 0, index = 0, size = board->boardSize, flag = 1,
			cellsFilled = numberOfCellsFilled(board), prevColl = 0, prevrow = 0;
	stack.stackArray = (StackNode*)malloc(sizeof(StackNode) * 625);
	if (stack.stackArray==NULL){
		printf("Allocation failed\n");
		return 0;
	}

	while (findEmptyCell(board, &line, &coll))
	{
		value = 1;
		while (value <= size)
		{
			if (legalInput(board, tempBoard, line, coll, value))
			{
				if (cellsFilled < size * size - 1)
				{
					tempBoard[line][coll].value = value;
					push(&stack, line, coll, value, index);
					index++;
					cellsFilled++;
				}
				else
				{
					counter++;
				}
				break;
			}
			value++;
		}
		if (tempBoard[line][coll].value == 0)
		{
			while (index > 0)
			{
				cellsFilled--;
				flag = 1;
				index--;
				pop(&stack, &prevrow, &prevColl, &value, index);
				value++;
				tempBoard[prevrow][prevColl].value = 0;

				while (value <= size)
				{

					if (legalInput(board, tempBoard, prevrow, prevColl, value))
					{
						if (cellsFilled < size*size - 1)
						{
							tempBoard[prevrow][prevColl].value = value;
							push(&stack, prevrow, prevColl, value, index);
							index++;
							cellsFilled++;
							flag = 0;
						}
						else
						{
							counter++;
						}
						break;
					}
					value++;
				}
				if (!flag)
				{
					break;
				}
			}
			if (index == 0)
			{
				free(stack.stackArray);
				return counter;
			}
		}
	}
	free(stack.stackArray);
	return counter;
}

int numberOfCellsFilled(Sudoku *board)
{
	int i = 0, j = 0, numberOfCellsFilled = 0;
	for (i = 0; i < board->boardSize; i++)
	{
		for (j = 0; j < board->boardSize; j++)
		{
			numberOfCellsFilled += board->currentBoard[i][j].value ? 1 : 0;
		}
	}
	return numberOfCellsFilled;
}

int findEmptyCell(Sudoku * board, int * line, int * coll)
{
	int i = 0;
	int	j = 0;
	for (i = 0; i < board->boardSize; i++)
	{
		for (j = 0; j < board->boardSize; j++)
		{
			if (!board->currentBoard[i][j].value)
			{
				*line = i;
				*coll = j;
				return TRUE;
			}
		}
	}
	return FALSE;
}

/*add a new item to the stack*/
void push(Stack* stack, int line, int coll, int value, int index)
{
	stack->stackArray[index].line = line;
	stack->stackArray[index].coll = coll;
	stack->stackArray[index].val = value;
}

/*remove a item from the stack*/
void pop(Stack* stack, int* line, int* coll, int* value, int index)
{
	*line = stack->stackArray[index].line;
	*coll = stack->stackArray[index].coll;
	*value = stack->stackArray[index].val;
}
