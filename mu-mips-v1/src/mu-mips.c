#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include "mu-mips.h"

// Globals
int instructionNum = 0;
uint32_t baseInstruction = MEM_TEXT_BEGIN;

/***************************************************************/
/* Print out a list of commands available                                                                  */
/***************************************************************/
void help() {        
	printf("------------------------------------------------------------------\n\n");
	printf("\t**********MU-MIPS Help MENU**********\n\n");
	printf("sim\t-- simulate program to completion \n");
	printf("run <n>\t-- simulate program for <n> instructions\n");
	printf("rdump\t-- dump register values\n");
	printf("reset\t-- clears all registers/memory and re-loads the program\n");
	printf("input <reg> <val>\t-- set GPR <reg> to <val>\n");
	printf("mdump <start> <stop>\t-- dump memory from <start> to <stop> address\n");
	printf("high <val>\t-- set the HI register to <val>\n");
	printf("low <val>\t-- set the LO register to <val>\n");
	printf("print\t-- print the program loaded into memory\n");
	printf("?\t-- display help menu\n");
	printf("quit\t-- exit the simulator\n\n");
	printf("------------------------------------------------------------------\n\n");
}

/***************************************************************/
/* Read a 32-bit word from memory                                                                            */
/***************************************************************/
uint32_t mem_read_32(uint32_t address)
{
	int i;
	for (i = 0; i < NUM_MEM_REGION; i++) {
		if ( (address >= MEM_REGIONS[i].begin) &&  ( address <= MEM_REGIONS[i].end) ) {
			uint32_t offset = address - MEM_REGIONS[i].begin;
			return (MEM_REGIONS[i].mem[offset+3] << 24) |
					(MEM_REGIONS[i].mem[offset+2] << 16) |
					(MEM_REGIONS[i].mem[offset+1] <<  8) |
					(MEM_REGIONS[i].mem[offset+0] <<  0);
		}
	}
	return 0;
}

/***************************************************************/
/* Write a 32-bit word to memory                                                                                */
/***************************************************************/
void mem_write_32(uint32_t address, uint32_t value)
{
	int i;
	uint32_t offset;
	for (i = 0; i < NUM_MEM_REGION; i++) {
		if ( (address >= MEM_REGIONS[i].begin) && (address <= MEM_REGIONS[i].end) ) {
			offset = address - MEM_REGIONS[i].begin;

			MEM_REGIONS[i].mem[offset+3] = (value >> 24) & 0xFF;
			MEM_REGIONS[i].mem[offset+2] = (value >> 16) & 0xFF;
			MEM_REGIONS[i].mem[offset+1] = (value >>  8) & 0xFF;
			MEM_REGIONS[i].mem[offset+0] = (value >>  0) & 0xFF;
		}
	}
}

/***************************************************************/
/* Execute one cycle                                                                                                              */
/***************************************************************/
void cycle() {                                                
	handle_instruction();
	CURRENT_STATE = NEXT_STATE;
	INSTRUCTION_COUNT++;
}

/***************************************************************/
/* Simulate MIPS for n cycles                                                                                       */
/***************************************************************/
void run(int num_cycles) {                                      
	
	if (RUN_FLAG == FALSE) {
		printf("Simulation Stopped\n\n");
		return;
	}

	printf("Running simulator for %d cycles...\n\n", num_cycles);
	int i;
	for (i = 0; i < num_cycles; i++) {
		if (RUN_FLAG == FALSE) {
			printf("Simulation Stopped.\n\n");
			break;
		}
		cycle();
	}
}

/***************************************************************/
/* simulate to completion                                                                                               */
/***************************************************************/
void runAll() {                                                     
	if (RUN_FLAG == FALSE) {
		printf("Simulation Stopped.\n\n");
		return;
	}

	printf("Simulation Started...\n\n");
	while (RUN_FLAG){
		cycle();
	}
	printf("Simulation Finished.\n\n");
}

/***************************************************************/ 
/* Dump a word-aligned region of memory to the terminal                              */
/***************************************************************/
void mdump(uint32_t start, uint32_t stop) {          
	uint32_t address;

	printf("-------------------------------------------------------------\n");
	printf("Memory content [0x%08x..0x%08x] :\n", start, stop);
	printf("-------------------------------------------------------------\n");
	printf("\t[Address in Hex (Dec) ]\t[Value]\n");
	for (address = start; address <= stop; address += 4){
		printf("\t0x%08x (%d) :\t0x%08x\n", address, address, mem_read_32(address));
	}
	printf("\n");
}

