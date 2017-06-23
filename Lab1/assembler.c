/*

Name 1: Danial Rizvi
Name 2: Travis Lenz
UTEID 1: dr28944
UTEID 2: tal859

*/



/*LIBRARIES*/
#include <stdio.h> /* standard input/output library */
#include <stdlib.h> /* Standard C Library */
#include <string.h> /* String operations library */
#include <ctype.h> /* Library for useful character operations */
#include <limits.h> /* Library for definitions of common variable type characteristics */
#include <stdint.h>

/*DEFINITIONS*/
#define MAX_LABEL_LEN 20
#define MAX_SYMBOLS 255
#define MAX_LINE_LEN 255

/*FUNCTIONS*/
int arg2opc(char* token); /*converts opcode to numerical value Ex. Add = 1, also detects labels*/
void parse(void); /*separates instruction into label, opcode, and operands*/

int add(void);
int and(void);
int br_nzp(void);
int jmp_ret(void);
int jsr_r(void);
int ldb(void);
int ldw(void);
int lea(void);
int not_xor(void);
int rti(void);
int lshf_rshf_la(void);
int stb(void);
int stw(void);
int trap_halt(void);
int orig(void);
int fill(void);
int nop(void);

void baseaddress(void);
int convert(void);

/*DATA STRUCTURES*/

/*Symbol_Label Table*/
typedef struct {
	int address;
	char label[MAX_LINE_LEN + 1]; /* Question for the reader: Why do we need to add 1? */
} TableEntry;
TableEntry LabelTable[MAX_SYMBOLS];

/*Arguments Array which holds label, opcode, or operand*/
typedef struct {
	char *Tok;
} Tokens;
Tokens Arg[6] = { "","","","","",""};

/*File I/O*/
FILE* infile = NULL;  /*BOTH*/
FILE* outfile = NULL; /*BOTH*/
char ifilename[50]; /* WINDOWS*/
char ofilename[50]; /* WINDOWS*/

char Line[MAX_LINE_LEN + 1]; /*Holds one line which includes instruction and any label or comments*/

/*VARIABLES*/
int LineNum = 0;
int BaseAddress = 0;
int LineAddress;
int i = 0;
int labindex = 0;
int opcval;
uint16_t Instr;
int labelpresent;
int reg;
int origfound = 0;

/*MAIN PROGRAM*/

