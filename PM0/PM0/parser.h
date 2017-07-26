// --------------------------- //
//    Parser/Code Gen, HW2
// Gabrielle Michaels, ga177098
//    COP 3402, Summer 2017
// --------------------------- //

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
	char * nested;	// is nested inside of this procedure and can be called by it or its parents
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
int tokenCount = 0;
Token currentToken;
Instruction gencode[MAX_CODE_LENGTH];

int codeLine = 0;
char * token;

FILE * writeTo;

int returnValue = 0;
int level = 0; // the lexicographical level


// --------------------------- //
//          FUNCTIONS          //
// --------------------------- //
int runParser();
void program();
void block(char * nested);
void statement(char * nested);
void condition();
void expression();
void term();
void factor();
void getToken();
Symbol * getSymbol(char * tokenName);
void putSymbol(Symbol sym);
void emit(int op, int l, int m);
void checkError(char * symType, int err);
char * chuckError(int error);



void program() {

	getToken();
	block("");

	// if current token is not a period, throw error
	checkError("periodsym", 9);

	// emit halt command
	emit(SIO, 0, 3);

}

// pass the nested procedure into the block so it can be noted
void block(char * nested) {
	
	if (level == 0)
		nested = "";

	// counting number of variables for INC instruction
	int varCount = 0;
	int temp = codeLine;
	emit(JMP, 0, 0);

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

			if (strcmp(currentToken.type, "becomesym") == 0)
				checkError("", 1);

			checkError("eqlsym", 3);

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

		checkError("semicolonsym", 5);

		getToken();
	}

	// variable declaration(s)
	if (strcmp(currentToken.type, "varsym") == 0) {

		varCount = 0; // resetting for when you need to "INC" after proc
		do {

			Symbol newSym;
			newSym.kind = 2;

			getToken();

			checkError("identsym", 4);

			varCount++;

			// get string for variable name and update symbol table
			newSym.name = currentToken.name;
			newSym.level = level;
			newSym.addr = 3 + varCount; // addresses start at 4
			putSymbol(newSym);

			getToken();

		} while (strcmp(currentToken.type, "commasym") == 0);

		checkError("semicolonsym", 5);

		getToken();
	}

	// procedures
	while (strcmp(currentToken.type, "procsym") == 0) {
		getToken();

		checkError("identsym", 4);

		Symbol newSym;
		newSym.kind = 3; // proc sym
		newSym.name = currentToken.name;
		newSym.level = level;
		newSym.addr = codeLine;
		newSym.nested = nested; // empty string if level 0 procedure, proc identifier if otherwise

		level++; // entered proc, increase lex level

		putSymbol(newSym);

		getToken();

		checkError("semicolonsym", 5);
		getToken();

		// if a new procedure is called, it's nested inside of another procedure
		block(newSym.name);

		// now that we've exited the block procedure, the nested procedure is whatever it previously was
		nested = newSym.nested;

		emit(OPR, 0, 0);

		level--; // exited proc, decrease lex level

		checkError("semicolonsym", 5);
		getToken();
	}

	gencode[temp].m = codeLine;

	// incoming 4 + number of variables
	emit(INC, 0, 4 + varCount);

	statement(nested);
}

