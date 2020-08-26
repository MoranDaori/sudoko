/*
 ============================================================================
 Name        : finalProject.c
 Author      : cohav
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "structs.h"
#include "SPBufferset.h"

int main(void)
{
	Sudoku * board = NULL;
	int ch = 0;
	char command[STRLEN + 2];
	SP_BUFF_SET();
	srand(time(NULL));
	board = init(board);
	if (board==NULL){
		printf("Allocation failed\n");
		return -1;
	}
	printf("Start Sudoku game\n-----------------\n");

	while (TRUE)
	{
		printf("Enter your command:\n");
		if (fgets(command, STRLEN + 2, stdin) == NULL)
		{
			printf("invaild command:\n");
			continue;
		}
		if (command[strlen(command)] == '\0' && command[strlen(command) - 1] != '\n')
				{
					while ((ch = getchar()) != '\n' && ch != EOF);
					memset(command, 0, sizeof(command));
					printf("max long commend is 256 chars\n");
					continue;
		}
		if (sscanf(command, "%d", &ch) == -1)
		{
			printf("invaild command:\n");
			continue;
		}
		else if (*command == EOF)
		{
			freeGame(board);
			printf("Exit message: Terminates the game");
			exit(1);
		}
		readCommand(board, command);
	}
	return 0;
}