int main(int argc, char* argv[]) {

	/* LINUX LINUX LINUX LINUX LINUX LINUX LINUX LINUX*/
	/*
	char *prgName   = NULL;
	char *iFileName = NULL;
	char *oFileName = NULL;

	prgName   = argv[0];
	iFileName = argv[1];
	oFileName = argv[2];

	printf("program name = '%s'\n", prgName);
	printf("input file name = '%s'\n", iFileName);
	printf("output file name = '%s'\n", oFileName);

	infile = fopen(argv[1], "r");
	outfile = fopen(argv[2], "w");

	if (!infile) {
	printf("Error: Cannot open file %s\n", argv[1]);
	exit(4);
	}
	if (!outfile) {
	printf("Error: Cannot open file %s\n", argv[2]);
	exit(4);
	}
	
	*/
	/*WINDOWS WINDOWS WINDOWS WINDOWS WINDOWS WINDOWS*/
	
	printf("Enter input file name (.asm file): ");
	scanf_s("%s", ifilename, 50);
	printf("Enter output file name (.obj file): ");
	scanf_s("%s", ofilename, 50);

	infile = fopen(ifilename, "r");
	outfile = fopen(ofilename, "w");

	if (!infile) {
		printf("Error: Cannot open file %s\n", ifilename);
		exit(4);
	}
	if (!outfile) {
		printf("Error: Cannot open file %s\n", ofilename);
		exit(4);
	}
	

	while (1) {

		/*FIRST PASS
		PARSE INSTRUCTIONS
		FILL LABEL TABLE*/

		parse();
		LineNum++;

		opcval = arg2opc(Arg[0].Tok); /*check for opcode/label*/

		i = 0;
		if (opcval == -2) { /*if opc is .orig find base/starting address*/
			baseaddress();

		}
		if (origfound != 1) {
			printf("Error: .ORIG can only be preceded by comments\n");
			exit(4);
		}
		LineAddress = BaseAddress + (2 * (LineNum - 2)); /*current line's address*/

		if (opcval == -1) { /*store label and its address in table*/

			/*ERROR CHECKING*/
							
			for (i = 0; i < MAX_SYMBOLS; i++) {
				if (strcmp(LabelTable[i].label, Arg[0].Tok) == 0) { 
					printf("Error: Label Repeated\n");
					exit(4);
				}
			}
			if ((Arg[0].Tok[0] < 'a') || (Arg[0].Tok[0] > 'z') || (Arg[0].Tok[0] == 'x')) {
				printf("Error: Label does not begin with letter or it begins with x\n");
				exit(1);
			}
			for (i = 0; Arg[0].Tok[i] != '\0'; i++) {
				if (isalnum(Arg[0].Tok[i]) == 0) {
					printf("Error: Label is not alphanumeric\n");
					exit(1);
				}
			}
			if ((strcmp(Arg[0].Tok, "in") == 0) || (strcmp(Arg[0].Tok, "out") == 0) ||
				(strcmp(Arg[0].Tok, "getc") == 0) || (strcmp(Arg[0].Tok, "puts") == 0)) {
				printf("Error: Label can not be IN, OUT, GETC, or PUTS\n");
				exit(1);
			}
			
			

			LabelTable[labindex].address = LineAddress;
			strcpy(LabelTable[labindex].label, Arg[0].Tok);
			
			labindex++;
			
			if(arg2opc(Arg[1].Tok) == -2){
				baseaddress();
			}
			if(arg2opc(Arg[1].Tok) == -4){
				break;
			}
		}

		if (opcval == -4) {
			break; /*break out of while loop if it reachs ".end"*/
		}
	}



	rewind(infile); /*rewind for second pass*/
	LineNum = 0;

	while (1) {
		int j;
		/*SECOND PASS
		CONVERT INSTRUCTIONS INTO MACHINE CODE
		WRITE TO OUTPUT FILE*/

		parse();
		LineNum++;

		opcval = arg2opc(Arg[0].Tok);
		LineAddress = BaseAddress + (2 * (LineNum - 2)); /*current line's address*/
		labelpresent = 0;
		if (opcval == -1) { /*first argument was label*/

			for (i = 0; i<5; i++) {
				Arg[i].Tok = Arg[i + 1].Tok;
			}
			Arg[5].Tok = "\0";

			opcval = arg2opc(Arg[0].Tok);
			if (opcval == -1) {
				printf("Error: Invalid Opcode\n");
				exit(2);
			}
		}

		if (Arg[4].Tok != '\0') {
			printf("Error: Too many operands\n");
			exit(4);
		}

		/*Based on opcode and operands convert instruction into
		integer value that equals the binary value of the
		instruction in machine code*/

		for(i = 1; i < 4; i++){
			if (Arg[i].Tok != '\0') {
				if (Arg[i].Tok[0] == 'r') {
					reg = atoi(&(Arg[i].Tok[1]));
					if ((reg < 0) || (reg > 7)) {
						printf("Error: Invalid Register\n");
						exit(4);
					}
				}
			}
		}
		

		if (opcval == -4) {
			break; /*reached .end*/
		}
		convert();
		
		fprintf(outfile, "0x%.4X\n", Instr);
	}

	fclose(infile);
	fclose(outfile);


}


int arg2opc(char *token) {
	if (token == '\0') {
		return -6;
	}
	if (strcmp(token, "add") == 0) {
		return 1;
	}
	if (strcmp(token, "and") == 0) {
		return 5;
	}
	if ((strcmp(token, "br") == 0) || (strcmp(token, "brp") == 0) || (strcmp(token, "brz") == 0) || (strcmp(token, "brzp") == 0) ||
		(strcmp(token, "brn") == 0) || (strcmp(token, "brnp") == 0) || (strcmp(token, "brnz") == 0) || (strcmp(token, "brnzp") == 0)) {
		return 0;
	}
	if ((strcmp(token, "jmp") == 0) || (strcmp(token, "ret") == 0)) {
		return 12;
	}
	if ((strcmp(token, "jsr") == 0) || (strcmp(token, "jsrr") == 0)) {
		return 4;
	}
	if (strcmp(token, "ldb") == 0) {
		return 2;
	}
	if (strcmp(token, "ldw") == 0) {
		return 6;
	}
	if (strcmp(token, "lea") == 0) {
		return 14;
	}
	if ((strcmp(token, "not") == 0) || (strcmp(token, "xor") == 0)) {
		return 9;
	}
	if (strcmp(token, "rti") == 0) {
		return 8;
	}
	if ((strcmp(token, "lshf") == 0) || (strcmp(token, "rshfl") == 0) || (strcmp(token, "rshfa") == 0)) {
		return 13;
	}
	if (strcmp(token, "stb") == 0) {
		return 3;
	}
	if (strcmp(token, "stw") == 0) {
		return 7;
	}
	if ((strcmp(token, "trap") == 0) || (strcmp(token, "halt") == 0)) {
		return 15;
	}


	if (strcmp(token, ".orig") == 0) {
		return -2;
	}
	if (strcmp(token, ".fill") == 0) {
		return -3;
	}
	if (strcmp(token, ".end") == 0) {
		return -4;
	}
	if (strcmp(token, "nop") == 0) {
		return -5;
	}

	return -1;
}

