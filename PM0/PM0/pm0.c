// --------------------------- //
//     PM/O Compiler, HW1
// Gabrielle Michaels, ga177098
//    CAP 3402, Summer 2017
// --------------------------- //

#include <stdio.h>
#include <string.h>

// --------------------------- //
//           SETUP             //
// --------------------------- //

// define compiler consts
#define MAX_STACK_HEIGHT 2000
#define MAX_CODE_LENGTH 500
#define MAX_LEXI_LEVEL 3

// making enums for the OP codes (for easy translation from IR.OP)
typedef enum OPCode {
	LIT = 1,
	OPR = 2,
	LOD = 3,
	STO = 4,
	CAL = 5,
	INC = 6,
	JMP = 7,
	JPC = 8,
	SIO = 9
};


// --------------------------- //
//         FUNCTIONS           //
// --------------------------- //

// base function, provided by homework pdf
int base(int l, int base);

// main function
int main(int argc, char * argv[])
{


	return 0;
}