/***************************************************************/
/* Dump current values of registers to the teminal                                              */   
/***************************************************************/
void rdump() {                               
	int i; 
	printf("-------------------------------------\n");
	printf("Dumping Register Content\n");
	printf("-------------------------------------\n");
	printf("# Instructions Executed\t: %u\n", INSTRUCTION_COUNT);
	printf("PC\t: 0x%08x\n", CURRENT_STATE.PC);
	printf("-------------------------------------\n");
	printf("[Register]\t[Value]\n");
	printf("-------------------------------------\n");
	for (i = 0; i < MIPS_REGS; i++){
		printf("[R%d]\t: 0x%08x\n", i, CURRENT_STATE.REGS[i]);
	}
	printf("-------------------------------------\n");
	printf("[HI]\t: 0x%08x\n", CURRENT_STATE.HI);
	printf("[LO]\t: 0x%08x\n", CURRENT_STATE.LO);
	printf("-------------------------------------\n");
}

/***************************************************************/
/* Read a command from standard input.                                                               */  
/***************************************************************/
void handle_command() {                         
	char buffer[20];
	uint32_t start, stop, cycles;
	uint32_t register_no;
	int register_value;
	int hi_reg_value, lo_reg_value;

	printf("MU-MIPS SIM:> ");

	if (scanf("%s", buffer) == EOF){
		exit(0);
	}

	switch(buffer[0]) {
		case 'S':
		case 's':
			runAll(); 
			break;
		case 'M':
		case 'm':
			if (scanf("%x %x", &start, &stop) != 2){
				break;
			}
			mdump(start, stop);
			break;
		case '?':
			help();
			break;
		case 'Q':
		case 'q':
			printf("**************************\n");
			printf("Exiting MU-MIPS! Good Bye...\n");
			printf("**************************\n");
			exit(0);
		case 'R':
		case 'r':
			if (buffer[1] == 'd' || buffer[1] == 'D'){
				rdump();
			}else if(buffer[1] == 'e' || buffer[1] == 'E'){
				reset();
			}
			else {
				if (scanf("%d", &cycles) != 1) {
					break;
				}
				run(cycles);
			}
			break;
		case 'I':
		case 'i':
			if (scanf("%u %i", &register_no, &register_value) != 2){
				break;
			}
			CURRENT_STATE.REGS[register_no] = register_value;
			NEXT_STATE.REGS[register_no] = register_value;
			break;
		case 'H':
		case 'h':
			if (scanf("%i", &hi_reg_value) != 1){
				break;
			}
			CURRENT_STATE.HI = hi_reg_value; 
			NEXT_STATE.HI = hi_reg_value; 
			break;
		case 'L':
		case 'l':
			if (scanf("%i", &lo_reg_value) != 1){
				break;
			}
			CURRENT_STATE.LO = lo_reg_value;
			NEXT_STATE.LO = lo_reg_value;
			break;
		case 'P':
		case 'p':
			print_program(); 
			break;
		default:
			printf("Invalid Command.\n");
			break;
	}
}

/***************************************************************/
/* reset registers/memory and reload program                                                    */
/***************************************************************/
void reset() {   
	int i;
	/*reset registers*/
	for (i = 0; i < MIPS_REGS; i++){
		CURRENT_STATE.REGS[i] = 0;
	}
	CURRENT_STATE.HI = 0;
	CURRENT_STATE.LO = 0;
	
	for (i = 0; i < NUM_MEM_REGION; i++) {
		uint32_t region_size = MEM_REGIONS[i].end - MEM_REGIONS[i].begin + 1;
		memset(MEM_REGIONS[i].mem, 0, region_size);
	}
	
	/*load program*/
	load_program();
	
	/*reset PC*/
	INSTRUCTION_COUNT = 0;
	CURRENT_STATE.PC =  MEM_TEXT_BEGIN;
	NEXT_STATE = CURRENT_STATE;
	RUN_FLAG = TRUE;
}