void parse(void) {
	fgets(Line, MAX_LINE_LEN, infile); /*get one line of instruction*/
	for (i = 0; i < strlen(Line); i++) {
		Line[i] = tolower(Line[i]); /*convert to lowercase*/
	}
	/*remove comments from instruction*/
	i = 0;
	while ((Line[i] != ';') && (Line[i] != '\0') && (Line[i] != '\n')) {
		i++;
	}
	Line[i] = '\0';
	/*separate arguments in instruction*/
	Arg[0].Tok = strtok(Line, "\t\n\r ,");
	i = 1;
	while ((Arg[i - 1].Tok != '\0') && (i < 6)) {
		Arg[i].Tok = strtok('\0', "\t\n\r ,");
		i++;
	}
	/*set everything after end of instruction ('\0') to '\0'*/
	i = 0;
	while((Arg[i].Tok != '\0') && (i < 6)){
		i++;
	}
	while (i < 6) {
		Arg[i].Tok = '\0';
		i++;
	}

	if (Arg[0].Tok == '\0') {
		parse();
	}

}

void baseaddress(void){
	if (origfound == 1) {
		printf("Error: .ORIG used more than once\n");
		exit(4);
	}
	while ((Arg[1].Tok[i] != '#') && (Arg[1].Tok[i] != 'x')) {
		i++;
	}

	if (Arg[1].Tok[i] == '#') {
		BaseAddress = atoi(&(Arg[1].Tok[i + 1]));
	}
	if (Arg[1].Tok[i] == 'x') {
		BaseAddress = strtol(&(Arg[1].Tok[i + 1]), '\0', 16);
	}

	if ((BaseAddress < 0) || (BaseAddress > 65535)) {	/*.ORIG has invalid starting address*/
		printf("Error: Base Address Invalid");
		exit(3);
	}

	if ((BaseAddress % 2) != 0) { /*.ORIG is not word aligned*/
		printf("Error: Base Address in front of .ORIG is odd\n");
		exit(3);
	}
	LineNum = 1;
	origfound = 1;
}

int convert(void) {
	if (opcval == 1) {
		Instr = add();
	}
	if (opcval == 5) {
		Instr = and ();
	}
	if (opcval == 0) {
		Instr = br_nzp();
	}
	if (opcval == 12) {
		Instr = jmp_ret();
	}
	if (opcval == 4) {
		Instr = jsr_r();
	}
	if (opcval == 2) {
		Instr = ldb();
	}
	if (opcval == 6) {
		Instr = ldw();
	}
	if (opcval == 14) {
		Instr = lea();
	}
	if (opcval == 9) {
		Instr = not_xor();
	}
	if (opcval == 8) {
		Instr = rti();
	}
	if (opcval == 13) {
		Instr = lshf_rshf_la();
	}
	if (opcval == 3) {
		Instr = stb();
	}
	if (opcval == 7) {
		Instr = stw();
	}
	if (opcval == 15) {
		Instr = trap_halt();
	}
	if (opcval == -2) {
		Instr = orig();
	}
	if (opcval == -3) {
		Instr = fill();
	}
	if (opcval == -5) {
		Instr = nop();
	}
}

int add(void) {
	int DR;
	int SR1;
	int SR2;
	int imm5;

	if ((Arg[1].Tok == '\0') || (Arg[2].Tok == '\0') || (Arg[3].Tok == '\0')) {
		printf("Error (ADD): Missing Operands\n");
		exit(4);
	}

	if ((Arg[1].Tok[0] != 'r') || (Arg[2].Tok[0] != 'r')) {
		printf("Error (ADD): Invalid Operands\n");
		exit(4);
	}
	if ((Arg[3].Tok[0] != 'r') && (Arg[3].Tok[0] != '#') && (Arg[3].Tok[0] != 'x')) {
		printf("Error (ADD): Invalid Operands\n");
		exit(4);
	}


	DR = atoi(&(Arg[1].Tok[1]));
	SR1 = atoi(&(Arg[2].Tok[1]));

	if (Arg[3].Tok[0] == 'r') {
		SR2 = atoi(&(Arg[3].Tok[1]));
		return ((1 << 12) | (DR << 9) | (SR1 << 6) | (SR2));
	}

	else {
		i = 0;
		while ((Arg[3].Tok[i] != '#') && (Arg[3].Tok[i] != 'x')) {
			i++;
		}
		if (Arg[3].Tok[i] == '#') {
			imm5 = atoi(&(Arg[3].Tok[i + 1]));
		}
		if (Arg[3].Tok[i] == 'x') {
			imm5 = strtol(&(Arg[3].Tok[i + 1]), '\0', 16);
		}
		if ((imm5 > 15) || (imm5 < -16)) {
			printf("Error (ADD): Constant too large");
			exit(3);
		}
		imm5 = imm5 & 0x01F;
		return ((1 << 12) | (DR << 9) | (SR1 << 6) | (1 << 5) | (imm5));
	}

}

