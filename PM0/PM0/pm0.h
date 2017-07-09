// --------------------------- //
//     PM/O Compiler, HW1
// Gabrielle Michaels, ga177098
//    COP 3402, Summer 2017
// --------------------------- //

#ifndef PM0_H_
#define PM0_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// --------------------------- //
//       VARIABLE SETUP        //
// --------------------------- //

// define compiler consts
#define MAX_STACK_HEIGHT 2000
#define MAX_CODE_LENGTH 500
#define MAX_LEXI_LEVEL 3

// making enums for the OP codes (for easy translation from IR.OP)
typedef enum opCodes {
	LIT = 1,
	OPR = 2,
	LOD = 3,
	STO = 4,
	CAL = 5,
	INC = 6,
	JMP = 7,
	JPC = 8,
	SIO = 9
} OPCode;

const char * opStrings[] = { "", "LIT", "OPR", "LOD", "STO", "CAL",
	"INC", "JMP", "JPC", "SIO" };

// instruction struct and typedef
// instructions look like OP L M
typedef struct {
	int op;		// op code
	int l;		// lexicographical level
	int m;		// modifier
	int line;	// line number
} Instruction;

// stack and code declarations
int stack[MAX_STACK_HEIGHT + 1] = { 0 };	// initializes all elements to 0
Instruction code[MAX_CODE_LENGTH + 1];
int ARIlist[MAX_STACK_HEIGHT + 1] = { -1 };	// keeps track of where ARIs start


// --------------------------- //
//    FUNCTION DECLARATONS     //
// --------------------------- //

// base function, provided by homework pdf
int base(int l, int base);
int runVM(int argc, char * argv[]);



// --------------------------- //
//    FUNCTION DEFINITIONS     //
// --------------------------- //