/***************************************************************/
/* Allocate and set memory to zero                                                                            */
/***************************************************************/
void init_memory() {                                           
	int i;
	for (i = 0; i < NUM_MEM_REGION; i++) {
		uint32_t region_size = MEM_REGIONS[i].end - MEM_REGIONS[i].begin + 1;
		MEM_REGIONS[i].mem = malloc(region_size);
		memset(MEM_REGIONS[i].mem, 0, region_size);
	}
}

/**************************************************************/
/* load program into memory                                                                                      */
/**************************************************************/
void load_program() {                   
	FILE * fp;
	int i, word;
	uint32_t address;

	/* Open program file. */
	fp = fopen(prog_file, "r");
	if (fp == NULL) {
		printf("Error: Can't open program file %s\n", prog_file);
		exit(-1);
	}

	/* Read in the program. */

	i = 0;
	while( fscanf(fp, "%x\n", &word) != EOF ) {
		address = MEM_TEXT_BEGIN + i;
		mem_write_32(address, word);
		printf("writing 0x%08x into address 0x%08x (%d)\n", word, address, address);
		i += 4;
	}
	PROGRAM_SIZE = i/4;
	printf("Program loaded into memory.\n%d words written into memory.\n\n", PROGRAM_SIZE);
	fclose(fp);
}

