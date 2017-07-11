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
#include "scanner.h"
#include "pm0.h"

// --------------------------- //
//       VARIABLE SETUP        //
// --------------------------- //
#define MAX_SYMBOL_TABLE_SIZE 500

typedef struct {
	int kind;		// const = 1, var = 2, proc = 3, but we're not using procedure so
	char * name;	// up to 11 characters
	int val;		// number (ASCII value)
	int level;		// L level
	int addr;		// M address
} Symbol;

typedef struct {
	int value;		// value for numbersym
	char * name;	// name for identsym
	char * type;	// token type
} Token;

// for const, store kind, name and val
// for var, store kind, name, L and M
Symbol symbol_table[MAX_SYMBOL_TABLE_SIZE];
int symPointer = 0;
Token tokens[1000];
int tokenPointer = 0;
Token currentToken;

int codeLine = 0;
char * token;

FILE * writeTo;

int returnValue = 0;


// --------------------------- //
//          FUNCTIONS          //
// --------------------------- //
int runParser(int verbose);
void program();

void program() {

	getToken();
	block();

	// if current token is not a period, throw error
	checkError("periodsym", 9);

	// emit halt command
	emit(SIO, 0, 2);

}

void block() {
	
	// counting number of variables for INC instruction
	int varCount = 0;

	// const declaration(s)
	// update symbol table here
	if (strcmp(currentToken.type, "constsym") == 0) {
		do {
			// create new const in symbol table
			Symbol newSym;
			newSym.kind = 1;

			getToken();

			// if there's no identifier, throw error
			checkError("identsym", 4);
			
			// put identifier into symbol for later storage
			newSym.name = currentToken.name;

			getToken();

			if (strcmp(currentToken.type, "becomesym") == 0) {
				printf(error(1));
				exit(1);
			}

			checkError("eqsym", 3);

			getToken();
			
			checkError("numbersym", 2);

			// put token value into symbol table
			newSym.val = currentToken.value;
			newSym.level = 0;
			newSym.addr = 0;
			// update symbol table
			putSymbol(newSym);

			getToken();

		} while (strcmp(currentToken.type, "commasym") == 0);

		checkError("commasym", 5);

		getToken();
	}

	// variable declaration(s)
	if (strcmp(currentToken.type, "varsym") == 0) {

		do {

			Symbol newSym;
			newSym.kind = 2;

			getToken();

			checkError("identsym", 4);

			varCount++;

			// get string for variable name and update symbol table
			newSym.name = currentToken.name;
			newSym.level = 0;
			newSym.addr = 3 + varCount; // addresses start at 4
			putSymbol(newSym);

			getToken();

		} while (strcmp(currentToken.type, "commasym") == 0);

		checkError("semicolonsym", 5);

		getToken();
	}

	// procedure declaration(s) would go here

	code[codeLine].m = codeLine;

	emit(INC, 0, 4 + varCount);

	statement();
}

