/***************************************************************/
/*                                                             */
/*   LC-3b Simulator                                           */
/*                                                             */
/*   EE 460N                                                   */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***************************************************************/
/*                                                             */
/* Files:  ucode        Microprogram file                      */
/*         isaprogram   LC-3b machine language program file    */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void eval_micro_sequencer();
void cycle_memory();
void eval_bus_drivers();
void drive_bus();
void latch_datapath_values();

int sext(int value, int mask_bit);
int ASR(int x, int n);
int LSR(int x, int n);

/***************************************************************/
/* A couple of useful definitions.                             */
/***************************************************************/
#define FALSE 0
#define TRUE  1

/***************************************************************/
/* Use this to avoid overflowing 16 bits on the bus.           */
/***************************************************************/
#define Low16Bits(x) ((x) & 0xFFFF)

/***************************************************************/
/* Definition of the control store layout.                     */
/***************************************************************/
#define CONTROL_STORE_ROWS 64
#define INITIAL_STATE_NUMBER 18

/***************************************************************/
/* Definition of bit order in control store word.              */
/***************************************************************/
enum CS_BITS {
	IRD,
	COND2, COND1, COND0,
	ECOND1, ECOND0,
	J5, J4, J3, J2, J1, J0,
	LD_MAR,
	LD_MDR,
	LD_IR,
	LD_BEN,
	LD_REG,
	LD_CC,
	LD_PC,
	GATE_PC,
	GATE_MDR,
	GATE_ALU,
	GATE_MARMUX,
	GATE_SHF,
	PCMUX1, PCMUX0,
	DRMUX1, DRMUX0,
	SR1MUX1, SR1MUX0,
	ADDR1MUX,
	ADDR2MUX1, ADDR2MUX0,
	MARMUX,
	ALUK1, ALUK0,
	MIO_EN,
	R_W,
	DATA_SIZE,
	LSHF1,

	LD_EC,
	LD_USP,
	LD_SSP,
	LD_EV,
	LD_VTR,
	LD_PSR,
	GATE_PSR,
	GATE_VTR,
	PSRMUX,
	IMUX1, IMUX0,
	VTRMUX,
	RMUX1, RMUX0,
	/* MODIFY: you have to add all your new control signals */
	CONTROL_STORE_BITS
} CS_BITS;

/***************************************************************/
/* Functions to get at the control bits.                       */
/***************************************************************/
int GetIRD(int *x) { return(x[IRD]); }
int GetCOND(int *x) { return((x[COND2] << 2) + (x[COND1] << 1) + x[COND0]); }
int GetECOND(int *x) { return((x[ECOND1] << 1) + x[ECOND0]); }
int GetJ(int *x) {
	return((x[J5] << 5) + (x[J4] << 4) +
		(x[J3] << 3) + (x[J2] << 2) +
		(x[J1] << 1) + x[J0]);
}
int GetLD_MAR(int *x) { return(x[LD_MAR]); }
int GetLD_MDR(int *x) { return(x[LD_MDR]); }
int GetLD_IR(int *x) { return(x[LD_IR]); }
int GetLD_BEN(int *x) { return(x[LD_BEN]); }
int GetLD_REG(int *x) { return(x[LD_REG]); }
int GetLD_CC(int *x) { return(x[LD_CC]); }
int GetLD_PC(int *x) { return(x[LD_PC]); }
int GetGATE_PC(int *x) { return(x[GATE_PC]); }
int GetGATE_MDR(int *x) { return(x[GATE_MDR]); }
int GetGATE_ALU(int *x) { return(x[GATE_ALU]); }
int GetGATE_MARMUX(int *x) { return(x[GATE_MARMUX]); }
int GetGATE_SHF(int *x) { return(x[GATE_SHF]); }
int GetPCMUX(int *x) { return((x[PCMUX1] << 1) + x[PCMUX0]); }
int GetDRMUX(int *x) { return((x[DRMUX1] << 1) + (x[DRMUX0])); }
int GetSR1MUX(int *x) { return((x[SR1MUX1] << 1) + (x[SR1MUX0])); }
int GetADDR1MUX(int *x) { return(x[ADDR1MUX]); }
int GetADDR2MUX(int *x) { return((x[ADDR2MUX1] << 1) + x[ADDR2MUX0]); }
int GetMARMUX(int *x) { return(x[MARMUX]); }
int GetALUK(int *x) { return((x[ALUK1] << 1) + x[ALUK0]); }
int GetMIO_EN(int *x) { return(x[MIO_EN]); }
int GetR_W(int *x) { return(x[R_W]); }
int GetDATA_SIZE(int *x) { return(x[DATA_SIZE]); }
int GetLSHF1(int *x) { return(x[LSHF1]); }

int GetLD_EC(int *x) { return(x[LD_EC]); }
int GetLD_USP(int *x) { return(x[LD_USP]); }
int GetLD_SSP(int *x) { return(x[LD_SSP]); }
int GetLD_EV(int *x) { return(x[LD_EV]); }
int GetLD_VTR(int *x) { return(x[LD_VTR]); }
int GetLD_PSR(int *x) { return(x[LD_PSR]); }
int GetGATE_PSR(int *x) { return(x[GATE_PSR]); }
int GetGATE_VTR(int *x) { return(x[GATE_VTR]); }
int GetPSRMUX(int *x) { return(x[PSRMUX]); }
int GetIMUX(int *x) { return((x[IMUX1] << 1) + x[IMUX0]); }
int GetVTRMUX(int *x) { return(x[VTRMUX]); }
int GetRMUX(int *x) { return((x[RMUX1] << 1) + x[RMUX0]); }