/************************************************************/
/* decode and execute instruction                                                                     */ 
/************************************************************/
void handle_instruction()
{
	/*IMPLEMENT THIS*/
	uint32_t addr = baseInstruction + (instructionNum * 4);
	printf("[0x%x]\t", addr);
	uint32_t readAddr = mem_read_32(addr);
	uint32_t mask = 0xFC000000; // 28
	uint32_t rsMask = 0x03E00000; // 21
	uint32_t rtMask = 0x001F0000; // 16
	uint32_t rdMask = 0x0000F800; // 11
	uint32_t saMask = 0x000007C0; // 11
	uint32_t immediateMask = 0x0000FFFF; // 11
	uint32_t lastMask = 0x0000003F;
	uint32_t targetMask = 0x03FFFFFF;
	uint32_t firstNums = (readAddr & mask) >> 26;
	uint32_t rs = (readAddr & rsMask)  >> 21;
	uint32_t rt = (readAddr & rtMask)  >> 16;
	uint32_t rd = (readAddr & rdMask)  >> 11;
	uint32_t sa = (readAddr & saMask)  >> 6;
	uint32_t immediate = (readAddr & immediateMask)  >> 16;
	uint32_t target = readAddr & targetMask;
	uint32_t lastNums = readAddr & lastMask;
	uint32_t result, base;

	// printf("\nInstruction: [0x%x]\n", readAddr);
	// printf("First Nums: [0x%.2x]\n", firstNums);
	// printf("R1 Nums: [0x%x]\n", rs);
	// printf("R2 Nums: [0x%x]\n", rt);
	// printf("Last Nums: [0x%x]\n", lastNums);
	// printf("rdMask: [0x%x]\n", rdMask);
	// printf("rd: [0x%x]\n", rd);
	// printf("sa: [0x%x]\n", sa);
	// printf("immediate: [0x%x]\n", immediate);
	// printf("target: [0x%x]\n", target);


	// Local Variables
	// CURRENT_STATE.REGS[0] = 5; // Example of how to put the operations in here

	switch (firstNums)
	{
	// Special case code
	case 0b000000:
		switch (lastNums)
		{
		case 0b100000:
			rd = rs + rt;
			printf("ADD: %lu = %lu + %lu\n", rd, rs, rt);
			break;
		case 0b100001:
			rd = rs + rt;
			printf("ADDU: %lu = %lu + %lu\n", rd, rs, rt);
			break;
		case 0b100010:
			rd = rs - rt;
			printf("SUB: %lu = %lu - %lu\n", rd, rs, rt);
			break;
		case 0b100011:
			rd = rs - rt;
			printf("SUBU: %lu = %lu - %lu\n", rd, rs, rt);
			break;
		case 0b011000:
			result = rs * rt;
			printf("MULT: %lu = %lu * %lu\n", result, rs, rt);
			break;
		case 0b011001:
			result = rs * rt;
			printf("MULTU: %lu = %lu * %lu\n", result, rs, rt);
			break;
		case 0b011010:
			result = rs / rt;
			printf("DIV: %lu = %lu / %lu\n", result, rs, rt);
			break;
		case 0b011011:
			result = rs / rt;
			printf("DIVU: %lu = %lu / %lu\n", result, rs, rt);
			break;
		case 0b100100:
			rd = rs & rt;
			printf("AND: %lu = %lu & %lu\n", rd, rs, rt);
			break;
		case 0b100101:
			rd = rs || rt;
			printf("OR: %lu = %lu || %lu\n", rd, rs, rt);
			break;
		case 0b100110:
			rd = rs ^ rt;
			printf("XOR: %lu = %lu ^ %lu\n", rd, rs, rt);
			break;
		case 0b100111:
			rd = !(rs || rt);
			printf("NOR: %lu = !(%lu || %lu)\n", rd, rs, rt);
			break;
		case 0b101010:
			if( rs < rt ){
				rd = 1;
			}
			else{
				rd = 0;
			}
			printf("SLT: %lu\n", rd);
			break;
		case 0b000000:
			rd = rt << sa;
			printf("SLL: %lu = %lu << %lu\n", rd, rt, sa);
			break;
		case 0b000010:
			rd = rt >> sa;
			printf("SRL: %lu = %lu >> %lu\n", rd, rt, sa);
			break;
		case 0b000011:
			rd = rt >> sa;
			printf("SRA: %lu = %lu >> %lu\n", rd, rt, sa);
			break;
		case 0b010000:
			rd = CURRENT_STATE.HI;
			printf("MFHI: %lu = %lu\n", rd, CURRENT_STATE.HI);
			break;
		case 0b010010:
			rd = CURRENT_STATE.LO;
			printf("MFLO: %lu = %lu\n", rd, CURRENT_STATE.LO);
			break;
		case 0b010001:
			// COME BACK
			printf("MDHI\n");
			break;
		case 0b010011:
			// COME BACK
			printf("MDLO\n");
			break;
		case 0b001000:
			// COME BACK
			printf("JR: Jump to %lu\n", rs);
			break;
		case 0b001001:
			// COME BACK
			printf("JALR: Jump to %lu with delay 1\n", rs);
			break;
		case 0b001100:
			printf("SYSCALL: Throw System Call excepetion\n");
			exit(0);
		default:
			printf("No Special Instruction Found\n");
			break;
		}
		break;
	
	// Register case code
	case 0b000110:
		if(rs <= 0){
			printf("BLEZ: Branch to %lu + \n", addr, immediate);
		}
		else{
			printf("BLEZ: No branch\n", rs);
		}
		break;
	case 0b000001:
		switch (rt){
		case 0b00000:
			if(rs < 0){
				printf("BLTZ: Branch to %lu + \n", addr, immediate);
			}
			else{
				printf("BLTZ: No branch\n");
			}
			break;
		case 0b00001:
			printf("BGEZ\n");
			break;
		default:
			printf("No Register Type Instruction Found\n");
			break;
	}
	case 0b000111:
		printf("BGTZ\n");
		break;

	// Normal case code
	case 0b001000:
		printf("ADDI\n");
		break;
	case 0b001001:
		printf("ADDIU\n");
		break;
	case 0b001100:
		printf("ANDI\n");
		break;
	case 0b001101:
		printf("ORI\n");
		break;
	case 0b001110:
		printf("XORI\n");
		break;
	case 0b001010:
		printf("SLTI\n");
		break;
	case 0b100011:
		printf("LW\n");
		break;
	case 0b100000:
		printf("LB\n");
		break;
	case 0b100001:
		printf("LH\n");
		break;
	case 0b001111:
		printf("LUI\n");
		break;
	case 0b101011:
		base = rt;
		printf("SW\n");
		break;
	case 0b101000:
		// COME BACK
		base = rt;
		printf("SB\n");
		break;
	case 0b101001:
		// COME BACK
		base = rt;
		printf("SH\n");
		break;
	case 0b000100:
		// COME BACK
		if(rs = rt){
			printf("BNE: Branch to %lu + \n", rs, rt);
		}
		printf("BEQ\n");
		break;
	case 0b000101:
		// COME BACK
		if(rs != rt){
			printf("BNE: Branch to %lu + \n", rs, rt);
		}
		printf("BNE\n");
		break;
	case 0b000010:
		// COME BACK
		printf("J: Jump to %lu\n", target);
		printf("J\n");
		break;
	case 0b000011:
		// COME BACK
		printf("JAL: Jump to %lu and link\n", target);
		printf("JAL\n");
		break;
	
	default:
		printf("No Normal Type Instruction Found\n");
		break;
	}
	printf("\n");

	instructionNum += 1;
}