void statement(char * nested) {

	if (strcmp(currentToken.type, "identsym") == 0)
	{
		// get symbol from table
		Symbol * sym = getSymbol(currentToken.name);

		// if symbol doesn't exist, it wasn't declared
		if (sym == NULL)
			checkError("", 11);

		// can't assign to a const or proc
		else if (sym->kind != 2)
			checkError("", 12);

		getToken();
		checkError("becomesym", 13);

		getToken();
		expression();

		// STORE
		emit(STO, level - sym->level, sym->addr);
	}

	// call a procedure
	else if (strcmp(currentToken.type, "callsym") == 0) {
		getToken();
		checkError("identsym", 14);

		Symbol * sym = getSymbol(currentToken.name);

		// incorrect or undeclared identifier
		if (sym == NULL)
			checkError("", 11);

		// can't call a const or var
		else if (sym->kind != 3)
			checkError("", 15);

		// if procedures are the same level (other than 0), can't be called
		// if a procedure's parent doesn't match the current procedure, can't be called
		if (!(sym->level == 0 && level == 0) || strcmp(sym->nested, nested) != 0)
			checkError("", 30);

		// SEE IF CALLEE IS IN "NESTED" CHAIN, IF NOT, RETURN ERROR

		if (sym->level < level)
			checkError("", 30);

		emit(CAL, level - sym->level, sym->addr);
		getToken();
	}

	else if (strcmp(currentToken.type, "beginsym") == 0) {
		getToken();
		statement(nested);

		while (strcmp(currentToken.type, "semicolonsym") == 0) {
			getToken();
			statement(nested);
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

		statement(nested);

		gencode[temp].m = codeLine;

		if (strcmp(currentToken.type, "elsesym") == 0) {
			gencode[temp].m = codeLine + 1;

			temp = codeLine;
			emit(JMP, 0, 0);
			getToken();
			statement(nested);

			gencode[temp].m = codeLine;
		}
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

		statement(nested);

		emit(JMP, 0, cx1);
		gencode[cx2].m = codeLine;
	}

	else if (strcmp(currentToken.type, "readsym") == 0) {
		getToken();
		checkError("identsym", 27);

		// get input and store in a symbol from symbol table
		Symbol * sym = getSymbol(currentToken.name);

		// identifier doesn't exist in symbol table
		if (sym == NULL)
			checkError("", 11);

		// assignment to const or proc not allowed
		else if (sym->kind != 2)
			checkError("", 12);

		// read in the input
		emit(SIO, 0, 2);

		if (strcmp(currentToken.type, "identsym") == 0)
			emit(STO, 0, sym->addr);

		else
			emit(LIT, 0, currentToken.value);

		getToken();
	}

	else if (strcmp(currentToken.type, "writesym") == 0) {
		getToken();

		if (strcmp(currentToken.type, "identsym") != 0 && strcmp(currentToken.type, "numbersym") != 0)
			checkError("", 27);

		// if the symbol being written to screen is a variable
		if (strcmp(currentToken.type, "identsym") == 0) {
			Symbol * sym = getSymbol(currentToken.name);

			// if symbol doesn't exist in symbol table, error
			if (sym == NULL)
				checkError("", 11);

			// can't read a procedure
			else if (sym->kind == 3)
				checkError("", 29);

			emit(LOD, level - sym->level, sym->addr);
		}
		
		// if it's just a number
		else
			emit(LIT, 0, currentToken.value);

		emit(SIO, 0, 1);

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

		char * tempType = currentToken.type;

		if (strcmp(tempType, "leqsym") != 0 && strcmp(tempType, "neqsym") != 0
			&& strcmp(tempType, "lessym") != 0 && strcmp(tempType, "geqsym") != 0
			&& strcmp(tempType, "gtrsym") != 0 && strcmp(tempType, "eqlsym") != 0)
			checkError("", 20);

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
		else if (strcmp(tempType, "eqlsym") == 0)
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

		// symbol doesn't exist in table, wasn't declared
		if (sym == NULL)
			checkError("", 11);

		// if symbol is a constant
		else if (sym->kind == 1)
			emit(LIT, 0, sym->val);

		// otherwise it's a variable
		else if (sym->kind == 2)
			emit(LOD, level - sym->level, sym->addr);

		// if it's a procedure identifier, throw an error
		else
			checkError("", 21);

				
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

// get next token from token array
void getToken() {
	if (tokenPointer <= tokenCount)
		currentToken = tokens[tokenPointer++];
}

// retrieve symbol from symbol table
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
	
	if (getSymbol(sym.name) == NULL) {
		symbol_table[symPointer] = sym;
		symPointer++;
	}

	// can't have two identifiers with the same name
	else
		checkError("", 28);
}

void emit(int op, int l, int m) {

	// throw error if generated code is too long
	if (codeLine > MAX_CODE_LENGTH)
		checkError("", 26);

	// write generated code to array
	else {
		gencode[codeLine].op = op;
		gencode[codeLine].l = l;
		gencode[codeLine].m = m;
		codeLine++;
	}

}

void checkError(char * symType, int err) {

	if (tokenPointer > tokenCount) {
		printf("%s", chuckError(err));
		exit(1);
	}

	else if (strcmp(currentToken.type, symType) != 0) {
		printf("%s", chuckError(err));
		exit(1);
	}

}

char * chuckError(int error) {
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

	case 27:
		return "read, write must be followed by identifier or number.\n";

	case 28:
		return "Can't have two identifiers with the same name.\n";

	case 29:
		return "Can't perform \"read\" on a procedure.\n";

	case 30:
		return "Can't call a procedure from that level.";

	default:
		return "";
	}
}

int runParser() {

	// creating output file for virtual machine
	writeTo = fopen("vmin.txt", "a+");
	
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

		tokens[tokenCount] = curr;
		tokenCount++;

		if (token != NULL)
			token = strtok(NULL, " ");
		else
			break;
	}

	program();

	int i = 0;
	//write to file
	fprintf(writeTo, "%d %d %d", gencode[0].op, gencode[0].l, gencode[0].m);
	for (i = 1; i < codeLine; i++) {
		fprintf(writeTo, "\n%d %d %d", gencode[i].op, gencode[i].l, gencode[i].m);
	}

	fclose(writeTo);

	return 0;
}

#endif