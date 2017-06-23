/*
    Name 1: Travis Lenz
    Name 2: Danial Rizvi 
    UTEID 1: tal859
    4TEID 2: dr28944
*/

/***************************************************************/
/*                                                             */
/*   LC-3b Instruction Level Simulator                         */
/*                                                             */
/*   EE 460N                                                   */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/***************************************************************/
/*                                                             */
/* Files: isaprogram   LC-3b machine language program file     */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void process_instruction();
void execute_ADD();
void execute_AND();
void execute_BR();
void execute_JMP_RET();
void execute_JSR_R();
void execute_LDB();
void execute_LDW();
void execute_LEA();
void execute_NOT_XOR();
void execute_SHF();
void execute_STB();
void execute_STW();
void execute_TRAP();

/*helper functions*/
void set_cc(int val);
int sext(int val, int mask_bit);
int ASR(int x, int n);
int LSR(int x, int n);



int ir=0;		/*ir will serve as the instruction register*/

typedef enum{
	ADD = 0x00000001,
	AND = 0x00000005,
	BR =  0x00000000,
	JMP_RET = 0x0000000C,
	JSR_R = 0x00000004,
	LDB = 0x00000002,
	LDW = 0x00000006,
	LEA = 0x0000000E,
	NOT_XOR = 0x00000009,
	SHF = 0x0000000D,
	STB = 0x00000003,
	STW = 0x00000007,
	TRAP = 0x0000000F
}ops;

/***************************************************************/
/* A couple of useful definitions.                             */
/***************************************************************/
#define FALSE 0
#define TRUE  1

/***************************************************************/
/* Use this to avoid overflowing 16 bits on the bus.           */
/***************************************************************/
#define Low16bits(x) ((x) & 0xFFFF)

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A 
*/

#define WORDS_IN_MEM    0x08000 
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT;	/* run bit */