/* MODIFY: you can add more Get functions for your new control signals */

/***************************************************************/
/* The control store rom.                                      */
/***************************************************************/
int CONTROL_STORE[CONTROL_STORE_ROWS][CONTROL_STORE_BITS];

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
MEMORY[A][1] stores the most significant byte of word at word address A
There are two write enable signals, one for each byte. WE0 is used for
the least significant byte of a word. WE1 is used for the most significant
byte of a word. */

#define WORDS_IN_MEM    0x08000 
#define MEM_CYCLES      5
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT;	/* run bit */
int BUS;	/* value of the bus */

typedef struct System_Latches_Struct {

	int PC,		/* program counter */
		MDR,	/* memory data register */
		MAR,	/* memory address register */
		IR,		/* instruction register */
		N,		/* n condition bit */
		Z,		/* z condition bit */
		P,		/* p condition bit */
		BEN;        /* ben register */

	int READY;	/* ready bit */
				/* The ready bit is also latched as you dont want the memory system to assert it
				at a bad point in the cycle*/

	int REGS[LC_3b_REGS]; /* register file. */

	int MICROINSTRUCTION[CONTROL_STORE_BITS]; /* The microintruction */

	int STATE_NUMBER; /* Current State Number - Provided for debugging */

					  /* For lab 4 */
	int INTV; /* Interrupt vector register */
	int EXCV; /* Exception vector register */
	int SSP; /* Initial value of system stack pointer */
			 /* MODIFY: You may add system latches that are required by your implementation */
	int USP;
	int PSR;
	int VTR;

	int EC0;
	int EC1;
	int EC2;
	int INTERRUPT; 


} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int CYCLE_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands.                   */
/*                                                             */
/***************************************************************/
void help() {
	printf("----------------LC-3bSIM Help-------------------------\n");
	printf("go               -  run program to completion       \n");
	printf("run n            -  execute program for n cycles    \n");
	printf("mdump low high   -  dump memory from low to high    \n");
	printf("rdump            -  dump the register & bus values  \n");
	printf("?                -  display this help menu          \n");
	printf("quit             -  exit the program                \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {

	eval_micro_sequencer();
	cycle_memory();
	eval_bus_drivers();
	drive_bus();
	latch_datapath_values();

	CURRENT_LATCHES = NEXT_LATCHES;

	CYCLE_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3b for n cycles.                 */
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
/* Purpose   : Simulate the LC-3b until HALTed.                 */
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

	printf("\nMemory content [0x%0.4x..0x%0.4x] :\n", start, stop);
	printf("-------------------------------------\n");
	for (address = (start >> 1); address <= (stop >> 1); address++)
		printf("  0x%0.4x (%d) : 0x%0.2x%0.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
	printf("\n");

	/* dump the memory contents into the dumpsim file */
	fprintf(dumpsim_file, "\nMemory content [0x%0.4x..0x%0.4x] :\n", start, stop);
	fprintf(dumpsim_file, "-------------------------------------\n");
	for (address = (start >> 1); address <= (stop >> 1); address++)
		fprintf(dumpsim_file, " 0x%0.4x (%d) : 0x%0.2x%0.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
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
	printf("Cycle Count  : %d\n", CYCLE_COUNT);
	printf("PC           : 0x%0.4x\n", CURRENT_LATCHES.PC);
	printf("IR           : 0x%0.4x\n", CURRENT_LATCHES.IR);
	//printf("STATE_NUMBER : 0x%0.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
	printf("STATE_NUMBER : %d\n\n", CURRENT_LATCHES.STATE_NUMBER);

	printf("PSR	     : 0x%0.4x\n", CURRENT_LATCHES.PSR);
	printf("SSP	     : 0x%0.4x\n", CURRENT_LATCHES.SSP);
	printf("USP	     : 0x%0.4x\n", CURRENT_LATCHES.USP);
	printf("VTR 	     : 0x%0.4x\n", CURRENT_LATCHES.VTR);

	printf("BUS          : 0x%0.4x\n", BUS);
	printf("MDR          : 0x%0.4x\n", CURRENT_LATCHES.MDR);
	printf("MAR          : 0x%0.4x\n", CURRENT_LATCHES.MAR);
	printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
	printf("Registers:\n");
	for (k = 0; k < LC_3b_REGS; k++)
		printf("%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
	printf("\n");

	/* dump the state information into the dumpsim file */
	fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
	fprintf(dumpsim_file, "-------------------------------------\n");
	fprintf(dumpsim_file, "Cycle Count  : %d\n", CYCLE_COUNT);
	fprintf(dumpsim_file, "PC           : 0x%0.4x\n", CURRENT_LATCHES.PC);
	fprintf(dumpsim_file, "IR           : 0x%0.4x\n", CURRENT_LATCHES.IR);
	fprintf(dumpsim_file, "STATE_NUMBER : 0x%0.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
	fprintf(dumpsim_file, "BUS          : 0x%0.4x\n", BUS);
	fprintf(dumpsim_file, "MDR          : 0x%0.4x\n", CURRENT_LATCHES.MDR);
	fprintf(dumpsim_file, "MAR          : 0x%0.4x\n", CURRENT_LATCHES.MAR);
	fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
	fprintf(dumpsim_file, "Registers:\n");
	for (k = 0; k < LC_3b_REGS; k++)
		fprintf(dumpsim_file, "%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
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

	switch (buffer[0]) {
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
/* Procedure : init_control_store                              */
/*                                                             */
/* Purpose   : Load microprogram into control store ROM        */
/*                                                             */
/***************************************************************/
void init_control_store(char *ucode_filename) {
	FILE *ucode;
	int i, j, index;
	char line[200];

	printf("Loading Control Store from file: %s\n", ucode_filename);

	/* Open the micro-code file. */
	if ((ucode = fopen(ucode_filename, "r")) == NULL) {
		printf("Error: Can't open micro-code file %s\n", ucode_filename);
		exit(-1);
	}

	/* Read a line for each row in the control store. */
	for (i = 0; i < CONTROL_STORE_ROWS; i++) {
		if (fscanf(ucode, "%[^\n]\n", line) == EOF) {
			printf("Error: Too few lines (%d) in micro-code file: %s\n",
				i, ucode_filename);
			exit(-1);
		}

		/* Put in bits one at a time. */
		index = 0;

		for (j = 0; j < CONTROL_STORE_BITS; j++) {
			/* Needs to find enough bits in line. */
			if (line[index] == '\0') {
				printf("Error: Too few control bits in micro-code file: %s\nLine: %d\n",
					ucode_filename, i);
				exit(-1);
			}
			if (line[index] != '0' && line[index] != '1') {
				printf("Error: Unknown value in micro-code file: %s\nLine: %d, Bit: %d\n",
					ucode_filename, i, j);
				exit(-1);
			}

			/* Set the bit in the Control Store. */
			CONTROL_STORE[i][j] = (line[index] == '0') ? 0 : 1;
			index++;
		}

		/* Warn about extra bits in line. */
		if (line[index] != '\0')
			printf("Warning: Extra bit(s) in control store file %s. Line: %d\n",
				ucode_filename, i);
	}
	printf("\n");
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

	for (i = 0; i < WORDS_IN_MEM; i++) {
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

/***************************************************************/
/*                                                             */
/* Procedure : initialize                                      */
/*                                                             */
/* Purpose   : Load microprogram and machine language program  */
/*             and set up initial state of the machine.        */
/*                                                             */
/***************************************************************/

char ucode4[50] = "ucode4.txt";
char add[50] = "add.obj";
char data[50] = "data.obj";
char vectable[50] = "vector_table.obj";
char int1[50] = "int.obj";
char excp1[50] = "except_prot.obj";
char excp2[50] = "except_unaligned.obj";
char excp3[50] = "except_unknown.obj";

void initialize(char *ucode_filename, char *program_filename, int num_prog_files) {
	int i;
	init_control_store(ucode_filename);

	init_memory();
	/*
	for (i = 0; i < num_prog_files; i++) {
		load_program(program_filename);
		while (*program_filename++ != '\0');
	}
	*/
	load_program(add);
	load_program(data);
	load_program(int1);
	load_program(vectable);
	load_program(excp1);
	load_program(excp2);
	load_program(excp3);
	
	CURRENT_LATCHES.Z = 1;
	CURRENT_LATCHES.STATE_NUMBER = INITIAL_STATE_NUMBER;
	memcpy(CURRENT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[INITIAL_STATE_NUMBER], sizeof(int)*CONTROL_STORE_BITS);
	CURRENT_LATCHES.SSP = 0x3000; /* Initial value of system stack pointer */
	CURRENT_LATCHES.USP = 0xFE00;
	CURRENT_LATCHES.PSR = 0x8002;
	CURRENT_LATCHES.EC0 = 0;
	CURRENT_LATCHES.EC1 = 0;
	CURRENT_LATCHES.EC2 = 0;
	CURRENT_LATCHES.REGS[6] = 0xFE00;

	NEXT_LATCHES = CURRENT_LATCHES;

	RUN_BIT = TRUE;
}

/***************************************************************/
/*                                                             */
/* Procedure : main                                            */
/*                                                             */
/***************************************************************/





int main(int argc, char *argv[]) {
	FILE * dumpsim_file;
	
	/* Error Checking */
	/*
	if (argc < 3) {
		printf("Error: usage: %s <micro_code_file> <program_file_1> <program_file_2> ...\n",
			argv[0]);
		exit(1);
	}

	printf("LC-3b Simulator\n\n");

	initialize(argv[1], argv[2], argc - 2);
	*/
	/*WINDOWS WINDOWS WINDOWS WINDOWS WINDOWS WINDOWS WINDOWS WINDOWS*/
	
	printf("LC-3b Simulator\n\n");

	initialize(ucode4, add, 1);
	

	if ((dumpsim_file = fopen("dumpsim", "w")) == NULL) {
		printf("Error: Can't open dumpsim file\n");
		exit(-1);
	}

	while (1)
		get_command(dumpsim_file);

}

/***************************************************************/
/* Do not modify the above code, except for the places indicated
with a "MODIFY:" comment.

Do not modify the rdump and mdump functions.

You are allowed to use the following global variables in your
code. These are defined above.

CONTROL_STORE
MEMORY
BUS

CURRENT_LATCHES
NEXT_LATCHES

You may define your own local/global variables and functions.
You may use the functions to get at the control bits defined
above.

Begin your code here 	  			       */
/***************************************************************/


void eval_micro_sequencer() {

	int IRD = GetIRD(CURRENT_LATCHES.MICROINSTRUCTION);
	int IR = (CURRENT_LATCHES.IR & 0xF000) >> 12;
	int BEN = CURRENT_LATCHES.BEN;
	int R = CURRENT_LATCHES.READY;
	int I = CURRENT_LATCHES.INTERRUPT;
	int IR11 = (CURRENT_LATCHES.IR & 0x0800) >> 11;
	int COND = GetCOND(CURRENT_LATCHES.MICROINSTRUCTION);
	int ECOND = GetECOND(CURRENT_LATCHES.MICROINSTRUCTION);
	int J = GetJ(CURRENT_LATCHES.MICROINSTRUCTION);
	int nextstate;
	int ES;

	if (CYCLE_COUNT == 299) {
		NEXT_LATCHES.INTERRUPT = 1;
	}
	if (CURRENT_LATCHES.STATE_NUMBER == 49) {
		NEXT_LATCHES.INTERRUPT = 0;
	}
	CURRENT_LATCHES.INTV = 0x01;
	
	if (ECOND == 0) {
		ES = 0;
	}
	if (ECOND == 1) {
		ES = CURRENT_LATCHES.EC0;
	}
	if (ECOND == 2) {
		ES = (CURRENT_LATCHES.EC0) | (CURRENT_LATCHES.EC1);
	}
	if (ECOND == 3) {
		ES = CURRENT_LATCHES.EC2;
	}
	
	if((IRD == 0) && (ES == 0)){
		if (COND == 0) {
			nextstate = J;
		}
		if (COND == 1) {
			nextstate = J | (R << 1);
		}
		if (COND == 2) {
			nextstate = J | (BEN << 2);
		}
		if (COND == 3) {
			nextstate = J | IR11;
		}
		if (COND == 4) {
			nextstate = J | (I << 3);
		}
	}
	if ((IRD == 1) && (ES == 0)) {
		nextstate = IR;
	}
	if (ES == 1) {
		nextstate = 26;
	}

	NEXT_LATCHES.STATE_NUMBER = nextstate;
	memcpy(NEXT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[nextstate], sizeof(int)*CONTROL_STORE_BITS);

	/*
	* Evaluate the address of the next state according to the
	* micro sequencer logic. Latch the next microinstruction.
	*/

}


int memwait = 0;


void cycle_memory() {

	/*
	* This function emulates memory and the WE logic.
	* Keep track of which cycle of MEMEN we are dealing with.
	* If fourth, we need to latch Ready bit at the end of
	* cycle to prepare microsequencer for the fifth cycle.
	*/
	int MIOEN = GetMIO_EN(CURRENT_LATCHES.MICROINSTRUCTION);
	int RW = GetR_W(CURRENT_LATCHES.MICROINSTRUCTION);
	int DATASIZE = GetDATA_SIZE(CURRENT_LATCHES.MICROINSTRUCTION);
	int MAR = CURRENT_LATCHES.MAR;
	int MDR = CURRENT_LATCHES.MDR;
	if (MIOEN == 1) {

		if (memwait == 3) {
			NEXT_LATCHES.READY = 1;
		}
		else {
			memwait++;
		}

		if (CURRENT_LATCHES.READY == 1) {
			if (RW == 0) {
				if (DATASIZE == 0) {
					MDR = MEMORY[(MAR / 2)][0];
					MDR = MDR | ((MEMORY[(MAR / 2)][1]) << 8);
				}
				if (DATASIZE == 1) {
					MDR = Low16Bits((MEMORY[(MAR / 2)][0]) | ((MEMORY[(MAR / 2)][1]) << 8));
				}
				NEXT_LATCHES.MDR = Low16Bits(MDR);
			}
			if (RW == 1) {
				if (DATASIZE == 0) {
					MEMORY[(MAR / 2)][(MAR % 2)] = Low16Bits((MDR & 0x00FF));
				}
				if (DATASIZE == 1) {
					MEMORY[(MAR / 2)][0] = Low16Bits((MDR & 0x00FF));
					MEMORY[(MAR / 2)][1] = Low16Bits(((MDR & 0xFF00) >> 8));
				}
			}
			memwait = 0;
			NEXT_LATCHES.READY = 0;

		}
	}


}

int InGatePC;
int InGateMARMUX;
int InGateALU;
int InGateSHF;
int InGateMDR;
int InGatePSR;
int InGateVTR;

void eval_bus_drivers() {

	/*
	* Datapath routine emulating operations before driving the bus.
	* Evaluate the input of tristate drivers
	*        Gate_MARMUX,
	*		 Gate_PC*,
	*		 Gate_ALU*,
	*		 Gate_SHF*,
	*		 Gate_MDR.
			Gate_PSR
			Gate_VTR
	*/

	int SR1MUX = GetSR1MUX(CURRENT_LATCHES.MICROINSTRUCTION);
	int ALUK = GetALUK(CURRENT_LATCHES.MICROINSTRUCTION);
	int SR2MUX = (CURRENT_LATCHES.IR & 0x0020) >> 5;
	int IMUX = GetIMUX(CURRENT_LATCHES.MICROINSTRUCTION);
	int SHF_imm4 = CURRENT_LATCHES.IR & 0x000F;
	int SHF_cond = (CURRENT_LATCHES.IR & 0x0030) >> 4;
	int SR1;
	int SR1val;

	int SR2 = CURRENT_LATCHES.IR & 0x0007;
	int SR2val;
	int imm5 = sext((CURRENT_LATCHES.IR & 0x001F), 0x0010);
	int A;
	int B;

	int DATASIZE = GetDATA_SIZE(CURRENT_LATCHES.MICROINSTRUCTION);
	int MAR0 = CURRENT_LATCHES.MAR & 0x0001;

	int PC = CURRENT_LATCHES.PC;
	int ADDR1MUX = GetADDR1MUX(CURRENT_LATCHES.MICROINSTRUCTION);
	int ADDR2MUX = GetADDR2MUX(CURRENT_LATCHES.MICROINSTRUCTION);
	int MARMUX = GetMARMUX(CURRENT_LATCHES.MICROINSTRUCTION);
	int LSHF1 = GetLSHF1(CURRENT_LATCHES.MICROINSTRUCTION);
	int IR10 = sext((CURRENT_LATCHES.IR & 0x07FF), 0x0400);
	int IR8 = sext((CURRENT_LATCHES.IR & 0x01FF), 0x0100);
	int IR5 = sext((CURRENT_LATCHES.IR & 0x003F), 0x0020);
	int IR7 = (CURRENT_LATCHES.IR & 0x00FF) << 1;
	int ADDR2MUX_out;
	int ADDR1MUX_out;
	int ADDER_out;
	int MARMUX_out;



	int SHF_out;
	int ALU_out;
	int MDR_out;

	/*InGatePC*/

	InGatePC = Low16Bits(CURRENT_LATCHES.PC);

	/*InGateSHF*/

	if (SR1MUX == 0) {
		SR1 = (CURRENT_LATCHES.IR & 0x0E00) >> 9;
	}
	if(SR1MUX == 1){
		SR1 = (CURRENT_LATCHES.IR & 0x01C0) >> 6;
	}
	if (SR1MUX == 2) {
		SR1 = 6;
	}
	SR1val = CURRENT_LATCHES.REGS[SR1];

	if (SHF_cond == 0) {
		SHF_out = (SR1val) << (SHF_imm4);
	}
	if (SHF_cond == 1) {
		SHF_out = LSR(SR1val, SHF_imm4);
	}
	if (SHF_cond == 3) {
		SHF_out = ASR(SR1val, SHF_imm4);
	}
	if (SHF_cond == 2) {
		SHF_out = 0;
	}


	InGateSHF = Low16Bits(SHF_out);

	/*InGateALU*/
	A = sext(SR1val, 0x8000);
	if (SR2MUX == 0) {
		SR2val = CURRENT_LATCHES.REGS[SR2];
		B = sext(SR2val, 0x8000);
	}
	else {
		B = imm5;
		B = sext(imm5, 0x010);
	}
	if (IMUX == 0) {
		B = B;
	}
	if (IMUX == 1) {
		B = 2;
	}
	if (IMUX == 2) {
		B = -2;
	}
	if (ALUK == 0) {
		ALU_out = A + B;
	}
	if (ALUK == 1) {
		ALU_out = A & B;
	}
	if (ALUK == 2) {
		ALU_out = A ^ B;
	}
	if (ALUK == 3) {
		ALU_out = A;
	}

	InGateALU = Low16Bits(ALU_out);

	/*InGateMDR*/

	if (DATASIZE == 1) {
		MDR_out = CURRENT_LATCHES.MDR;
	}
	else {
		if (MAR0 == 0) {
			MDR_out = sext((CURRENT_LATCHES.MDR & 0x00FF), 0x0080);
		}
		if (MAR0 == 1) {
			MDR_out = sext(((CURRENT_LATCHES.MDR & 0xFF00) >> 8), 0x0080);
		}
	}

	InGateMDR = MDR_out;

	/*InGateMARMUX*/

	if (MARMUX == 0) {
		MARMUX_out = IR7;
	}
	else {
		if (ADDR1MUX == 0) {
			ADDR1MUX_out = PC;
		}
		if (ADDR1MUX == 1) {
			ADDR1MUX_out = SR1val;
		}

		if (ADDR2MUX == 0) {
			ADDR2MUX_out = 0;
		}
		if (ADDR2MUX == 1) {
			ADDR2MUX_out = IR5;
		}
		if (ADDR2MUX == 2) {
			ADDR2MUX_out = IR8;
		}
		if (ADDR2MUX == 3) {
			ADDR2MUX_out = IR10;
		}
		if (LSHF1 == 1) {
			ADDR2MUX_out = ADDR2MUX_out << 1;
		}
		ADDER_out = ADDR1MUX_out + ADDR2MUX_out;
		MARMUX_out = ADDER_out;
	}

	InGateMARMUX = MARMUX_out;

	/*InGatePSR*/

	InGatePSR = Low16Bits(CURRENT_LATCHES.PSR);

	/*InGateVTR*/

	InGateVTR = Low16Bits(CURRENT_LATCHES.VTR);
	
}



void drive_bus() {

	/*
	* Datapath routine for driving the bus from one of the 5 possible
	* tristate drivers.
	*/

	int GateMARMUX = GetGATE_MARMUX(CURRENT_LATCHES.MICROINSTRUCTION);
	int GatePC = GetGATE_PC(CURRENT_LATCHES.MICROINSTRUCTION);
	int GateSHF = GetGATE_SHF(CURRENT_LATCHES.MICROINSTRUCTION);
	int GateALU = GetGATE_ALU(CURRENT_LATCHES.MICROINSTRUCTION);
	int GateMDR = GetGATE_MDR(CURRENT_LATCHES.MICROINSTRUCTION);
	int GateVTR = GetGATE_VTR(CURRENT_LATCHES.MICROINSTRUCTION);
	int GatePSR = GetGATE_PSR(CURRENT_LATCHES.MICROINSTRUCTION);

	if (GateMARMUX == TRUE) {
		BUS = InGateMARMUX;
	}
	if (GatePC == TRUE) {
		BUS = InGatePC;
	}
	if (GateSHF == TRUE) {
		BUS = InGateSHF;
	}
	if (GateALU == TRUE) {
		BUS = InGateALU;
	}
	if (GateMDR == TRUE) {
		BUS = InGateMDR;
	}
	if (GateVTR == TRUE) {
		BUS = InGateVTR;
	}
	if (GatePSR == TRUE) {
		BUS = InGatePSR;
	}
	if ((GateMARMUX == FALSE) && (GatePC == FALSE) && (GateSHF == FALSE) && (GateALU == FALSE) && (GateMDR == FALSE) && (GatePSR == FALSE) && (GateVTR == FALSE)) {
		BUS = 0;
	}

}


void latch_datapath_values() {

	/*
	* Datapath routine for computing all functions that need to latch
	* values in the data path at the end of this cycle.  Some values
	* require sourcing the bus; therefore, this routine has to come
	* after drive_bus.
	*/

	int LDPC = GetLD_PC(CURRENT_LATCHES.MICROINSTRUCTION);
	int LDREG = GetLD_REG(CURRENT_LATCHES.MICROINSTRUCTION);
	int LDCC = GetLD_CC(CURRENT_LATCHES.MICROINSTRUCTION);
	int LDBEN = GetLD_BEN(CURRENT_LATCHES.MICROINSTRUCTION);
	int LDMDR = GetLD_MDR(CURRENT_LATCHES.MICROINSTRUCTION);
	int LDMAR = GetLD_MAR(CURRENT_LATCHES.MICROINSTRUCTION);
	int LDIR = GetLD_IR(CURRENT_LATCHES.MICROINSTRUCTION);
	int LDSSP = GetLD_SSP(CURRENT_LATCHES.MICROINSTRUCTION);
	int LDUSP = GetLD_USP(CURRENT_LATCHES.MICROINSTRUCTION);
	int LDEC = GetLD_EC(CURRENT_LATCHES.MICROINSTRUCTION);
	int LDPSR = GetLD_PSR(CURRENT_LATCHES.MICROINSTRUCTION);
	int LDVTR = GetLD_VTR(CURRENT_LATCHES.MICROINSTRUCTION);
	int LDEV = GetLD_EV(CURRENT_LATCHES.MICROINSTRUCTION);

	int IR11 = ((CURRENT_LATCHES.IR & 0x0800) >> 11);
	int IR10 = ((CURRENT_LATCHES.IR & 0x0400) >> 10);
	int IR9 = ((CURRENT_LATCHES.IR & 0x0200) >> 9);
	

	int DR;
	int DRMUX = GetDRMUX(CURRENT_LATCHES.MICROINSTRUCTION);
	int RMUX = GetRMUX(CURRENT_LATCHES.MICROINSTRUCTION);

	int DATASIZE = GetDATA_SIZE(CURRENT_LATCHES.MICROINSTRUCTION);
	int MAR0 = CURRENT_LATCHES.MAR & 0x0001;
	int MIOEN = GetMIO_EN(CURRENT_LATCHES.MICROINSTRUCTION);


	BUS = Low16Bits(BUS);

	if (LDPC == FALSE) {
		NEXT_LATCHES.PC = CURRENT_LATCHES.PC;
	}

	/*Update Regs regardless of LDREG*/

	NEXT_LATCHES.REGS[0] = CURRENT_LATCHES.REGS[0];
	NEXT_LATCHES.REGS[1] = CURRENT_LATCHES.REGS[1];
	NEXT_LATCHES.REGS[2] = CURRENT_LATCHES.REGS[2];
	NEXT_LATCHES.REGS[3] = CURRENT_LATCHES.REGS[3];
	NEXT_LATCHES.REGS[4] = CURRENT_LATCHES.REGS[4];
	NEXT_LATCHES.REGS[5] = CURRENT_LATCHES.REGS[5];
	NEXT_LATCHES.REGS[6] = CURRENT_LATCHES.REGS[6];
	NEXT_LATCHES.REGS[7] = CURRENT_LATCHES.REGS[7];

	if (LDCC == FALSE) {
		NEXT_LATCHES.N = CURRENT_LATCHES.N;
		NEXT_LATCHES.Z = CURRENT_LATCHES.Z;
		NEXT_LATCHES.P = CURRENT_LATCHES.P;
	}
	if (LDBEN == FALSE) {
		NEXT_LATCHES.BEN = CURRENT_LATCHES.BEN;
	}
	if (LDMDR == FALSE) {
		NEXT_LATCHES.MDR = CURRENT_LATCHES.MDR;
	}
	if (LDMAR == FALSE) {
		NEXT_LATCHES.MAR = CURRENT_LATCHES.MAR;
	}
	if (LDIR == FALSE) {
		NEXT_LATCHES.IR = CURRENT_LATCHES.IR;
	}
	if (LDSSP == FALSE) {
		NEXT_LATCHES.SSP = CURRENT_LATCHES.SSP;
	}
	if (LDUSP == FALSE) {
		NEXT_LATCHES.USP = CURRENT_LATCHES.USP;
	}
	if (LDEC == FALSE) {
		NEXT_LATCHES.EC0 = CURRENT_LATCHES.EC0;
		NEXT_LATCHES.EC1 = CURRENT_LATCHES.EC1;
		NEXT_LATCHES.EC2 = CURRENT_LATCHES.EC2;
	}
	if (LDPSR == FALSE) {
		NEXT_LATCHES.PSR = CURRENT_LATCHES.PSR;
	}
	if (LDVTR == FALSE) {
		NEXT_LATCHES.VTR = CURRENT_LATCHES.VTR;
	}
	if (LDEV == FALSE) {
		NEXT_LATCHES.EXCV = CURRENT_LATCHES.EXCV;
	}


	/*LDMAR*/
	if (LDMAR == TRUE) {
		NEXT_LATCHES.MAR = BUS;
	}

	/*LDIR*/
	if (LDIR == TRUE) {
		NEXT_LATCHES.IR = BUS;
	}

	/*LDCC*/
	if (LDCC == TRUE) {
		if ((BUS & 0x00008000) == 0x00008000) { /*bit 15 = 1 means val is negative*/
			NEXT_LATCHES.P = 0;
			NEXT_LATCHES.Z = 0;
			NEXT_LATCHES.N = 1;
			NEXT_LATCHES.PSR = ((CURRENT_LATCHES.PSR & 0xFFF8) + 4);
		}

		else if (BUS == 0) {
			NEXT_LATCHES.N = 0;
			NEXT_LATCHES.P = 0;
			NEXT_LATCHES.Z = 1;
			NEXT_LATCHES.PSR = ((CURRENT_LATCHES.PSR & 0xFFF8) + 2);
		}

		else {
			NEXT_LATCHES.Z = 0;
			NEXT_LATCHES.P = 1;
			NEXT_LATCHES.N = 0;
			NEXT_LATCHES.PSR = ((CURRENT_LATCHES.PSR & 0xFFF8) + 1);
		}
	}

	/*LDBEN*/
	if (LDBEN == TRUE) {
		NEXT_LATCHES.BEN = ((IR11 & CURRENT_LATCHES.N) | (IR10 & CURRENT_LATCHES.Z) | (IR9 & CURRENT_LATCHES.P));
	}

	/*LDREG*/

	if (LDREG == TRUE) {
		if (DRMUX == 0) {
			DR = ((CURRENT_LATCHES.IR & 0x0E00) >> 9);
		}
		if (DRMUX == 1) {
			DR = 7;
		}
		if (DRMUX == 2) {
			DR = 6;
		}
		if (RMUX == 0) {
			NEXT_LATCHES.REGS[DR] = BUS;
		}
		if (RMUX == 1) {
			NEXT_LATCHES.REGS[DR] = CURRENT_LATCHES.USP;
		}
		if (RMUX == 2) {
			NEXT_LATCHES.REGS[DR] = CURRENT_LATCHES.SSP;
		}
	}

	/*LDPC*/

	if (LDPC == TRUE) {
		int PCMUX = GetPCMUX(CURRENT_LATCHES.MICROINSTRUCTION);
		int PC = CURRENT_LATCHES.PC;

		int ADDR1MUX = GetADDR1MUX(CURRENT_LATCHES.MICROINSTRUCTION);
		int ADDR2MUX = GetADDR2MUX(CURRENT_LATCHES.MICROINSTRUCTION);
		int MARMUX = GetMARMUX(CURRENT_LATCHES.MICROINSTRUCTION);
		int SR1MUX = GetSR1MUX(CURRENT_LATCHES.MICROINSTRUCTION);
		int LSHF1 = GetLSHF1(CURRENT_LATCHES.MICROINSTRUCTION);
		int IR10 = sext((CURRENT_LATCHES.IR & 0x07FF), 0x0400);
		int IR8 = sext((CURRENT_LATCHES.IR & 0x01FF), 0x0100);
		int IR5 = sext((CURRENT_LATCHES.IR & 0x003F), 0x0020);
		int IR7 = (CURRENT_LATCHES.IR & 0x00FF) << 1;
		int ADDR2MUX_out;
		int ADDR1MUX_out;
		int ADDER_out;
		int SR1;
		int SR1val;



		if (PCMUX == 0) {
			NEXT_LATCHES.PC = PC + 2;
		}
		if (PCMUX == 1) {
			NEXT_LATCHES.PC = BUS;
		}

		/*ADDER*/
		if (PCMUX == 2) {
			if (SR1MUX == 0) {
				SR1 = (CURRENT_LATCHES.IR & 0x0E00) >> 9;
			}
			else {
				SR1 = (CURRENT_LATCHES.IR & 0x01C0) >> 6;
			}
			SR1val = CURRENT_LATCHES.REGS[SR1];

			if (ADDR1MUX == 0) {
				ADDR1MUX_out = PC;
			}
			if (ADDR1MUX == 1) {
				ADDR1MUX_out = SR1val;
			}

			if (ADDR2MUX == 0) {
				ADDR2MUX_out = 0;
			}
			if (ADDR2MUX == 1) {
				ADDR2MUX_out = IR5;
			}
			if (ADDR2MUX == 2) {
				ADDR2MUX_out = IR8;
			}
			if (ADDR2MUX == 3) {
				ADDR2MUX_out = IR10;
			}
			if (LSHF1 == 1) {
				ADDR2MUX_out = ADDR2MUX_out << 1;
			}
			ADDER_out = ADDR1MUX_out + ADDR2MUX_out;
			NEXT_LATCHES.PC = ADDER_out;
		}
		if (PCMUX == 3) {
			NEXT_LATCHES.PC = PC - 2;
		}
	}

	/*LDMDR*/
	if (LDMDR == TRUE) {
		if (MIOEN == 0) {

			if (DATASIZE == 0) {
				NEXT_LATCHES.MDR = (BUS & 0x00FF);
				NEXT_LATCHES.MDR = (NEXT_LATCHES.MDR | ((BUS & 0x00FF) << 8));
			}
			if (DATASIZE == 1) {
				NEXT_LATCHES.MDR = BUS;
			}
		}
	}
	/*LDPSR*/
	int PSRMUX = GetPSRMUX(CURRENT_LATCHES.MICROINSTRUCTION);

	if (LDPSR == 1) {
		if (PSRMUX == 0) {
			NEXT_LATCHES.PSR = BUS;
			NEXT_LATCHES.N = ((NEXT_LATCHES.PSR & 0x0004) >> 2);
			NEXT_LATCHES.Z = ((NEXT_LATCHES.PSR & 0x0002) >> 1);
			NEXT_LATCHES.P = (NEXT_LATCHES.PSR & 0x0001);
		}
		if (PSRMUX == 1) {
			NEXT_LATCHES.PSR = (CURRENT_LATCHES.PSR & (~0x8000));
		}
	}

	/*LDEC*/
	int IRop = ((BUS & 0xF000) >> 9);
	int BUS0 = (BUS & 0x0001);

	if (LDEC == 1) {
		if ((BUS >= 0) && (BUS < 0x3000)) {
			if ((CURRENT_LATCHES.PSR & 0x8000) == 0x8000) {
				NEXT_LATCHES.EC0 = 1;
			}
			else {
				NEXT_LATCHES.EC0 = 0;
			}
		}
		else {
			NEXT_LATCHES.EC0 = 0;
		}
		if ((IRop == 10) || (IRop == 11)) {
			NEXT_LATCHES.EC2 = 1;
		}
		else {
			NEXT_LATCHES.EC2 = 0;
		}
		if (BUS0 == 1) {
			NEXT_LATCHES.EC1 = 1;
		}
		else {
			NEXT_LATCHES.EC1 = 0;
		}
	}

	/*LDSSP*/
	if (LDSSP == 1) {
		NEXT_LATCHES.SSP = CURRENT_LATCHES.REGS[6];
	}

	/*LDUSP*/
	
	if (LDUSP == 1) {
		NEXT_LATCHES.USP = CURRENT_LATCHES.REGS[6];
	}

	/*LDEV*/
	int ECOND = GetECOND(CURRENT_LATCHES.MICROINSTRUCTION);
	
	if (LDEV == 1) {
		if (ECOND == 0) {
			NEXT_LATCHES.EXCV = 0x02;
		}
		if (ECOND == 1) {
			if (CURRENT_LATCHES.EC0 == 1) {
				NEXT_LATCHES.EXCV = 0x02;
			}
		}
		if (ECOND == 2) {
			if (CURRENT_LATCHES.EC0 == 1) {
				NEXT_LATCHES.EXCV = 0x02;
			}
			if ((CURRENT_LATCHES.EC0 == 0) && (CURRENT_LATCHES.EC1 == 1)) {
				NEXT_LATCHES.EXCV = 0x03;
			}
		}
		if (ECOND == 3) {
			if (CURRENT_LATCHES.EC2 == 1) {
				NEXT_LATCHES.EXCV = 0x04;
			}
		}
	}

	/*LDVTR*/
	int VTRMUX = GetVTRMUX(CURRENT_LATCHES.MICROINSTRUCTION);
	int Vector;
	if (LDVTR == 1) {
		if (VTRMUX == 0) {
			Vector = 0x0200 + (CURRENT_LATCHES.EXCV << 1);
		}
		if (VTRMUX == 1) {
			Vector = 0x0200 + (CURRENT_LATCHES.INTV << 1);
		}
		NEXT_LATCHES.VTR = Vector;
	}


}

int sext(int value, int mask_bit) {
	int or_mask = 0;
	int i = 0;
	if ((value & mask_bit) == mask_bit) {
		for (i = mask_bit; i != 0; (i = (i << 1))) {
			value = value | i;
		}
	}
	return value;
}

int LSR(int x, int n) {
	return (x >> n);
}

int ASR(int x, int n) {
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