int and(void) {
	int DR;
	int SR1;
	int SR2;
	int imm5;

	if ((Arg[1].Tok == '\0') || (Arg[2].Tok == '\0') || (Arg[3].Tok == '\0')) {
		printf("Error (AND): Missing Operands\n");
		exit(4);
	}

	if ((Arg[1].Tok[0] != 'r') || (Arg[2].Tok[0] != 'r')) {
		printf("Error (AND): Invalid Operands\n");
		exit(4);
	}
	if ((Arg[3].Tok[0] != 'r') && (Arg[3].Tok[0] != '#') && (Arg[3].Tok[0] != 'x')) {
		printf("Error (AND): Invalid Operands\n");
		exit(4);
	}

	DR = atoi(&(Arg[1].Tok[1]));
	SR1 = atoi(&(Arg[2].Tok[1]));

	if (Arg[3].Tok[0] == 'r') {
		SR2 = atoi(&(Arg[3].Tok[1]));
		
		return ((5 << 12) | (DR << 9) | (SR1 << 6) | (SR2));
	}

	else {
		i = 0;
		while ((Arg[3].Tok[i] != '#') && (Arg[3].Tok[i] != 'x')) {
			i++;
		}
		if (Arg[3].Tok[i] == '#') {
			imm5 = atoi(&(Arg[3].Tok[i + 1]));
		}
		if (Arg[3].Tok[i] == 'x') {
			imm5 = strtol(&(Arg[3].Tok[i + 1]), '\0', 16);
		}
		if ((imm5 > 15) || (imm5 < -16)) {
			printf("Error (AND): Constant too large");
			exit(3);
		}
		imm5 = imm5 & 0x01F;
		return ((5 << 12) | (DR << 9) | (SR1 << 6) | (1 << 5) | (imm5));
	}

}

int jmp_ret(void) {
	int BaseR;

	if (strcmp(Arg[0].Tok, "jmp") == 0) {

		if ((Arg[1].Tok == '\0')) {
			printf("Error (JMP): Missing Operands\n");
			exit(4);
		}

		if ((Arg[2].Tok != '\0') || (Arg[3].Tok != '\0')) {
			printf("Error (JMP): Too many Operands\n");
			exit(4);
		}

		if ((Arg[1].Tok[0] != 'r')) {
			printf("Error (JMP): Invalid Operands\n");
			exit(4);
		}

		BaseR = atoi(&(Arg[1].Tok[1]));
		
		return ((12 << 12) | (BaseR << 6));
	}

	if (strcmp(Arg[0].Tok, "ret") == 0) {

		if ((Arg[1].Tok != '\0') || (Arg[2].Tok != '\0') || (Arg[3].Tok != '\0')) {
			printf("Error (RET): Too many Operands\n");
			exit(4);
		}

		return ((12 << 12) | (7 << 6));
	}

}

