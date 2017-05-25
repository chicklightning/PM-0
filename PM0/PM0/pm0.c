// --------------------------- //
//     PM/O Compiler, HW1
// Gabrielle Michaels, ga177098
//    CAP 3402, Summer 2017
// --------------------------- //

#include <stdio.h>
#include <string.h>

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
} Instruction;

// stack and code declarations
int stack[MAX_STACK_HEIGHT + 1] = { 0 };	// initializes all elements to 0
Instruction code[MAX_CODE_LENGTH + 1];


// --------------------------- //
//    FUNCTION DECLARATONS     //
// --------------------------- //

// base function, provided by homework pdf
int base(int l, int base);



// --------------------------- //
//    FUNCTION DEFINITIONS     //
// --------------------------- //

// main function
int main(int argc, char * argv[])
{
	// opening text file for instructions
	FILE * file;
	file = fopen("vminput.txt", "r");

	// if file doesn't exist, exit
	if (!file) {
		printf("Error opening file pmasm.txt.");
		return 0;
	}

	int codeCounter = 0;
	while (!feof(file)) {

		// if codeCounter increases to MAX_CODE_LENGTH - 1, regardless of file length, HALT is inserted
		// at the end of the code array and the file stops being read
		if (codeCounter == MAX_CODE_LENGTH - 1) {
			printf("Code overflow, HALT instruction automatically inserted at end of code, and rest of code is discarded.");
			code[(codeCounter)].op = 9;
			code[(codeCounter)].l = 0;
			code[(codeCounter)].m = 3;

			break;
		}

		fscanf(file, "%d ", &code[codeCounter].op);
		fscanf(file, "%d ", &code[codeCounter].l);
		fscanf(file, "%d", &code[codeCounter].m);

		codeCounter++;
	}

	fclose(file);

	printf("\nLine    OP    L    M\n");

	// prints file contents and op code interpretation
	int i;
	for (i = 0; i < codeCounter; i++) {
		printf("%4d    %s   %1d    %d\n", i, opStrings[code[i].op], code[i].l, code[i].m);
	}

	// setup pointers
	int sp = 0;		// stack pointer
	int bp = 1;		// base pointer
	int pc = 0;		// program counter
	Instruction ir;	// instruction register
	int ARIcount = 0;


	// while a halt instruction hasn't occurred, keep running
	int run = 0;
	while (run == 0) {

		// fetch instruction
		ir.op	= code[pc].op;
		ir.l	= code[pc].l;
		ir.m	= code[pc].m;

		// increase pc by 1
		pc++;

		// check for halt (9 0 3)
		if (ir.op == SIO && ir.l == 0 && ir.m == 3)
		{
			run = 1;
			break;
		}

		// check if sp is trying to access past stack boundaries
		if (sp > MAX_STACK_HEIGHT) {
			printf("Stack pointer attempting to access past max stack height, stack pointer set to top of stack.");
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
				
				break;
			
			// NEG
			case 1:

				break;

			// ADD
			case 2:

				break;

			// SUB
			case 3:
			
				break;

			// MUL
			case 4:
				
				break;

			// DIV
			case 5:

				break;

			// ODD
			case 6:

				break;

			// MOD
			case 7:
				
				break;

			// EQL
			case 8:

				break;

			// NEQ
			case 9:
				
				break;

			// LSS
			case 10:

				break;

			// LEQ
			case 11:

				break;

			// GTR
			case 12:
				
				break;

			// GEQ
			case 13:

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
			stack[base(ir.l, bp)] = stack[sp];
			sp--;
			break;

		// op code 05 is CAL (call procedure)
		case CAL:
			stack[sp + 1] = 0;				// space to return value
			stack[sp + 2] = base(ir.l, bp);	// static link (SL)
			stack[sp + 3] = bp;				// dynamic link (DL)
			stack[sp + 4] = pc;				// return address (RA)

			bp = sp + 1;
			pc = ir.m;

			// find a way to delineate activation records
			ARIcount++;
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
					printf("Input failed, value of '0' input into stack.");
				}
				else if (items == 0) {
					printf("No input, value of '0' input into stack.");
				}
				else if (!isdigit(num)) {
					printf("Input was not a number, value of '0' input into stack.");
				}
				else {
					stack[sp] = num;
				}
			}
			break;

		default:
			break;
		}

	}



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