typedef struct System_Latches_Struct{

  int PC,		/* program counter */
    N,		/* n condition bit */
    Z,		/* z condition bit */
    P;		/* p condition bit */
  int REGS[LC_3b_REGS]; /* register file. */
} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int INSTRUCTION_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands                    */
/*                                                             */
/***************************************************************/
void help() {                                                    
  printf("----------------LC-3b ISIM Help-----------------------\n");
  printf("go               -  run program to completion         \n");
  printf("run n            -  execute program for n instructions\n");
  printf("mdump low high   -  dump memory from low to high      \n");
  printf("rdump            -  dump the register & bus values    \n");
  printf("?                -  display this help menu            \n");
  printf("quit             -  exit the program                  \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {                                                

  process_instruction();
  CURRENT_LATCHES = NEXT_LATCHES;
  INSTRUCTION_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3b for n cycles                 */
/*                                                             */
/***************************************************************/
void run(int num_cycles) {                                      
  int i;

  if (RUN_BIT == FALSE) {
    printf("Can't simulate, Simulator is halted\n\n");
    return;
  }

  printf("Simulating for %d cycles...\n\n", num_cycles);
  for (i = 0; i < num_cycles; i++) {
    if (CURRENT_LATCHES.PC == 0x0000) {
	    RUN_BIT = FALSE;
	    printf("Simulator halted\n\n");
	    break;
    }
    cycle();
  }
}

/***************************************************************/
/*                                                             */
/* Procedure : go                                              */
/*                                                             */
/* Purpose   : Simulate the LC-3b until HALTed                 */
/*                                                             */
/***************************************************************/
void go() {                                                     
  if (RUN_BIT == FALSE) {
    printf("Can't simulate, Simulator is halted\n\n");
    return;
  }

  printf("Simulating...\n\n");
  while (CURRENT_LATCHES.PC != 0x0000)
    cycle();
  RUN_BIT = FALSE;
  printf("Simulator halted\n\n");
}

/***************************************************************/ 
/*                                                             */
/* Procedure : mdump                                           */
/*                                                             */
/* Purpose   : Dump a word-aligned region of memory to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void mdump(FILE * dumpsim_file, int start, int stop) {          
  int address; /* this is a byte address */

  printf("\nMemory content [0x%.4x..0x%.4x] :\n", start, stop);
  printf("-------------------------------------\n");
  for (address = (start >> 1); address <= (stop >> 1); address++)
    printf("  0x%.4x (%d) : 0x%.2x%.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
  printf("\n");

  /* dump the memory contents into the dumpsim file */
  fprintf(dumpsim_file, "\nMemory content [0x%.4x..0x%.4x] :\n", start, stop);
  fprintf(dumpsim_file, "-------------------------------------\n");
  for (address = (start >> 1); address <= (stop >> 1); address++)
    fprintf(dumpsim_file, " 0x%.4x (%d) : 0x%.2x%.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
  fprintf(dumpsim_file, "\n");
  fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : rdump                                           */
/*                                                             */
/* Purpose   : Dump current register and bus values to the     */   
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void rdump(FILE * dumpsim_file) {                               
  int k; 

  printf("\nCurrent register/bus values :\n");
  printf("-------------------------------------\n");
  printf("Instruction Count : %d\n", INSTRUCTION_COUNT);
  printf("PC                : 0x%.4x\n", CURRENT_LATCHES.PC);
  printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
  printf("Registers:\n");
  for (k = 0; k < LC_3b_REGS; k++)
    printf("%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
  printf("\n");

  /* dump the state information into the dumpsim file */
  fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
  fprintf(dumpsim_file, "-------------------------------------\n");
  fprintf(dumpsim_file, "Instruction Count : %d\n", INSTRUCTION_COUNT);
  fprintf(dumpsim_file, "PC                : 0x%.4x\n", CURRENT_LATCHES.PC);
  fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
  fprintf(dumpsim_file, "Registers:\n");
  for (k = 0; k < LC_3b_REGS; k++)
    fprintf(dumpsim_file, "%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
  fprintf(dumpsim_file, "\n");
  fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : get_command                                     */
/*                                                             */
/* Purpose   : Read a command from standard input.             */  
/*                                                             */
/***************************************************************/
void get_command(FILE * dumpsim_file) {                         
  char buffer[20];
  int start, stop, cycles;

  printf("LC-3b-SIM> ");

  scanf("%s", buffer);
  printf("\n");

  switch(buffer[0]) {
  case 'G':
  case 'g':
    go();
    break;

  case 'M':
  case 'm':
    scanf("%i %i", &start, &stop);
    mdump(dumpsim_file, start, stop);
    break;

  case '?':
    help();
    break;
  case 'Q':
  case 'q':
    printf("Bye.\n");
    exit(0);

  case 'R':
  case 'r':
    if (buffer[1] == 'd' || buffer[1] == 'D')
	    rdump(dumpsim_file);
    else {
	    scanf("%d", &cycles);
	    run(cycles);
    }
    break;

  default:
    printf("Invalid Command\n");
    break;
  }
}

/***************************************************************/
/*                                                             */
/* Procedure : init_memory                                     */
/*                                                             */
/* Purpose   : Zero out the memory array                       */
/*                                                             */
/***************************************************************/
void init_memory() {                                           
  int i;

  for (i=0; i < WORDS_IN_MEM; i++) {
    MEMORY[i][0] = 0;
    MEMORY[i][1] = 0;
  }
}

/**************************************************************/
/*                                                            */
/* Procedure : load_program                                   */
/*                                                            */
/* Purpose   : Load program and service routines into mem.    */
/*                                                            */
/**************************************************************/
void load_program(char *program_filename) {                   
  FILE * prog;
  int ii, word, program_base;

  /* Open program file. */
  prog = fopen(program_filename, "r");
  if (prog == NULL) {
    printf("Error: Can't open program file %s\n", program_filename);
    exit(-1);
  }

  /* Read in the program. */
  if (fscanf(prog, "%x\n", &word) != EOF)
    program_base = word >> 1;
  else {
    printf("Error: Program file is empty\n");
    exit(-1);
  }

  ii = 0;
  while (fscanf(prog, "%x\n", &word) != EOF) {
    /* Make sure it fits. */
    if (program_base + ii >= WORDS_IN_MEM) {
	    printf("Error: Program file %s is too long to fit in memory. %x\n",
             program_filename, ii);
	    exit(-1);
    }

    /* Write the word to memory array. */
    MEMORY[program_base + ii][0] = word & 0x00FF;
    MEMORY[program_base + ii][1] = (word >> 8) & 0x00FF;
    ii++;
  }

  if (CURRENT_LATCHES.PC == 0) CURRENT_LATCHES.PC = (program_base << 1);

  printf("Read %d words from program into memory.\n\n", ii);
}

/************************************************************/
/*                                                          */
/* Procedure : initialize                                   */
/*                                                          */
/* Purpose   : Load machine language program                */ 
/*             and set up initial state of the machine.     */
/*                                                          */
/************************************************************/
void initialize(char *program_filename, int num_prog_files) { 
  int i;

  init_memory();
  for ( i = 0; i < num_prog_files; i++ ) {
    load_program(program_filename);
    while(*program_filename++ != '\0');
  }
  CURRENT_LATCHES.Z = 1;  
 NEXT_LATCHES = CURRENT_LATCHES;
    
  RUN_BIT = TRUE;
}

/***************************************************************/
/*                                                             */
/* Procedure : main                                            */
/*                                                             */
/***************************************************************/
char ifilename[50]; /* WINDOWS*/

int main(int argc, char *argv[]) {                              
  FILE * dumpsim_file;

  /* Error Checking */
  /*LINUX LINUX LINUX LINUX LINUX LINUX*/
  /*if (argc < 2) {
    printf("Error: usage: %s <program_file_1> <program_file_2> ...\n",
           argv[0]);
    exit(1);
  }

  printf("LC-3b Simulator\n\n");

  initialize(argv[1], argc - 1);*/

  /*WINDOWS WINDOWS WINDOWS WINDOWS WINDOWS*/
  printf("Enter input file name (.obj file): ");
  scanf_s("%s", ifilename, 50);

  initialize(ifilename, 1);
  /**/

  if ( (dumpsim_file = fopen( "dumpsim", "w" )) == NULL ) {
    printf("Error: Can't open dumpsim file\n");
    exit(-1);
  }

  while (1)
    get_command(dumpsim_file);
    
}

/***************************************************************/
/* Do not modify the above code.
   You are allowed to use the following global variables in your
   code. These are defined above.

   MEMORY

   CURRENT_LATCHES
   NEXT_LATCHES

   You may define your own local/global variables and functions.
   You may use the functions to get at the control bits defined
   above.

   Begin your code here 	  			       */

/***************************************************************/



void process_instruction(){
  /*  function: process_instruction
   *  
   *    Process one instruction at a time  
   *       -Fetch one instruction
   *       -Decode 
   *       -Execute
   */     
	int low_byte = 0;
	int high_byte = 0;

	NEXT_LATCHES.PC = CURRENT_LATCHES.PC + 2;		/*Increment PC*/

	low_byte = MEMORY[CURRENT_LATCHES.PC/2][0];	/*low_byte now has the low 8 bits of the instruction*/
	high_byte = MEMORY[CURRENT_LATCHES.PC/2][1];	/*high_byte now has the high 8 bits of the instruction*/
	ir = ((high_byte << 8) | low_byte);		/*ir = xHBLB  high_byte and low_byte are now low 16 bits of ir */

	ops  opcode = (ir & 0xF000) >> 12;
	
	switch(opcode){

	case ADD:
		execute_ADD();
		break;
	case AND:
		execute_AND();
		break;
	case BR:				/**********MAKE SURE TO TEST FOR NOP (x0000)***********/
		execute_BR();
		break;
	case JMP_RET:
		execute_JMP_RET();
		break;
	case JSR_R:
		execute_JSR_R();
		break;
	case LDB:
		execute_LDB();
		break;
	case LDW:
		execute_LDW();
		break;
	case LEA:
		execute_LEA();
		break;
	case NOT_XOR: 
		execute_NOT_XOR();
		break;
	case SHF:
		execute_SHF();
		break;
	case STB:
		execute_STB();
		break;
	case STW:
		execute_STW();
		break;
	case TRAP: 
		execute_TRAP();
		break;

	;}
	
}


void execute_ADD(void){
	int DR = 0;
	int SR1 = 0;
	int SR2 = 0;
	int imm = 0;

	DR = (ir & 0x0E00) >> 9;		/*DR has int value of destination register*/	
	SR1 = (ir & 0x01C0) >> 6;
	
	if((ir & 0x00000020) == 0x00000020){			/*use immediate value*/

		imm = sext((ir & 0x0000001F), 0x0010);

		NEXT_LATCHES.REGS[DR] = Low16bits(CURRENT_LATCHES.REGS[SR1] +imm);
	}
	else{				/*use SR2*/
		SR2 = (ir & 0x00000007);

		NEXT_LATCHES.REGS[DR] = Low16bits(CURRENT_LATCHES.REGS[SR1] + CURRENT_LATCHES.REGS[SR2]);
	}

	set_cc(NEXT_LATCHES.REGS[DR]);

}

void execute_JMP_RET(void){
	int BR = 0;

	BR = (ir & 0x01C0) >> 6;
	
	NEXT_LATCHES.PC = Low16bits(CURRENT_LATCHES.REGS[BR]);	/*PC <-- BaseR*/	
	

}

void execute_AND(void){
	int DR = 0;
	int SR1 = 0;
	int SR2 = 0;
	int imm5 = 0;

	DR = ((ir & 0x0E00) >> 9);
	SR1 = ((ir & 0x01C0) >> 6);

	if ((ir & 0x0020) == 0) {
		SR2 = ((ir & 0x0007));
		NEXT_LATCHES.REGS[DR] = Low16bits(CURRENT_LATCHES.REGS[SR1] & CURRENT_LATCHES.REGS[SR2]);
	}

	else if ((ir & 0x0020) == 0x0020) {
		imm5 = sext((ir & 0x001F), 0x0010);
		
		NEXT_LATCHES.REGS[DR] = Low16bits(CURRENT_LATCHES.REGS[SR1] & imm5);
	}

	set_cc(NEXT_LATCHES.REGS[DR]);

}

void execute_BR(void){
	int BEN = 0;
	int PCoffset9 = 0;
	BEN = (CURRENT_LATCHES.N & ((ir & 0x0800) >> 11)) | 
		  (CURRENT_LATCHES.Z & ((ir & 0x0400) >> 10)) | 
		  (CURRENT_LATCHES.P & ((ir & 0x0200) >> 9));

	if (BEN == 1) {
		PCoffset9 = sext((ir & 0x01FF), 0x0100);
		NEXT_LATCHES.PC = Low16bits(NEXT_LATCHES.PC + (PCoffset9 << 1));
	}

}

void execute_JSR_R(void){
	int BaseR = 0;
	int PCoffset11 = 0;

	NEXT_LATCHES.REGS[7] = NEXT_LATCHES.PC;

	if((ir & 0x0800) == 0x0800) {
		PCoffset11 = sext((ir & 0x07FF), 0x0700);
		NEXT_LATCHES.PC = Low16bits(NEXT_LATCHES.PC + (PCoffset11 << 1));
	}
	else {
		BaseR = ((ir & 0x1C0) >> 6);
		NEXT_LATCHES.PC = Low16bits(CURRENT_LATCHES.REGS[BaseR]);
	}

}

void execute_LDB(void){
	int DR = 0;
	int BR = 0;
	int offset = 0;
	int address = 0;
	int data = 0;

	DR = (ir & 0x00000E00) >> 9;
	BR = (ir & 0x000001C0) >> 6;
	offset = (sext((ir & 0x0000003F), 0x0020));
	address = (CURRENT_LATCHES.REGS[BR] + offset);	/*address contains contents of BR + offset*/

	data = Low16bits(MEMORY[address/2][address%2]);

	NEXT_LATCHES.REGS[DR] = data;

}

void execute_LDW(void){
	int DR = 0;
	int BR = 0;
	int offset = 0;
	int address = 0;
	int data = 0;

	DR = (ir & 0x00000E00) >> 9;
	BR = (ir & 0x000001C0) >> 6;
	offset = (sext((ir & 0x0000003F), 0x0020) << 1);
	address = (CURRENT_LATCHES.REGS[BR] + offset);	/*address contains contents of BR + offset*/

	data = Low16bits((MEMORY[address/2][1] << 8) | MEMORY[address/2][0]);
	NEXT_LATCHES.REGS[DR] = data;
}

void execute_LEA(void){
	int DR = 0;
	int offset = 0;

	DR = ((ir & 0x0E00) >> 9);
	offset = ir & 0x01FF;
	offset = sext(offset, 0x0100);
	offset = offset << 1;

	NEXT_LATCHES.REGS[DR] = Low16bits(NEXT_LATCHES.PC + offset);

}	

void execute_NOT_XOR(void){		
	int DR = 0;
	int SR1 = 0;
	int SR2 = 0;
	int imm = 0;	
	
	DR = (ir & 0x0E00) >> 9;		/*DR has int value of destination register*/	
	SR1 = (ir & 0x01C0) >> 6;
	
	if((ir & 0x00000020) == 0x00000020){			/*use imm5 value*/
		imm = sext((ir & 0x0000001F), 0x0010);		/*sext from bit from bit 4 onward*/
		NEXT_LATCHES.REGS[DR] = Low16bits(CURRENT_LATCHES.REGS[SR1]^imm);
	}
	
	else{	/*Use SR2 instead of imm5*/
		NEXT_LATCHES.REGS[DR] = Low16bits(CURRENT_LATCHES.REGS[SR1]^CURRENT_LATCHES.REGS[SR2]);
	}
	
	set_cc(NEXT_LATCHES.REGS[DR]);
	
}

void execute_SHF(void){
	int DR = 0;
	int SR = 0;
	int amount4 = 0;
	int condition = 0;
	int value = 0;

	DR = ((ir & 0x0E00) >> 9);
	SR = ((ir & 0x01C0) >> 6);
	amount4 = (ir & 0x000F);
	condition = ((ir & 0x0030) >> 4);
	value = CURRENT_LATCHES.REGS[SR];

	if (condition == 0) {
		NEXT_LATCHES.REGS[DR] = Low16bits((value << amount4));
	}
	if (condition == 1) {
		value = LSR(value, amount4);
		NEXT_LATCHES.REGS[DR] = Low16bits(value);
	}
	if (condition == 3) {
		value = ASR(value, amount4);
		NEXT_LATCHES.REGS[DR] = Low16bits(value);
	}

	set_cc(NEXT_LATCHES.REGS[DR]);
	
}

void execute_STB(void){
	int SR = 0;
	int BR = 0;
	int offset = 0;
	int address = 0;
	int lowbyte = 0;

	SR = (ir & 0x00000E00) >> 9;
	BR = (ir & 0x000001C0) >> 6;
	offset = (sext((ir & 0x0000003F), 0x0020));

	address = (CURRENT_LATCHES.REGS[BR] + offset);		/*address contains contents of BR + offset*/
	lowbyte = (CURRENT_LATCHES.REGS[SR] & 0x000000FF); 	/*lowbyte contains bits 7-0 of SR*/

	MEMORY[address/2][address%2] = lowbyte;			/*store lowbyte*/

}

void execute_STW(void){
	int SR = 0;
	int BR = 0;
	int offset = 0;
	int address = 0;
	int lowbyte = 0;
	int highbyte = 0;

	SR = (ir & 0x00000E00) >> 9;
	BR = (ir & 0x000001C0) >> 6;
	offset = (sext((ir & 0x0000003F), 0x0020) << 1);

	address = (CURRENT_LATCHES.REGS[BR] + offset);	/*address contains contents of BR + offset*/

	lowbyte = (CURRENT_LATCHES.REGS[SR] & 0x000000FF); 		/*lowbyte contains bits 7-0 of SR*/
	highbyte = ((CURRENT_LATCHES.REGS[SR] & 0x0000FF00) >> 8); 	/*highbyte contains bits 15-8 of SR*/

	MEMORY[address/2][0] = lowbyte;		/*store lowbyte*/
	MEMORY[address/2][1] = highbyte;	/*store highbyte*/

}

void execute_TRAP(void){
	
	int vector = 0;
	vector = (ir & 0x000000FF); /*vector gets low 8 bits of IR*/

	NEXT_LATCHES.REGS[7] = Low16bits(NEXT_LATCHES.PC);
	NEXT_LATCHES.PC = Low16bits((MEMORY[vector][1] << 8) | MEMORY[vector][0]);	/*PC gets value stored at location of vector*/ 
	
}

/*Helper function definitions*/

int LSR(int x, int n){
	return (x >> n);
}

int ASR(int x, int n){
	int i = 0;
	int mask = 0x8000;
	if ((x & 0x8000) == 0x8000) {
		x = x >> n;
		for (i = 0; i < n; i++) {
			x = x | mask;
			mask = mask >> 1;
		}
	}
	else {
		x = x >> n;
	}
	return x;
}


void set_cc(int val){
	if((val & 0x00008000) == 0x00008000){ /*bit 15 = 1 means val is negative*/
		NEXT_LATCHES.P = 0;
		NEXT_LATCHES.Z = 0;
		NEXT_LATCHES.N = 1;
	}

	else if(val ==  0){
		NEXT_LATCHES.N = 0;
		NEXT_LATCHES.P = 0;
		NEXT_LATCHES.Z = 1;
	}

	else{
		NEXT_LATCHES.Z = 0;
		NEXT_LATCHES.P = 1;
		NEXT_LATCHES.N = 0;
	}

}

int sext(int value, int mask_bit){		/*value is the number being sign extended, mask_bit is the bit value we're checking to see neg or pos*/

	switch(mask_bit){
		case 0x0010:	/*5 bit immediate*/ 
			if((value & mask_bit) == mask_bit){
				value = value | 0xFFFFFFF0;
			}
			break;
		case 0x0020:	/*6 bit immediate*/
			if((value & mask_bit) == mask_bit){
				value = value | 0xFFFFFFE0;
			}
			break;
		case 0x0100:	/*9 bit immediate*/
			if((value & mask_bit) == mask_bit){
				value = value | 0xFFFFFF00;
			}
			break;
		case 0x0400:	/*11 bit immediate*/
			if((value & mask_bit) == mask_bit){
				value = value | 0xFFFFFC00;
			}
			break;
	;}

	return value;

}




