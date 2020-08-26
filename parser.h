#ifndef PARSER
#define PARSER

#include "structs.h"
#include "mainAux.h"
#include "game.h"

#define NOTGOODMOD "ERROR: invalid command (game mode)\n"
#define TOOMANYPAR "ERROR: too many parameters were entered\n"
#define NUMPAR "Error: invalid number of arguments\n"

/*readCommand - reads the command from the user and calls the relevant function */
void readCommand(Sudoku* board, char *command);

#endif 