void statement() {

	if (strcmp(currentToken.type, "identsym") == 0)
	{
		// get symbol from table
		Symbol * sym = getSymbol(currentToken.name);

		getToken();
		checkError("becomesym", 13);

		getToken();
		expression();

		// STORE at lex level 0 using symbol address
		emit(STO, 0, sym->addr);
	}

	// call sym would go here

	else if (strcmp(currentToken.type, "beginsym") == 0) {
		getToken();
		statement();

		while (strcmp(currentToken.type, "semicolonsym") == 0) {
			getToken();
			statement();
		}

		checkError("endsym", 8);
		getToken();
	}

	else if (strcmp(currentToken.type, "ifsym") == 0) {
		getToken();
		condition();

		checkError("thensym", 16);
		getToken();

		int temp = codeLine;
		emit(JPC, 0, 0);

		statement();

		code[temp].m = codeLine;
	}

	else if (strcmp(currentToken.type, "whilesym") == 0) {
		int cx1, cx2;

		cx1 = codeLine;

		getToken();
		condition();

		cx2 = codeLine;
		emit(JPC, 0, 0);

		checkError("dosym", 18);
		getToken();

		statement();

		emit(JMP, 0, cx1);
		code[cx2].m = codeLine;
	}

	else if (strcmp(currentToken.type, "readsym") == 0) {
		getToken();
		checkError("identsym", 14);

		// get input and store in a symbol from symbol table
		Symbol * sym = getSymbol(currentToken.name);

		// read in the input
		emit(SIO, 0, 1);

		if (strcmp(currentToken.type, "identsym") == 0)
			emit(STO, 0, sym->addr);

		else
			emit(LIT, 0, currentToken.value);

		getToken();
	}

	else if (strcmp(currentToken.type, "writesym") == 0) {
		getToken();

		checkError("identsym", 14);
		checkError("numbersym", 14);

		Symbol * sym = getSymbol(currentToken.name);

		// if the symbol being written to is a variable
		if (sym->kind == 2)
			emit(LOD, 0, sym->addr);
		else
			emit(LIT, 0, sym->val);

		emit(SIO, 0, 0);

		getToken();
	}
}

void condition() {

	if (strcmp(currentToken.type, "oddsym") == 0) {
		getToken();
		expression();
	}

	else {
		expression();

		checkError("leqsym", 20);
		checkError("neqsym", 20);
		checkError("lessym", 20);
		checkError("geqsym", 20);
		checkError("gtrsym", 20);
		checkError("eqsym", 20);

		char * tempType = currentToken.type;

		getToken();
		expression();

		if (strcmp(tempType, "leqsym") == 0)
			emit(OPR, 0, 11);
		else if (strcmp(tempType, "neqsym") == 0)
			emit(OPR, 0, 9);
		else if (strcmp(tempType, "lessym") == 0)
			emit(OPR, 0, 10);
		else if (strcmp(tempType, "geqsym") == 0)
			emit(OPR, 0, 13);
		else if (strcmp(tempType, "gtrsym") == 0)
			emit(OPR, 0, 12);
		else if (strcmp(tempType, "eqsym") == 0)
			emit(OPR, 0, 8);
		else
			checkError("", 26);
	}
}

void expression() {
	int addop;

	if (strcmp(currentToken.type, "minussym") == 0 || strcmp(currentToken.type, "plussym") == 0) {
		addop = (strcmp(currentToken.type, "plussym") == 0) ? 4 : 5;

		getToken();
		term();

		// if addop == minussym, negate
		if(addop == 5)
			emit(OPR, 0, 1);
	}

	else
		term();

	while (strcmp(currentToken.type, "plussym") == 0 || strcmp(currentToken.type, "minussym") == 0) {
		
		addop = (strcmp(currentToken.type, "plussym") == 0) ? 4 : 5;

		getToken();
		term();

		// if addop == plussym
		if (addop == 4)
			emit(OPR, 0, 2); // addition
		else
			emit(OPR, 0, 3); // subtraction
	}
}

void term() {
	int mulop;

	factor();

	while (strcmp(currentToken.type, "multsym") == 0 || strcmp(currentToken.type, "slashsym") == 0) {
		mulop = (strcmp(currentToken.type, "multsym") == 0) ? 6 : 7;

		getToken();
		factor();

		// if mulop == multsym
		if (mulop == 6)
			emit(OPR, 0, 4); // multiplication
		else
			emit(OPR, 0, 5); // division
	}
}

void factor() {
	
	if (strcmp(currentToken.type, "identsym") == 0) {

		Symbol * sym = getSymbol(currentToken.name);

		// if symbol is a variable
		if (sym->kind == 1)
			emit(LIT, 0, sym->val);

		// otherwise it's a constant
		else
			emit(LOD, 0, sym->addr);

		getToken();
	}

	else if (strcmp(currentToken.type, "numbersym") == 0) {
		
		emit(LIT, 0, currentToken.value);

		getToken();
	}

	else if (strcmp(currentToken.type, "lparentsym") == 0) {

		getToken();
		expression();

		checkError("rparentsym", 22);

		getToken();
	}

	else
		checkError("", 23);
}