int not_xor(void) {
	int DR;
	int SR;
	int SR1;
	int SR2;
	int imm5;

	if (strcmp(Arg[0].Tok, "not") == 0) {

		if ((Arg[1].Tok == '\0') || (Arg[2].Tok == '\0')) {
			printf("Error (NOT): Missing Operands\n");
			exit(4);
		}

		if ((Arg[3].Tok != '\0')) {
			printf("Error (NOT): Too many Operands\n");
			exit(4);
		}

		if ((Arg[1].Tok[0] != 'r') || (Arg[2].Tok[0] != 'r')) {
			printf("Error (NOT): Invalid Operands\n");
			exit(4);
		}

		DR = atoi(&(Arg[1].Tok[1]));
		SR = atoi(&(Arg[2].Tok[1]));
		return ((9 << 12) | (DR << 9) | (SR << 6) | (63));
	}

	if (strcmp(Arg[0].Tok, "xor") == 0) {

		if ((Arg[1].Tok == '\0') || (Arg[2].Tok == '\0') || (Arg[3].Tok == '\0')) {
			printf("Error (XOR): Missing Operands\n");
			exit(4);
		}

		if ((Arg[1].Tok[0] != 'r') || (Arg[2].Tok[0] != 'r')) {
			printf("Error (XOR): Invalid Operands\n");
			exit(4);
		}
		if ((Arg[3].Tok[0] != 'r') && (Arg[3].Tok[0] != '#') && (Arg[3].Tok[0] != 'x')) {
			printf("Error (XOR): Invalid Operands\n");
			exit(4);
		}

		DR = atoi(&(Arg[1].Tok[1]));
		SR1 = atoi(&(Arg[2].Tok[1]));

		if (Arg[3].Tok[0] == 'r') {
			SR2 = atoi(&(Arg[3].Tok[1]));
			return ((9 << 12) | (DR << 9) | (SR1 << 6) | (SR2));
		}

		else {
			i = 0;
			while ((Arg[3].Tok[i] != '#') && (Arg[3].Tok[i] != 'x')) {
				i++;
			}
			if (Arg[3].Tok[i] == '#') {
				imm5 = atoi(&(Arg[3].Tok[i + 1]));
			}
			if (Arg[3].Tok[i] == 'x') {
				imm5 = strtol(&(Arg[3].Tok[i + 1]), '\0', 16);
			}
			if ((imm5 > 15) || (imm5 < -16)) {
				printf("Error (XOR): Constant too large");
				exit(3);
			}
			imm5 = imm5 & 0x01F;
			return ((9 << 12) | (DR << 9) | (SR1 << 6) | (1 << 5) | (imm5));
		}

	}

}

int rti(void) {

	if ((Arg[1].Tok != '\0') || (Arg[2].Tok != '\0') || (Arg[3].Tok != '\0')) {
		printf("Error (RTI): Too many Operands\n");
		exit(4);
	}

	return (8 << 12);

}

int lshf_rshf_la(void) {

	int DR;
	int SR;
	int amount4;

	if ((Arg[1].Tok == '\0') || (Arg[2].Tok == '\0') || (Arg[3].Tok == '\0')) {
		printf("Error (LSHF/RSHFL/RSHFA): Missing Operands\n");
		exit(4);
	}

	if ((Arg[1].Tok[0] != 'r') || (Arg[2].Tok[0] != 'r')) {
		printf("Error (LSHF/RSHFL/RSHFA): Invalid Operands\n");
		exit(4);
	}
	if ((Arg[3].Tok[0] != '#') && (Arg[3].Tok[0] != 'x')) {
		printf("Error (LSHF/RSHFL/RSHFA): Invalid Operands\n");
		exit(4);
	}

	DR = atoi(&(Arg[1].Tok[1]));
	SR = atoi(&(Arg[2].Tok[1]));

	i = 0;
	while ((Arg[3].Tok[i] != '#') && (Arg[3].Tok[i] != 'x')) {
		i++;
	}
	if (Arg[3].Tok[i] == '#') {
		amount4 = atoi(&(Arg[3].Tok[i + 1]));
	}
	if (Arg[3].Tok[i] == 'x') {
		amount4 = strtol(&(Arg[3].Tok[i + 1]), '\0', 16);
	}
	if ((amount4 > 7) || (amount4 < 0)) {
		printf("Error (LSHF/RSHFL/RSHFA): Constant too large or nonzero");
		exit(3);
	}

	amount4 = amount4 & 0x0F;

	if (strcmp(Arg[0].Tok, "lshf") == 0) {
		return ((13 << 12) | (DR << 9) | (SR << 6) | (amount4));
	}

	if (strcmp(Arg[0].Tok, "rshfl") == 0) {
		return ((13 << 12) | (DR << 9) | (SR << 6) | (1 << 4) | (amount4));
	}

	if (strcmp(Arg[0].Tok, "rshfa") == 0) {
		return ((13 << 12) | (DR << 9) | (SR << 6) | (3 << 4) | (amount4));
	}


}

int trap_halt(void) {

	int trapvector8;

	if (strcmp(Arg[0].Tok, "trap") == 0) {

		if ((Arg[1].Tok == '\0')) {
			printf("Error (TRAP): Missing Operands\n");
			exit(4);
		}

		if ((Arg[2].Tok != '\0') || (Arg[3].Tok != '\0')) {
			printf("Error (TRAP): Too many Operands\n");
			exit(4);
		}

		if ((Arg[1].Tok[0] != 'x')) {
			printf("Error (TRAP): Invalid Operands\n");
			exit(4);
		}
		
		i = 0;
		while ((Arg[1].Tok[i] != '#') && (Arg[1].Tok[i] != 'x')) {
			i++;
		}
		
		if (Arg[1].Tok[i] == 'x') {
			trapvector8 = strtol(&(Arg[1].Tok[i + 1]), '\0', 16);
		}

		if (Arg[1].Tok[i] == '#') {
			printf("Error (TRAP): Invalid Operand\n");
			exit(4);
		}

		if (trapvector8 < 0) {
			printf("Error (TRAP): Invalid constant\n");
			exit(3);
		}

		trapvector8 = trapvector8 & 0x0FF;

		return ((15 << 12) | (trapvector8));
	}

	if (strcmp(Arg[0].Tok, "halt") == 0) {

		if ((Arg[1].Tok != '\0') || (Arg[2].Tok != '\0') || (Arg[3].Tok != '\0')) {
			printf("Error (HALT): Too many Operands\n");
			exit(4);
		}

		return ((15 << 12) | (37));
	}

}