// main function
int runVM(int argc, char * argv[])
{
	// opening text file for instructions
	FILE * readFrom;
	readFrom = fopen("vminput.txt", "r");

	// creating output file
	FILE * writeTo;
	writeTo = fopen("vmoutput.txt", "w+");

	// if file doesn't exist, exit
	if (!readFrom) {
		perror("Error opening file vminput.txt.");
		return 0;
	}

	int codeCounter = 0;
	while (!feof(readFrom)) {

		// if codeCounter increases to MAX_CODE_LENGTH + 1, regardless of file length, HALT is inserted
		// at the end of the code array and the file stops being read
		if (codeCounter == MAX_CODE_LENGTH + 1) {
			codeCounter--;
			perror("Code overflow, HALT instruction automatically inserted at end of code, and rest of code is discarded.");
			code[(codeCounter)].op = 9;
			code[(codeCounter)].l = 0;
			code[(codeCounter)].m = 3;

			break;
		}

		fscanf(readFrom, "%d ", &code[codeCounter].op);
		fscanf(readFrom, "%d ", &code[codeCounter].l);
		fscanf(readFrom, "%d", &code[codeCounter].m);
		code[(codeCounter)].line = codeCounter;

		codeCounter++;
	}

	fclose(readFrom);

	fprintf(writeTo, "Line    OP    L    M\n");

	// prints file contents and op code interpretation
	int i;
	for (i = 0; i < codeCounter; i++) {
		fprintf(writeTo, "%4d    %s   %1d    %d\n", i, opStrings[code[i].op], code[i].l, code[i].m);
	}

	// setup pointers
	int sp = 0;		// stack pointer
	int bp = 1;		// base pointer
	int pc = 0;		// program counter
	Instruction ir;	// instruction register

	// print initial stuff and headers
	fprintf(writeTo, "\n");
	fprintf(writeTo, "\n");
	fprintf(writeTo, "-- stack after each instruction --\n");
	fprintf(writeTo, "                       %3s   %3s   %3s    stack\n", "pc", "bp", "sp");
	fprintf(writeTo, "Initial values         %3d   %3d   %3d    \n", pc, bp, sp);


	// while a halt instruction hasn't occurred, keep running
	int run = 0;
	while (run == 0) {

		fprintf(writeTo, "%3d   %s  %3d  %3d    ", code[pc].line, opStrings[code[pc].op], code[pc].l, code[pc].m);

		// fetch instruction
		ir.op	= code[pc].op;
		ir.l	= code[pc].l;
		ir.m	= code[pc].m;

		// increase pc by 1
		pc++;

		// check if sp is trying to access past stack boundaries
		if (sp > MAX_STACK_HEIGHT) {
			perror("Stack pointer attempting to access past max stack height, stack pointer set to top of stack.");
			sp = MAX_STACK_HEIGHT;
		}

		switch ((OPCode)ir.op) {
		// op code 01 is LIT (literal)
		case LIT:
			sp++;
			stack[sp] = ir.m;
			break;

		// op code 02 is OPR (arithmetic or logical operation)
		case OPR:
			switch (ir.m) {

			//RET
			case 0:
				sp = bp - 1;
				pc = stack[sp + 4];
				bp = stack[sp + 3];
				break;

			// NEG
			case 1:
				stack[sp] = -stack[sp];
				break;

			// ADD
			case 2:
				sp--;
				stack[sp] += stack[sp + 1];
				break;

			// SUB
			case 3:
				sp--;
				stack[sp] -= stack[sp + 1];
				break;

			// MUL
			case 4:
				sp--;
				stack[sp] *= stack[sp + 1];
				break;

			// DIV
			case 5:
				sp--;
				stack[sp] /= stack[sp + 1];
				break;

			// ODD
			case 6:
				stack[sp] = stack[sp] % 2;
				break;

			// MOD
			case 7:
				sp--;
				stack[sp] %= stack[sp + 1];
				break;

			// EQL, 0 is true, 1 is false
			case 8:
				sp--;
				stack[sp] = (stack[sp] == stack[sp + 1]) ? 1 : 0;
				break;

			// NEQ, 0 is true, 1 is false
			case 9:
				sp--;
				stack[sp] = (stack[sp] != stack[sp + 1]) ? 1 : 0;
				break;

			// LSS
			case 10:
				sp--;
				stack[sp] = (stack[sp] < stack[sp + 1]) ? 1 : 0;
				break;

			// LEQ
			case 11:
				sp--;
				stack[sp] = (stack[sp] <= stack[sp + 1]) ? 1 : 0;
				break;

			// GTR
			case 12:
				sp--;
				stack[sp] = (stack[sp] > stack[sp + 1]) ? 1 : 0;
				break;

			// GEQ
			case 13:
				sp--;
				stack[sp] = (stack[sp] >= stack[sp + 1]) ? 1 : 0;
				break;

			default:
				break;
			}
			break;

		// op code 03 is LOD (load)
		case LOD:
			sp++;
			stack[sp] = stack[base(ir.l, bp) + ir.m];
			break;

		// op code 04 is STO (store)
		case STO:
			stack[base(ir.l, bp) + ir.m] = stack[sp];
			sp--;
			break;

		// op code 05 is CAL (call procedure)
		case CAL:
			ARIlist[sp + 1] = 1;			// activation record instance separator goes here
			stack[sp + 1] = 0;				// space to return value
			stack[sp + 2] = base(ir.l, bp);	// static link (SL)
			stack[sp + 3] = bp;				// dynamic link (DL)
			stack[sp + 4] = pc;				// return address (RA)

			bp = sp + 1;
			pc = ir.m;

			// find a way to delineate activation records
			break;

		// op code 06 is INC (allocating for incoming locals)
		case INC:
			sp += ir.m;
			break;

		// op code 07 is JMP (jump)
		case JMP:
			pc = ir.m;
			break;

		// op code 08 is JPC (jump if 0)
		case JPC:
			if (stack[sp] == 0) {
				pc = ir.m;
				sp--;
			}
			break;

		// op code 09 is SIO (standard input/output and halt)
		case SIO:

			// print top of stack to screen
			if (ir.m == 1) {
				printf("Stack at position %d: %d", sp, stack[sp]);
				sp--;
			}

			// read input from user and put on stack
			else if (ir.m == 2) {
				int num, items;

				items = scanf("%d", &num);

				sp++;
				stack[sp] = 0;

				if (items == EOF) {
					perror("Input failed, value of '0' input into stack.");
				}
				else if (items == 0) {
					perror("No input, value of '0' input into stack.");
				}
				else if (!isdigit(num)) {
					perror("Input was not a number, value of '0' input into stack.");
				}
				else {
					stack[sp] = num;
				}
			}

			else {
				run = 1;
				sp = 0;
				bp = 0;
				pc = 0;
			}
			break;

		default:
			break;
		}

		fprintf(writeTo, "%3d   %3d   %3d    ", pc, bp, sp);

		// because of the CAL instruction, sometimes there are values past sp and bp is larger than sp
		int stop = sp;
		if (sp < bp) {
			stop = bp + 3;
		}

		if (ir.op == JMP || run == 1)
			stop = 0;

		for (i = 1; i <= stop; i++) {

			// if there's a pipe at that location, then an ARI starts there
			if (ARIlist[i] == 1)
				fprintf(writeTo, "| ");

			fprintf(writeTo, "%d ", stack[i]);
		}

		fprintf(writeTo, "\n");

	}

	printf("PM/0 finished running. Please see vmoutput.txt for execution details.\n");
	fclose(writeTo);

	return 0;
}

// find base l levels down
int base(int l, int base) {
	int b1;
	b1 = base;

	while (l > 0)
	{
		b1 = stack[b1 + 1];
		l--;
	}

	return b1;
}

#endif