// delimiting tokens by whitespace using strtok
void getToken() {
	currentToken = tokens[tokenPointer++];
}

Symbol * getSymbol(char * tokenName) {

	int i;

	for (i = 0; i < symPointer; i++) {

		// if a match is found, return pointer to symbol table locatoin
		if (strcmp(tokenName, symbol_table[i].name) == 0)
			return &symbol_table[i];

	}

	return NULL;

}

void putSymbol(Symbol sym) {
	
	symbol_table[symPointer] = sym;

	symPointer++;
}

void emit(int op, int l, int m) {

	// throw error if generated code is too long
	if (codeLine > MAX_CODE_LENGTH)
		printf(error(26));

	// write generated code to output file
	else {
		if (codeLine != 0)
			fprintf(writeTo, "\n");

		fprintf(writeTo, "%d %d %d", op, l, m);
		codeLine++;
	}

}

void checkError(char * symType, int err) {

	if (strcmp(currentToken.type, symType) != 0) {
		printf(error(err));
		exit(1);
	}

}

char * error(int error) {
	returnValue = 1;

	switch (error) {
	case 0:
		returnValue = 0;
		return "No errors, program is syntatically correct.\n";

	case 1:
		return "Use = instead of :=.\n";

	case 2:
		return "= must be followed by a number.\n";

	case 3:
		return "Identifier must be followed by =.\n";

	case 4:
		return "\"const\", \"var\", and \"procedure\" must be followed by identifier.\n";

	case 5:
		return "Semicolon or comma missing.\n";

	case 6:
		return "Incorrect symbol after procedure declaration.\n";

	case 7:
		return "Statement expected.\n";

	case 8:
		return "Incorrect symbol after statement part in block.\n";

	case 9:
		return "Period expected.\n";

	case 10:
		return "Semicolon between statements missing.\n";

	case 11:
		return "Undeclared identifier.\n";

	case 12:
		return "Assignment to constant or procedure is not allowed.\n";

	case 13:
		return "Assignment operator expected.\n";

	case 14:
		return "\"call\" must be followed by an identifier.\n";

	case 15:
		return "Call of a constant or variable is meaningless.\n";

	case 16:
		return "\"then\" expected.\n";

	case 17:
		return "Semicolon or } expected.\n";

	case 18:
		return "\"do\" expected.\n";

	case 19:
		return "Incorrect symbol following statement.\n";

	case 20:
		return "Relational operator expected.\n";

	case 21:
		return "Expression must not contain a procedure identifier.\n";

	case 22:
		return "Right parenthesis missing.\n";

	case 23:
		return "The preceding factor cannot begin with this symbol.\n";

	case 24:
		return "An expression cannot begin with this symbol.\n";

	case 25:
		return "This number is too large.\n";

	case 26:
		return "Generated assembly code is too long.\n";

	default:
		return "";
	}
}

int runParser(int verbose) {

	// creating output file for virtual machine
	writeTo = fopen("vmin.txt", "rw+");
	
	// get all tokens and put into a token table
	token = strtok(symbolicLexemeList, " ");

	while (token != NULL) {

		Token curr;
		curr.type = token;

		// if identsym, store name
		if (strcmp(curr.type, "identsym") == 0) {
			token = strtok(NULL, " ");
			curr.name = token;
		}

		// if numbersym, store value
		else if (strcmp(curr.type, "numbersym") == 0) {
			token = strtok(NULL, " ");
			curr.value = atoi(token);
		}

		token = strtok(NULL, " ");
	}

	program();

	// decided by user when running compiler
	if (verbose == 1) {
		// print generated assembly code
		int c;
		printf("Generated assembly code:/n");
		while ((c = fgetc(writeTo)) != EOF)
			putchar(c);

	}

	fclose(writeTo);

	return 0;
}

#endif