/************************************************************/
/* Initialize Memory                                                                                                    */ 
/************************************************************/
void initialize() { 
	init_memory();
	CURRENT_STATE.PC = MEM_TEXT_BEGIN;
	NEXT_STATE = CURRENT_STATE;
	RUN_FLAG = TRUE;
}

/************************************************************/
/* Print the program loaded into memory (in MIPS assembly format)    */ 
/************************************************************/
void print_program(){
	int i;
	uint32_t addr;
	
	for(i=0; i<PROGRAM_SIZE; i++){
		addr = MEM_TEXT_BEGIN + (i*4);
		printf("[0x%x]\t", addr);
		print_instruction(addr);
	}
}

/************************************************************/
/* Print the instruction at given memory address (in MIPS assembly format)    */
/************************************************************/
void print_instruction(uint32_t addr){
	/*IMPLEMENT THIS*/

	// ALU Instructions: 
	// ADD, ADDU, ADDI, ADDIU, SUB, SUBU, MULT, MULTU, DIV, DIVU, AND, ANDI, OR, ORI, XOR, XORI, NOR, SLT, SLTI, SLL, SRL, SRA

	// Load/Store Instructions: 
	// LW, LB, LH, LUI, SW, SB, SH, MFHI, MFLO, MTHI, MTLO

	// Control Flow Instructions: 
	// BEQ, BNE, BLEZ, BLTZ, BGEZ, BGTZ, J, JR, JAL, JALR

	// System Call: 
	// SYSCALL (you should implement it to exit the program. To exit the program, the value of 10 (0xA in hex) should be in $v0 when SYSCALL is executed.
	enum ALU{
		ADD, 	// 000000, 100000
		ADDU, 	// 000000, 100001
		ADDI,	// 001000
		ADDIU,	// 001001
		SUB,	// 000000, 100010
		SUBU,	// 000000, 100011
		MULT, 	// 000000, 011000
		MULTU, 	// 000000, 011001
		DIV, 	// 000000, 011010
		DIVU, 	// 000000, 011011
		AND, 	// 000000, 100100
		ANDI, 	// 001100
		OR, 	// 000000, 100101
		ORI, 	// 001101
		XOR, 	// 000000, 100110
		XORI, 	// 001110
		NOR, 	// 000000, 100111
		SLT, 	// 000000, 101010
		SLTI, 	// 001010
		SLL, 	// 000000, 000000
		SRL, 	// 000000, 000010
		SRA		// 000000, 000011
	};

	enum LS{
		LW, 	// 100011
		LB, 	// 100000
		LH, 	// 100001
		LUI, 	// 001111
		SW, 	// 101011
		SB, 	// 101000
		SH, 	// 101001
		MFHI, 	// 000000, 010000
		MFLO, 	// 000000, 010010
		MTHI, 	// 000000, 010001
		MTLO	// 000000, 010011
	};

	enum CF{
		BEQ, 	// 000100
		BNE, 	// 000101
		BLEZ, 	// 000110, XXXXX, 00000
		BLTZ, 	// 000001, XXXXX, 00000
		BGEZ, 	// 000001, XXXXX, 00001
		BGTZ, 	// 000111, XXXXX, 00000
		J, 		// 000010
		JR, 	// 000000, 001000
		JAL, 	// 000011
		JALR	// 000000, 001001
	};

	uint32_t readAddr = mem_read_32(addr);
	uint32_t mask = 0xFC000000; // 28
	uint32_t r1Mask = 0x03E00000; // 21
	uint32_t r2Mask = 0x001F0000; // 16
	uint32_t lastMask = 0x0000003F;
	uint32_t firstNums = (readAddr & mask) >> 26;
	uint32_t r1Nums = (readAddr & r1Mask)  >> 21;
	uint32_t r2Nums = (readAddr & r2Mask)  >> 16;
	uint32_t lastNums = readAddr & lastMask;

	printf("\nInstruction: [0x%x]\n", readAddr);
	printf("First Nums: [0x%.2x]\n", firstNums);
	printf("R1 Nums: [0x%x]\n", r1Nums);
	printf("R2 Nums: [0x%x]\n", r2Nums);
	printf("Last Nums: [0x%x]\n", lastNums);


	switch (firstNums)
	{
	// Special case code
	case 0b000000:
		switch (lastNums)
		{
		case 0b100000:
			printf("ADD\n");
			break;
		case 0b100001:
			printf("ADDU\n");
			break;
		case 0b100010:
			printf("SUB\n");
			break;
		case 0b100011:
			printf("SUBU\n");
			break;
		case 0b011000:
			printf("MULT\n");
			break;
		case 0b011001:
			printf("MULTU\n");
			break;
		case 0b011010:
			printf("DIV\n");
			break;
		case 0b011011:
			printf("DIVU\n");
			break;
		case 0b100100:
			printf("AND\n");
			break;
		case 0b100101:
			printf("OR\n");
			break;
		case 0b100110:
			printf("XOR\n");
			break;
		case 0b100111:
			printf("NOR\n");
			break;
		case 0b101010:
			printf("SLT\n");
			break;
		case 0b000000:
			printf("SLL\n");
			break;
		case 0b000010:
			printf("SRL\n");
			break;
		case 0b000011:
			printf("SRA\n");
			break;
		case 0b010000:
			printf("MFHI\n");
			break;
		case 0b010010:
			printf("MFLO\n");
			break;
		case 0b010001:
			printf("MDHI\n");
			break;
		case 0b010011:
			printf("MDLO\n");
			break;
		case 0b001000:
			printf("JR\n");
			break;
		case 0b001001:
			printf("JALR\n");
			break;
		case 0b001100:
			printf("SYSCALL\n");
			exit(0);
		default:
			printf("No Special Instruction Found\n");
			break;
		}
		break;
	
	// Register case code
	case 0b000110:
		printf("BLEZ\n");
		break;
	case 0b000001:
		switch (r2Nums){
		case 0b00000:
			printf("BLTZ\n");
			break;
		case 0b00001:
			printf("BGEZ\n");
			break;
		default:
			printf("No Register Type Instruction Found\n");
			break;
	}
	case 0b000111:
		printf("BGTZ\n");
		break;

	// Normal case code
	case 0b001000:
		printf("ADDI\n");
		break;
	case 0b001001:
		printf("ADDIU\n");
		break;
	case 0b001100:
		printf("ANDI\n");
		break;
	case 0b001101:
		printf("ORI\n");
		break;
	case 0b001110:
		printf("XORI\n");
		break;
	case 0b001010:
		printf("SLTI\n");
		break;
	case 0b100011:
		printf("LW\n");
		break;
	case 0b100000:
		printf("LB\n");
		break;
	case 0b100001:
		printf("LH\n");
		break;
	case 0b001111:
		printf("LUI\n");
		break;
	case 0b101011:
		printf("SW\n");
		break;
	case 0b101000:
		printf("SB\n");
		break;
	case 0b101001:
		printf("SH\n");
		break;
	case 0b000100:
		printf("BEQ\n");
		break;
	case 0b000101:
		printf("BNE\n");
		break;
	case 0b000010:
		printf("J\n");
		break;
	case 0b000011:
		printf("JAL\n");
		break;
	
	default:
		printf("No Normal Type Instruction Found\n");
		break;
	}
	printf("\n");
}



/***************************************************************/
/* main                                                                                                                                   */
/***************************************************************/
int main(int argc, char *argv[]) {                              
	printf("\n**************************\n");
	printf("Welcome to MU-MIPS SIM...\n");
	printf("**************************\n\n");
	
	if (argc < 2) {
		printf("Error: You should provide input file.\nUsage: %s <input program> \n\n",  argv[0]);
		exit(1);
	}

	strcpy(prog_file, argv[1]);
	initialize();
	load_program();
	help();
	while (1){
		handle_command();
	}
	return 0;
}
