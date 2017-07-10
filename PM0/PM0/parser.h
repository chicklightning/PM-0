// --------------------------- //
//    Parser/Code Gen, HW2
// Gabrielle Michaels, ga177098
//    COP 3402, Summer 2017
// --------------------------- //

// This reads the output tokens from scanner.c and
// produces output saying whether the program is syntatically correct
// if not, error type must be printed
// open "scannerout.txt"

// must also generate symbol table which contains all variables,
// procedure names, and constants from the program

// gives the okay for the intermediate code generator

// code generator can also be in here (done as tokens are parsed)


#ifndef PARSER_H_
#define PARSER_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// --------------------------- //
//       VARIABLE SETUP        //
// --------------------------- //
#define MAX_SYMBOL_TABLE_SIZE 500
#define MAX_CODE_LENGTH 500

typedef struct {
	int kind;		// const = 1, var = 2, proc = 3, but we're not using procedure so
	char name[10];	// up to 11 characters
	int val;		// number (ASCII value)
	int level;		// L level
	int addr;		// M address
} Symbol;

// instruction struct and typedef
// instructions look like OP L M
typedef struct {
	int op;		// op code
	int l;		// lexicographical level
	int m;		// modifier
	int line;	// line number
} Instruction;

// for const, store kind, name and val
// for var, store kind, name, L and M
Symbol symbol_table[MAX_SYMBOL_TABLE_SIZE];
Instruction code[MAX_CODE_LENGTH];

int cx;
int currentToken;
char * tokens[1000];


// --------------------------- //
//          FUNCTIONS          //
// --------------------------- //
int runParser(char * symLexemeList, int verbose);

void parseFile(FILE * readFrom, FILE * writeTo) {



}

int runParser(char * symLexemeList, int verbose) {

	// reading from scanner
	FILE * readFrom;
	readFrom = fopen("scannerout.txt", "r");

	// creating output file
	FILE * writeTo;
	writeTo = fopen("vmin.txt", "rw+");
	
	parseFile(readFrom, writeTo);

	// decided by user when running compiler
	if (verbose == 1) {
		// print generated assembly code
		int c;
		printf("Generated assembly code:/n");
		while ((c = fgetc(writeTo)) != EOF)
			putchar(c);

	}

	return 0;
}

#endif