int orig(void) {

	int address;

	if ((Arg[1].Tok == '\0')) {
		printf("Error (ORIG): Missing Operands\n");
		exit(4);
	}

	if ((Arg[2].Tok != '\0') || (Arg[3].Tok != '\0')) {
		printf("Error (ORIG): Too many Operands\n");
		exit(4);
	}

	if ((Arg[1].Tok[0] != '#') && (Arg[1].Tok[0] != 'x')) {
		printf("Error (ORIG): Invalid Operands\n");
		exit(4);
	}

	i = 0;
	while ((Arg[1].Tok[i] != '#') && (Arg[1].Tok[i] != 'x')) {
		i++;
	}
	if (Arg[1].Tok[i] == '#') {
		address = atoi(&(Arg[1].Tok[i + 1]));
	}
	if (Arg[1].Tok[i] == 'x') {
		address = strtol(&(Arg[1].Tok[i + 1]), '\0', 16);
	}

	return address;

}

int fill(void) {
	int value;

	if ((Arg[1].Tok == '\0')) {
		printf("Error (FILL): Missing Operands\n");
		exit(4);
	}

	if ((Arg[2].Tok != '\0') || (Arg[3].Tok != '\0')) {
		printf("Error (FILL): Too many Operands\n");
		exit(4);
	}

	if ((Arg[1].Tok[0] != '#') && (Arg[1].Tok[0] != 'x')) {
		printf("Error (FILL): Invalid Operands\n");
		exit(4);
	}

	i = 0;
	while ((Arg[1].Tok[i] != '#') && (Arg[1].Tok[i] != 'x')) {
		i++;
	}
	if (Arg[1].Tok[i] == '#') {
		value = atoi(&(Arg[1].Tok[i + 1]));
	}
	if (Arg[1].Tok[i] == 'x') {
		value = strtol(&(Arg[1].Tok[i + 1]), '\0', 16);
	}

	return value;

}

int nop(void) {

	if ((Arg[1].Tok != '\0') || (Arg[2].Tok != '\0') || (Arg[3].Tok != '\0')) {
		printf("Error (NOP): Too many Operands\n");
		exit(4);
	}

	return 0;
}


int br_nzp(void) {
	int nzp;
	int LabelAddress;
	int PCoffset9;

	if ((Arg[1].Tok == '\0')) {
		printf("Error (BR): Missing Operands\n");
		exit(4);
	}

	if ((Arg[2].Tok != '\0') || (Arg[3].Tok != '\0')) {
		printf("Error (BR): Too many Operands\n");
		exit(4);
	}

	if ((strcmp(Arg[0].Tok, "br") == 0) || (strcmp(Arg[0].Tok, "brnzp") == 0)) {
		nzp = 7;
	}
	if (strcmp(Arg[0].Tok, "brn") == 0) {
		nzp = 4;
	}
	if (strcmp(Arg[0].Tok, "brz") == 0) {
		nzp = 2;
	}
	if (strcmp(Arg[0].Tok, "brp") == 0) {
		nzp = 1;
	}
	if (strcmp(Arg[0].Tok, "brnz") == 0) {
		nzp = 6;
	}
	if (strcmp(Arg[0].Tok, "brnp") == 0) {
		nzp = 5;
	}
	if (strcmp(Arg[0].Tok, "brzp") == 0) {
		nzp = 3;
	}

	LabelAddress = 0;
	for (i = 0; i < MAX_SYMBOLS; i++) {
		if (strcmp(Arg[1].Tok, LabelTable[i].label) == 0) {
			LabelAddress = LabelTable[i].address;
		}
	}
	if (LabelAddress == 0) {
		printf("Error (BR): Undefined Label, Label not in Table\n");
		exit(1);
	}

	PCoffset9 = ((LabelAddress - (LineAddress + 2)) / 2);

	if ((PCoffset9 > 255) || (PCoffset9 < -256)) {
		printf("Error (BR): Label Offset too large");
		exit(4);
	}

	PCoffset9 = PCoffset9 & 0x1FF;

	return ((nzp << 9) | (PCoffset9));

}

