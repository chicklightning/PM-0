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
int stack[MAX_STACK_HEIGHT] = { 0 };	// initializes all elements to 0
Instruction code[MAX_CODE_LENGTH];


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


	// while a halt instruction hasn't occurred, keep running
	int run = 0;
	while (run == 0) {

		// fetch instruction
		ir.op	= code[pc].op;
		ir.l	= code[pc].l;
		ir.m	= code[pc].m;

		// increase pc by 1
		pc++;

		// check for halt
		if (ir.op == SIO && ir.l == 0 && ir.m == 3)
		{
			run = 1;
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