#include "parser.h"
#include "game.h"
#include <string.h>
#include "structs.h"
#include <stdio.h>
#include <stdlib.h>


/*read the wanted commend and run the game*/
void readCommand(Sudoku * board, char *command)
{
	char *str = NULL;
	char *delim = " \t\r\n";
	char* colStr, *lineStr, *valueStr, *X, *Y;

	str = strtok(command, delim);

	if (!strcmp(str, "print_board"))
	{
		if (strtok(NULL, delim))
		{
			printf("%s", TOOMANYPAR);
			return;
		}
		if (board->gameStatus == 0)
		{
			printf("%s", NOTGOODMOD);
			return;
		}
		findErrors(board);
		printBoard(board);
		return;
	}

	else if (!strcmp(str, "edit"))
	{
		str = strtok(NULL, delim);
		if (strtok(NULL, delim))
		{
			printf("%s", TOOMANYPAR);
			return;
		}
		editMode(str, board);
		return;
	}

	else if (!strcmp(str, "solve"))
	{
		str = strtok(NULL, delim);
		if (str == NULL)
		{
			printf("Error: invalid number of arguments. Usage: 'solve' <path>\n");
			return;
		}
		if (strtok(NULL, delim))
		{
			printf("%s", TOOMANYPAR);
			return;
		}
		solveMode(str, board);
		return;
	}

	else if (!strcmp(str, "mark_errors"))
	{
		str = strtok(NULL, delim);
		if (str == NULL)
		{
			printf("Error: invalid number of arguments. Usage: 'mark_errors' <1 / 0>\n");
			return;
		}
		if (strtok(NULL, delim))
		{
			printf("%s", TOOMANYPAR);
			return;
		}
		if (!(!strcmp(str, "0") || !strcmp(str, "1")))
		{
			printf("Error: invalid command. Usage: 'mark_errors' <1 / 0>\n");
			return;
		}
		if (board->gameStatus == 0 || board->gameStatus == 1)
		{
			printf("%s", NOTGOODMOD);
			return;
		}
		if (!strcmp(str, "0"))
		{
			board->markErrors = 0;
			return;
		}
		else if (!strcmp(str, "1"))
		{
			board->markErrors = 1;
			return;
		}
	}

	else if (!strcmp(str, "set"))
	{
		str = strtok(NULL, delim);
		if (str == NULL)
		{
			printf("%s", NUMPAR);
			return;
		} 
		colStr = str;

		str = strtok(NULL, delim);
		if (str == NULL)
		{
			printf("%s", NUMPAR);
			return;
		}
		lineStr = str;

		str = strtok(NULL, delim);
		if (str == NULL)
		{
			printf("%s", NUMPAR);
			return;
		}
		valueStr = str;

		if (strtok(NULL, delim))
		{
			printf("%s", TOOMANYPAR);
			return;
		}

		if (!legalRange(colStr, board->boardSize) || atoi(colStr) == 0)
		{
			printf("Error: the value of parameter 1 not integer in range: %d-%d\n", 1, board->boardSize);
			return;
		}

		if (!legalRange(lineStr, board->boardSize) || atoi(lineStr) == 0)
		{
			printf("Error: the value of parameter 2 not integer in range: %d-%d\n", 1, board->boardSize);
			return;
		}

		if (!legalRange(str, board->boardSize))
		{
			printf("Error: the value of parameter 3 not integer in range: %d-%d\n", 0, board->boardSize);
			return;
		}

		if (board->gameStatus == 0)
		{
			printf("%s", NOTGOODMOD);
			return;
		}
		set(atoi(lineStr), atoi(colStr), atoi(valueStr), board);
		return;
	}

	else if (!strcmp(str, "validate")) 
	{
		if (strtok(NULL, delim))
		{
			printf("%s", TOOMANYPAR);
			return;
		}
		if (board->gameStatus == 0)
		{
			printf("%s", NOTGOODMOD);
			return;
		}
		validate(board);
		return;
	}
	else if (!strcmp(str, "guess"))
	{

		str = strtok(NULL, delim);
		if (str == NULL)
		{
			printf("%s", NUMPAR);
			return;
		}
		X = str;
		str = strtok(NULL, delim);
		if (strtok(NULL, delim))
				{
					printf("%s", TOOMANYPAR);
					return;
				}
		if (board->gameStatus != 2)
		{
			printf("%s", NOTGOODMOD);
			return;
		}

		guess(board,atof(X));
		return;

	}
	else if (!strcmp(str, "generate"))
	{

		str = strtok(NULL, delim);
		if (str == NULL)
		{
			printf("%s", NUMPAR);
			return;
		}
		X = str;
		str = strtok(NULL, delim);
		if (str == NULL)
		{
			printf("%s", NUMPAR);
			return;
		}
		Y=str;
		if (strtok(NULL, delim))
		{
			printf("%s", TOOMANYPAR);
			return;
		}
		if (!(legalRange(X, ((board->boardSize)*(board->boardSize)))))
		{
			printf("Error: the value of parameter 1 not integer in range: %d-%d\n", 0, ((board->boardSize)*(board->boardSize)));
			return;
		}
		if (!(legalRange(Y, ((board->boardSize)*(board->boardSize)))))
		{
			printf("Error: the value of parameter 2 not integer in range: %d-%d\n", 0, ((board->boardSize) * (board->boardSize)));
			return;
		}
		if (board->gameStatus != 1)
		{
			printf("%s", NOTGOODMOD);
			return;
		}
		generate(board, atoi(X), atoi(Y));
		findErrors(board);
		printBoard(board);
		return;
	}

	else if (!strcmp(str, "undo"))
	{
		if (strtok(NULL, delim))
		{
			printf("%s", TOOMANYPAR);
			return;
		}
		if (board->gameStatus == 0)
		{
			printf("%s", NOTGOODMOD);
			return;
		}
		undo(board);
		findErrors(board);
		printBoard(board);
		return;
	}

	else if (!strcmp(str, "redo"))
	{
		if (strtok(NULL, delim))
		{
			printf("%s", TOOMANYPAR);
			return;
		}
		if (board->gameStatus == 0)
		{
			printf("%s", NOTGOODMOD);
			return;
		}
		redo(board);
		findErrors(board);
		printBoard(board);
		return;
	}

	else if (!strcmp(str, "save"))
	{
		str = strtok(NULL, delim);
		if (str == NULL)
		{
			printf("Error: invalid number of arguments. Usage: 'save' <path>\n");
			return;
		}
		if (strtok(NULL, delim))
		{
			printf("%s", TOOMANYPAR);
			return;
		}
		if (board->gameStatus == 0)
		{
			printf("%s", NOTGOODMOD);
			return;
		}
		saveBoard(str, board);
		return;
	}

	else if (!strcmp(str, "hint"))
	{
		str = strtok(NULL, delim);
		if (str == NULL)
		{
			printf("%s", NUMPAR);
			return;
		}
		X = str;
		str = strtok(NULL, delim);
		if (str == NULL)
		{
			printf("%s", NUMPAR);
			return;
		}
		Y = str;

		if (strtok(NULL, delim))
		{
			printf("%s", TOOMANYPAR);
			return;
		}
		if ((legalRange(X, (board->boardSize)) == 0) || (atoi(X) == 0))
		{
			printf("Error: the value of parameter 1 not integer in range: %d-%d\n", 1, (board->boardSize));
			return;
		}

		else if (legalRange(Y, (board->boardSize)) == 0 || (atoi(Y) == 0))
		{
			printf("Error: the value of parameter 2 not integer in range: %d-%d\n", 1, (board->boardSize));
			return;
		}

		if (board->gameStatus != 2)
		{
			printf("%s", NOTGOODMOD);
			return;
		}
		hint(board, atoi(Y), atoi(X));
		return;
	}

	else if (!strcmp(str, "guess_hint"))
	{
		str = strtok(NULL, delim);
		if (str == NULL)
		{
			printf("%s", NUMPAR);
			return;
		}
		X = str;
		str = strtok(NULL, delim);
		if (str == NULL)
		{
			printf("%s", NUMPAR);
			return;
		}
		Y = str;

		if (strtok(NULL, delim))
		{
			printf("%s", TOOMANYPAR);
			return;
		}
		if ((legalRange(X, (board->boardSize)) == 0) || (atoi(X) == 0))
		{
			printf("Error: the value of parameter 1 not integer in range: %d-%d\n", 1, (board->boardSize));
			return;
		}

		else if ((legalRange(Y, (board->boardSize)) == 0) || (atoi(Y) == 0))
		{
			printf("Error: the value of parameter 2 not integer in range: %d-%d\n", 1, (board->boardSize));
			return;
		}

		if (board->gameStatus != 2)
		{
			printf("%s", NOTGOODMOD);
			return;
		}
		guessHint ( board, atoi(Y), atoi(X));
		return;
	}



	else if (!strcmp(str, "num_solutions"))
	{
		if (strtok(NULL, delim))
		{
			printf("%s", TOOMANYPAR);
			return;
		}
		if (board->gameStatus == 0)
		{
			printf("%s", NOTGOODMOD);
			return;
		}

		numOfSolutions(board);
		return;
	}

	else if (!strcmp(str, "autofill"))
	{
		if (strtok(NULL, delim))
		{
			printf("%s", TOOMANYPAR);
			return;
		}
		if (board->gameStatus != 2)
		{
			printf("%s", NOTGOODMOD);
			return;
		}

		autofill(board);
		findErrors(board);
		printBoard(board);
		return;
	}

	else if (!strcmp(str, "reset"))
	{
		if (strtok(NULL, delim))
		{
			printf("%s", TOOMANYPAR);
			return;
		}
		if (board->gameStatus == 0)
		{
			printf("%s", NOTGOODMOD);
			return;
		}
		reset(board);
		findErrors(board);
		printBoard(board);
		return;
	}

	else if (!strcmp(str, "exit"))
	{
		if (strtok(NULL, delim))
		{
			printf("%s", TOOMANYPAR);
			return;
		}
		freeGame(board);
		printf("Exit message: Terminates the game\n");
		exit(1);
	}

	else
	{
		printf("Error: the command is invalid\n");
		return;
	}
	return;
}