int lea(void) {
	int DR;
	int LabelAddress;
	int PCoffset9;

	if ((Arg[1].Tok == '\0') || (Arg[2].Tok == '\0')) {
		printf("Error (LEA): Missing Operands\n");
		exit(4);
	}

	if ((Arg[3].Tok != '\0')) {
		printf("Error (LEA): Too many Operands\n");
		exit(4);
	}

	if ((Arg[1].Tok[0] != 'r')) {
		printf("Error (LEA): Invalid Operands\n");
		exit(4);
	}
	
	DR = atoi(&(Arg[1].Tok[1]));

	LabelAddress = 0;
	for (i = 0; i < MAX_SYMBOLS; i++) {
		if (strcmp(Arg[2].Tok, LabelTable[i].label) == 0) {
			LabelAddress = LabelTable[i].address;
		}
	}
	if (LabelAddress == 0) {
		printf("Error (LEA): Undefined Label, Label not in Table\n");
		exit(1);
	}

	PCoffset9 = ((LabelAddress - (LineAddress + 2)) / 2);

	if ((PCoffset9 > 255) || (PCoffset9 < -256)) {
		printf("Error (LEA): Label Offset too large");
		exit(4);
	}

	PCoffset9 = PCoffset9 & 0x1FF;

	return ((14 << 12) | (DR << 9) | (PCoffset9));

}

int jsr_r(void) {
	int BaseR;
	int LabelAddress;
	int PCoffset11;

	if (strcmp(Arg[0].Tok, "jsrr") == 0) {

		if ((Arg[1].Tok == '\0')) {
			printf("Error (JSRR): Missing Operands\n");
			exit(4);
		}

		if ((Arg[2].Tok != '\0') || (Arg[3].Tok != '\0')) {
			printf("Error (JSRR): Too many Operands\n");
			exit(4);
		}

		if ((Arg[1].Tok[0] != 'r')) {
			printf("Error (JSRR): Invalid Operands\n");
			exit(4);
		}

		BaseR = atoi(&(Arg[1].Tok[1]));
		return ((4 << 12) | (BaseR << 6));
	}

	if (strcmp(Arg[0].Tok, "jsr") == 0) {

		if ((Arg[1].Tok == '\0')) {
			printf("Error (JSR): Missing Operands\n");
			exit(4);
		}

		if ((Arg[2].Tok != '\0') || (Arg[3].Tok != '\0')) {
			printf("Error (JSR): Too many Operands\n");
			exit(4);
		}

		LabelAddress = 0;
		for (i = 0; i < MAX_SYMBOLS; i++) {
			if (strcmp(Arg[1].Tok, LabelTable[i].label) == 0) {
				LabelAddress = LabelTable[i].address;
			}
		}
		if (LabelAddress == 0) {
			printf("Error (JSR): Undefined Label, Label not in Table\n");
			exit(1);
		}

		PCoffset11 = ((LabelAddress - (LineAddress + 2)) / 2);

		if ((PCoffset11 > 1023) || (PCoffset11 < -1024)) {
			printf("Error (JSR): Label Offset too large");
			exit(4);
		}

		PCoffset11 = PCoffset11 & 0x7FF;

		

		return ((4 << 12) | (1 << 11) | (PCoffset11));

	}

}


int ldb(void) {

	int DR;
	int BaseR;
	int boffset6;

	if ((Arg[1].Tok == '\0') || (Arg[2].Tok == '\0') || (Arg[3].Tok == '\0')) {
		printf("Error (LDB): Missing Operands\n");
		exit(4);
	}

	if ((Arg[1].Tok[0] != 'r') || (Arg[2].Tok[0] != 'r')) {
		printf("Error (LDB): Invalid Operands\n");
		exit(4);
	}
	if ((Arg[3].Tok[0] != '#') && (Arg[3].Tok[0] != 'x')) {
		printf("Error (LDB): Invalid Operands\n");
		exit(4);
	}

	DR = atoi(&(Arg[1].Tok[1]));
	BaseR = atoi(&(Arg[2].Tok[1]));

	i = 0;
	while ((Arg[3].Tok[i] != '#') && (Arg[3].Tok[i] != 'x')) {
		i++;
	}
	if (Arg[3].Tok[i] == '#') {
		boffset6 = atoi(&(Arg[3].Tok[i + 1]));
	}
	if (Arg[3].Tok[i] == 'x') {
		boffset6 = strtol(&(Arg[3].Tok[i + 1]), '\0', 16);
	}
	if ((boffset6 > 31) || (boffset6 < -32)) {
		printf("Error (LDB): Offset too large");
		exit(4);
	}

	boffset6 = boffset6 & 0x3F;

	return ((2 << 12) | (DR << 9) | (BaseR << 6) | (boffset6));

}

