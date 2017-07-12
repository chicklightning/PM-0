// --------------------------- //
//    Compiler Driver, HW2
// Gabrielle Michaels, ga177098
//    COP 3402, Summer 2017
// --------------------------- //

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "scanner.h"
#include "parser.h"
#include "pm0.h"

// runs scanner, sends output to parser
// runs parser, sends output to virtual machine
// give input file name to the driver

int main(int argc, char ** argv) {

	if (argc < 2) {
		printf("Invalid arguments for compiler.\nUSAGE: ./compiler [input file] [-l | -a | -v optional]\n");
		return 1;
	}

	// concatenate output files into "out.txt"
	FILE * out = fopen("out.txt", "w+");
	FILE * in1, *in2, *vmtrace;

	// checking for verbose command line arguments
	int l = 0, a = 0, v = 0;
	if (argc > 2)
	{
		switch (argc) {
		case 3:
			l = (strcmp(argv[2], "-l") == 0) ? 1 : 0;
			a = (strcmp(argv[2], "-a") == 0) ? 1 : 0;
			v = (strcmp(argv[2], "-v") == 0) ? 1 : 0;
			break;
		
		case 4:
			l = (strcmp(argv[2], "-l") == 0 || strcmp(argv[3], "-l") == 0) ? 1 : 0;
			a = (strcmp(argv[2], "-a") == 0 || strcmp(argv[3], "-a") == 0) ? 1 : 0;
			v = (strcmp(argv[2], "-v") == 0 || strcmp(argv[3], "-v") == 0) ? 1 : 0;
			break;

		case 5:
			l = (strcmp(argv[2], "-l") == 0 || strcmp(argv[3], "-l") == 0 || strcmp(argv[4], "-l") == 0) ? 1 : 0;
			a = (strcmp(argv[2], "-a") == 0 || strcmp(argv[3], "-a") == 0 || strcmp(argv[4], "-a") == 0) ? 1 : 0;
			v = (strcmp(argv[2], "-v") == 0 || strcmp(argv[3], "-v") == 0 || strcmp(argv[4], "-v") == 0) ? 1 : 0;
			break;

		default:
			break;
		}
	}
	printf("Beginning to scan given code...\n");
	int check1 = runScanner(argv[1], l);
	int check2 = -1;

	// scanner ran correctly
	if (check1 == 0) {
		// put contents of scanner to output
		in1 = fopen("scannerout.txt", "r+");
		char c = fgetc(in1);
		while (c != EOF)
		{
			fputc(c, out);
			c = fgetc(in1);
		}
		fclose(in1);
		check2 = runParser();

	}
	else {
		// open the error file created by the scanner
		in1 = fopen("ef", "r+");
		char c = fgetc(in1);
		while (c != EOF)
		{
			fputc(c, out);
			c = fgetc(in1);
		}
		printf("For fully detailed description of scanning, parsing, and execution, see \"out.txt\".\n");
		fclose(in1);
		return 1;
	}

		
	// parser found no errors
	if (check2 == 0) {
		in2 = fopen("vmin.txt", "r+");

		printf("No errors, program is syntactically correct.\n");

		// put assembly language into output file
		char c = fgetc(in2);
		while (c != EOF)
		{
			fputc(c, out);
			c = fgetc(in2);
		}

		runVM(a);
		
		if (v == 1) // if verbose
			printf("\nVirtual Machine execution trace:\n");
		
		// add vmtrace to output file
		vmtrace = fopen("vmout.txt", "r+");
		c = fgetc(vmtrace);
		while (c != EOF)
		{
			if (v == 1) // if verbose, also print to console
				printf("%c", c);

			fputc(c, out);
			c = fgetc(vmtrace);
		}

		fclose(in2);
		fclose(vmtrace);
	
	}

	printf("For fully detailed description of scanning, parsing, and execution, see \"out.txt\".\n");

	fclose(out);

	return 0;
}