int ldw(void) {
	int DR;
	int BaseR;
	int boffset6;

	if ((Arg[1].Tok == '\0') || (Arg[2].Tok == '\0') || (Arg[3].Tok == '\0')) {
		printf("Error (LDW): Missing Operands\n");
		exit(4);
	}

	if ((Arg[1].Tok[0] != 'r') || (Arg[2].Tok[0] != 'r')) {
		printf("Error (LDW): Invalid Operands\n");
		exit(4);
	}
	if ((Arg[3].Tok[0] != '#') && (Arg[3].Tok[0] != 'x')) {
		printf("Error (LDW): Invalid Operands\n");
		exit(4);
	}

	DR = atoi(&(Arg[1].Tok[1]));
	BaseR = atoi(&(Arg[2].Tok[1]));

	i = 0;
	while ((Arg[3].Tok[i] != '#') && (Arg[3].Tok[i] != 'x')) {
		i++;
	}
	if (Arg[3].Tok[i] == '#') {
		boffset6 = atoi(&(Arg[3].Tok[i + 1]));
	}
	if (Arg[3].Tok[i] == 'x') {
		boffset6 = strtol(&(Arg[3].Tok[i + 1]), '\0', 16);
	}
	if ((boffset6 > 31) || (boffset6 < -32)) {
		printf("Error (LDW): Offset too large");
		exit(4);
	}

	boffset6 = boffset6 & 0x3F;

	return ((6 << 12) | (DR << 9) | (BaseR << 6) | (boffset6));

}

int stb(void) {
	int SR;
	int BaseR;
	int boffset6;

	if ((Arg[1].Tok == '\0') || (Arg[2].Tok == '\0') || (Arg[3].Tok == '\0')) {
		printf("Error (STB): Missing Operands\n");
		exit(4);
	}

	if ((Arg[1].Tok[0] != 'r') || (Arg[2].Tok[0] != 'r')) {
		printf("Error (STB): Invalid Operands\n");
		exit(4);
	}
	if ((Arg[3].Tok[0] != '#') && (Arg[3].Tok[0] != 'x')) {
		printf("Error (STB): Invalid Operands\n");
		exit(4);
	}

	SR = atoi(&(Arg[1].Tok[1]));
	BaseR = atoi(&(Arg[2].Tok[1]));

	i = 0;
	while ((Arg[3].Tok[i] != '#') && (Arg[3].Tok[i] != 'x')) {
		i++;
	}
	if (Arg[3].Tok[i] == '#') {
		boffset6 = atoi(&(Arg[3].Tok[i + 1]));
	}
	if (Arg[3].Tok[i] == 'x') {
		boffset6 = strtol(&(Arg[3].Tok[i + 1]), '\0', 16);
	}
	if ((boffset6 > 31) || (boffset6 < -32)) {
		printf("Error (STB): Offset too large");
		exit(4);
	}

	boffset6 = boffset6 & 0x3F;

	return ((3 << 12) | (SR << 9) | (BaseR << 6) | (boffset6));

}

int stw(void) {
	int SR;
	int BaseR;
	int boffset6;

	if ((Arg[1].Tok == '\0') || (Arg[2].Tok == '\0') || (Arg[3].Tok == '\0')) {
		printf("Error (STW): Missing Operands\n");
		exit(4);
	}

	if ((Arg[1].Tok[0] != 'r') || (Arg[2].Tok[0] != 'r')) {
		printf("Error (STW): Invalid Operands\n");
		exit(4);
	}
	if ((Arg[3].Tok[0] != '#') && (Arg[3].Tok[0] != 'x')) {
		printf("Error (STW): Invalid Operands\n");
		exit(4);
	}

	SR = atoi(&(Arg[1].Tok[1]));
	BaseR = atoi(&(Arg[2].Tok[1]));

	i = 0;
	while ((Arg[3].Tok[i] != '#') && (Arg[3].Tok[i] != 'x')) {
		i++;
	}
	if (Arg[3].Tok[i] == '#') {
		boffset6 = atoi(&(Arg[3].Tok[i + 1]));
	}
	if (Arg[3].Tok[i] == 'x') {
		boffset6 = strtol(&(Arg[3].Tok[i + 1]), '\0', 16);
	}
	if ((boffset6 > 31) || (boffset6 < -32)) {
		printf("Error (STW): Offset too large");
		exit(4);
	}

	boffset6 = boffset6 & 0x3F;

	return ((7 << 12) | (SR << 9) | (BaseR << 6) | (boffset6));
}





