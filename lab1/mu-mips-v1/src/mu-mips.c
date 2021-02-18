#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include "mu-mips.h"

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
	char returnString[20];
	uint32_t start, stop, cycles;
	uint32_t register_no;
	int register_value;
	int hi_reg_value, lo_reg_value;

	printf("MU-MIPS SIM:> ");

	if (scanf("%s", returnString) == EOF){
		exit(0);
	}

	switch(returnString[0]) {
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
			if (returnString[1] == 'd' || returnString[1] == 'D'){
				rdump();
			}else if(returnString[1] == 'e' || returnString[1] == 'E'){
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
	// Incrementing address by instructNum bytes to get the right addresss
	// uint32_t addr = baseInstruction + (instructionNum * 4);

	// Reading Instruction and Relevant Masks
	uint32_t instruction = mem_read_32(CURRENT_STATE.PC); 			// Get the 32-bit instruction from memory
	uint32_t specialMask = 0xFC000000; 					// Mask for bits 26-31
	uint32_t rsMask = 0x03E00000; 						// Mask for bits 21-25
	uint32_t rtMask = 0x001F0000; 						// Mask for bits 16-20
	uint32_t rdMask = 0x0000F800;						// Mask for bits 11-15
	uint32_t functMask = 0x0000003F;					// Mask for bits 1 - 6
	uint32_t immediateMask = 0x0000FFFF; 				// Mask for bits 0 -16
	uint32_t targetMask = 0x03FFFFFF; 					// Mask for bits  0-26
	uint32_t saMask = 0x000007C0; 						// Mask for bits  6-20

	// Variables for R-Type Instructions
	uint32_t special 	= (instruction & specialMask) 	>> 26; 	// Shifting to get correct digits
	uint32_t rs 		= (instruction & rsMask)  		>> 21;
	uint32_t rt 		= (instruction & rtMask)  		>> 16;
	uint32_t rd 		= (instruction & rdMask)  		>> 11;
	uint32_t function 	= instruction & functMask;
	uint32_t sa 		= (instruction & saMask)		>>  6;

	// Variables for I-Type Instructions
	uint32_t immediate 	= instruction & immediateMask;

	// Variables for J-Type Instructions
	uint32_t target 	= instruction & targetMask;

	// Variables needed for operation
	char returnString[40];
	uint32_t result, value, value2, location;
	int jumpAmmount = 4; 
	uint64_t temp;
	uint32_t highBits = 0xFFFFFFFF;
	uint32_t offset = immediate;

	uint32_t base = rs;

	switch (special)
	{
	// Special case code
	case 0b000000:
		switch (function)
		{
		case 0b100000: //ADD instruction
			NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] + CURRENT_STATE.REGS[rt];
			sprintf(returnString, "ADD $r%d, $r%d, $r%d\n", rd, rs, rt);
			break;
				
		case 0b100001: //ADDU instruction
			NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] + CURRENT_STATE.REGS[rt];
			sprintf(returnString, "ADDU $r%d, $r%d, $r%d\n", rd, rs, rt);
			break;
				
		case 0b100010: //SUB instruction
			NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] - CURRENT_STATE.REGS[rt];
			sprintf(returnString, "SUB $r%d, $r%d, $r%d\n", rd, rs, rt);
			break;
				
		case 0b100011: //SUBU instruction
			NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] - CURRENT_STATE.REGS[rt];
			sprintf(returnString, "SUBU $r%d, $r%d, $r%d\n", rd, rs, rt);
			break;
				
		case 0b011000: //MULT instruction
			temp = CURRENT_STATE.REGS[rs] * CURRENT_STATE.REGS[rt];
			NEXT_STATE.HI = temp >> 32;
			NEXT_STATE.LO = temp & 0xFFFFFFFF;
			sprintf(returnString, "MULT $r%d, $r%d\n", rs, rt);
			break;
				
		case 0b011001:{ //MULTU instruction
			temp = CURRENT_STATE.REGS[rs] * CURRENT_STATE.REGS[rt];
			NEXT_STATE.HI = temp >> 32;
			NEXT_STATE.LO = temp & 0xFFFFFFFF;
			sprintf(returnString, "MULTU $r%d, $r%d\n", rs, rt);
			}
			break;
				
		case 0b011010: //DIV instruction
			NEXT_STATE.HI = CURRENT_STATE.REGS[rs] % CURRENT_STATE.REGS[rt];
			NEXT_STATE.LO = CURRENT_STATE.REGS[rs] / CURRENT_STATE.REGS[rt];
			sprintf(returnString, "DIV $r%d, $r%d\n", rs, rt);
			break;
				
		case 0b011011: //DIVU instruction
			NEXT_STATE.HI = CURRENT_STATE.REGS[rs] % CURRENT_STATE.REGS[rt];
			NEXT_STATE.LO = CURRENT_STATE.REGS[rs] / CURRENT_STATE.REGS[rt];
			sprintf(returnString, "DIVU $r%d, $r%d\n", rs, rt);
			break;
				
		case 0b100100: //AND instruction
			NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] & CURRENT_STATE.REGS[rt];
			sprintf(returnString, "AND $r%d, $r%d, $r%d\n", rd, rs, rt);
			break;
				
		case 0b100101: //OR instruction
			NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] | CURRENT_STATE.REGS[rt];
			sprintf(returnString, "OR $r%d, $r%d, $r%d\n", rd, rs, rt);
			break;
				
		case 0b100110: //XOR instruction
			NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] ^ CURRENT_STATE.REGS[rt];
			sprintf(returnString, "XOR $r%d, $r%d, $r%d\n", rd, rs, rt);
			break;
				
		case 0b100111: //NOR instruction
			NEXT_STATE.REGS[rd] = ~ (CURRENT_STATE.REGS[rs] | CURRENT_STATE.REGS[rt]);
			sprintf(returnString, "NOR $r%d, $r%d, $r%d\n", rd, rs, rt);
			break;
				
		case 0b101010: //SLT instruction
			if(CURRENT_STATE.REGS[rs] < CURRENT_STATE.REGS[rt])
			{
                		NEXT_STATE.REGS[rd] = 0x01;
			}
            		else
			{
                		NEXT_STATE.REGS[rd] = 0x00;
            		}
			sprintf(returnString, "SLT $r%d, $r%d, $r%d\n", rd, rs, rt);
			break;
				
		case 0b000000: //SLL instruction
			NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rt] << sa;
			sprintf(returnString, "SLL $r%d, $r%d, 0x%x\n", rd, rt, sa);
			break;
				
		case 0b000010: //SRL instruction
			NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rt] >> sa;
			sprintf(returnString, "SRL $r%d, $r%d, 0x%x\n", rd, rt, sa);
			break;
				
		case 0b000011: //SRA instruction
			if(CURRENT_STATE.REGS[rt] >> 31)
			{
				NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rt] >> sa;
				NEXT_STATE.REGS[rd] |= 0x80000000;
			}
			else
			{
				NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rt] >> sa;
			}
			sprintf(returnString, "SRA $r%d, $r%d, 0x%x\n", rd, rt, sa);
			break;
				
		case 0b010000: //MFHI instruction
			NEXT_STATE.REGS[rd] = CURRENT_STATE.HI;
			sprintf(returnString, "MFHI $r%d\n", rd);
			break;
				
		case 0b010010: //MFLO instruction
			NEXT_STATE.REGS[rd] = CURRENT_STATE.LO;
			sprintf(returnString, "MFLO $r%d\n", rd);
			break;

		case 0b010001:
			sprintf(returnString, "MTHI $r%d\n", rs);
			NEXT_STATE.HI = CURRENT_STATE.REGS[rs];
			break;

		case 0b010011:
			sprintf(returnString, "MTLO $r%d\n", rs);
			NEXT_STATE.LO = CURRENT_STATE.REGS[rs];
			break;

		case 0b001000:
			sprintf(returnString, "JR $r%d\n", rs);
			jumpAmmount = CURRENT_STATE.REGS[rs] - CURRENT_STATE.PC;
			break;

		case 0b001001:
			sprintf(returnString, "JALR $r%d, $r%d\n", rd, rs);
			NEXT_STATE.REGS[rd] = CURRENT_STATE.PC + 8;
			jumpAmmount =  CURRENT_STATE.REGS[rs] - CURRENT_STATE.PC;
			break;

		case 0b001100:
			sprintf(returnString, "SYSCALL\n");
			RUN_FLAG = FALSE;
			break;

		default:
			printf("No Special Instruction Found\n");
			break;
		}
		break;
	
	// Register case code
	case 0b000110:
		offset = offset << 2;
		sprintf(returnString, "BLEZ $r%d, 0x%x\n", rs, offset);
		if(((offset & 0x00008000)>>15)){
			offset = offset | 0xFFFF0000;
		}
		if(((CURRENT_STATE.REGS[rs] & 0x80000000)>>31) || (CURRENT_STATE.REGS[rs] == 0x00)){
			jumpAmmount = offset;
		}
		break;

	case 0b000001:
		switch (rt){
		case 0b00000:
			offset = offset << 2;
			sprintf(returnString, "BLTZ $r%d, 0x%x\n", rs, offset);
			// sign extend (check if most significant bit is a 1)
			if(((offset & 0x00008000)>>15)){
				offset = offset | 0xFFFF0000;
			}
			if(((CURRENT_STATE.REGS[rs] & 0x80000000)>>31) == 0x1){
				jumpAmmount = offset; // changed
			}
			break;

		case 0b00001:
			offset = offset << 2;
			sprintf(returnString, "BGEZ $r%d, 0x%x\n", rs, offset);
			// Do a sign extenstion only if the most signifcant bit is a 1
			if(((offset & 0x00008000)>>15)){
				offset = offset | 0xFFFF0000;
			}
			if(((CURRENT_STATE.REGS[rs] & 0x80000000)>>31) == 0x0 ){
				jumpAmmount = offset;
			}
			break;

		default:
			printf("No Register Type Instruction Found\n");
			break;
	}
	break;

	case 0b000111:
		offset = offset << 2;
		sprintf(returnString, "BGTZ $r%d, 0x%x\n", rs, offset);
		// Do a sign extenstion only if the most signifcant bit is a 1
		if(((offset & 0x00008000)>>15)){
			offset = offset | 0xFFFF0000;
		}
		if(((CURRENT_STATE.REGS[rs] & 0x80000000)>>31) == 0x0 && (CURRENT_STATE.REGS[rs] != 0x00)){
			jumpAmmount = offset;
		}
		break;

	// Normal case code
	case 0b001000:
		sprintf(returnString, "ADDI $r%d, $r%d, 0x%x\n", rt, rs, immediate);
		value = (immediate & 0x00008000) == 0x8000 ? 0xFFFF0000 | immediate : immediate;
		NEXT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] + value;
		break;

	case 0b001001:
		sprintf(returnString, "ADDIU $r%d, $r%d, 0x%x\n", rt, rs, immediate);
		value = (immediate & 0x00008000) == 0x8000 ? 0xFFFF0000 | immediate : immediate;
		NEXT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] + value;
		break;

	case 0b001100:
		sprintf(returnString, "ANDI $r%d, $r%d, 0x%x\n", rt, rs, immediate);
		NEXT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] & immediate;
		break;

	case 0b001101:
		sprintf(returnString, "ORI $r%d, $r%d, 0x%x\n", rt, rs, immediate);
		NEXT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] | immediate;
		break;

	case 0b001110:
		sprintf(returnString, "XORI $r%d, $r%d, 0x%x\n", rt, rs, immediate);
		NEXT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] ^ immediate;
		break;

	case 0b001010:
		sprintf(returnString, "SLTI $r%d, $r%d, 0x%x\n", rt, rs, immediate);
		// Do a sign extenstion only if the most signifcant bit is a 1
		if(((immediate & 0x00008000)>>15)){
			immediate = immediate | 0xFFFF0000;
		}
		if(CURRENT_STATE.REGS[rs] < immediate){
			NEXT_STATE.REGS[rt] = 0x01;
		}
		else{
			NEXT_STATE.REGS[rt] = 0x00;
		}
		break;

	case 0b100011:
		sprintf(returnString, "LW $r%d, 0x%x($r%d)\n", rt, immediate, rs);
		offset = (offset & 0x00008000) == 0x8000 ? 0xFFFF0000 | offset : offset;
		offset = CURRENT_STATE.REGS[base] + offset;
		NEXT_STATE.REGS[rt] = mem_read_32(offset);
		break;

	case 0b100000:
		sprintf(returnString, "LB $r%d, 0x%x($r%d)\n", rt, immediate, rs);
		offset = (offset & 0x00008000) == 0x8000 ? 0xFFFF0000 | offset : offset;
		value = CURRENT_STATE.REGS[base] + offset;
		value2 = mem_read_32(value) & 0x000000FF;
		value2 = (value2 & 0x00000080) == 0x80 ? 0xFFFFFF00 | value2 : value2;
		NEXT_STATE.REGS[rt] = value2;
		break;

	case 0b100001:
		sprintf(returnString, "LH $r%d, 0x%x($r%d)\n", rt, immediate, rs);
		offset = (offset & 0x00008000) == 0x8000 ? 0xFFFF0000 | offset : offset;
		value = CURRENT_STATE.REGS[base] + offset;
		value2 = mem_read_32(value) & 0x0000FFFF;
		value2 = (value2 & 0x00008000) == 0x8000 ? 0xFFFF0000 | value2 : value2;
		NEXT_STATE.REGS[rt] = value2;	
		break;

	case 0b001111:
		sprintf(returnString, "LUI $r%d, 0x%x\n", rt, immediate);
		NEXT_STATE.REGS[rt] = (immediate << 16);
		break;

	case 0b101011:
		sprintf(returnString, "SW $r%d, 0x%x($r%d)\n", rt, immediate, rs);
		if(((offset & 0x00008000)>>15)){
			offset = offset | 0xFFFF0000;
		}
		location = CURRENT_STATE.REGS[base] + offset;
		value = CURRENT_STATE.REGS[rt];
		mem_write_32(location, value);
		break;

	case 0b101000:
		sprintf(returnString, "SB $r%d, 0x%x($r%d)\n", rt, immediate, rs);
		if(((offset & 0x00008000)>>15)){
			offset = offset | 0xFFFF0000;
		}
		value = CURRENT_STATE.REGS[base] + offset;
		mem_write_32(value, CURRENT_STATE.REGS[rt] & 0x000000FF);
		break;

	case 0b101001:
		sprintf(returnString, "SH $r%d, 0x%x($r%d)\n", rt, immediate, rs);
		offset = (offset & 0x00008000) == 0x8000 ? 0xFFFF0000 | offset : offset;
		value = CURRENT_STATE.REGS[base] + offset;
		mem_write_32(value, CURRENT_STATE.REGS[rt] & 0x0000FFFF);
		break;

	case 0b000100:
		offset = offset << 2;
		sprintf(returnString, "BEQ $r%d, $r%d, 0x%x\n", rs, rt, offset);
		// Do a sign extenstion only if the most signifcant bit is a 1
		if(((offset & 0x00008000)>>15)){
			offset = offset | 0xFFFF0000;
		}
		if(CURRENT_STATE.REGS[rs] == CURRENT_STATE.REGS[rt]){
			printf("%x\n", CURRENT_STATE.PC + offset);

			// CHANGED THIS
			jumpAmmount = offset;
		}
		break;

	case 0b000101:
		offset = offset << 2;
		sprintf(returnString, "BNE $r%d, $r%d, 0x%x\n", rs, rt, offset);
		if(((offset & 0x00008000)>>15)){
			offset = offset | 0xFFFF0000;
		}
		if(CURRENT_STATE.REGS[rs] != CURRENT_STATE.REGS[rt]){
			jumpAmmount = offset;
		}
		break;

	case 0b000010:

		sprintf(returnString, "J %lu\n", (size_t)(target) << 2);
		target = target << 2;

		highBits = 0xF0000000 & CURRENT_STATE.PC;
		jumpAmmount = (target | highBits);// - CURRENT_STATE.PC;
		
		// try to find the offset, so that we can jump to that address
		offset = jumpAmmount - CURRENT_STATE.PC;
		jumpAmmount = offset;

		// Jump to that address with the delay of one instruction
		NEXT_STATE.PC = (CURRENT_STATE.PC & 0xF0000000) || target;
		break;

	case 0b000011:

		sprintf(returnString, "JAL %lu\n", (size_t)(target) << 2);
		target = target << 2;

		highBits = 0xF0000000 & CURRENT_STATE.PC;
		jumpAmmount = (target | highBits);
		
		// try to find the offset, so that we can jump to that address
		offset = jumpAmmount - CURRENT_STATE.PC;
		jumpAmmount = offset;

		// Jump to that address with the delay of one instruction
		NEXT_STATE.PC = (CURRENT_STATE.PC & 0xF0000000) || target;

		// Address of instruction after delay placed in link register
		NEXT_STATE.REGS[31] = CURRENT_STATE.PC + 8;
		break;

	default:
		printf("No Normal Type Instruction Found\n");
		break;
	}
	printf("[%x]\t", CURRENT_STATE.PC);
	printf("%s", returnString);

	NEXT_STATE.PC = CURRENT_STATE.PC + jumpAmmount;
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
	/*
	Program Must Contain These Instructions -
	ALU Instructions: 
	ADD, ADDU, ADDI, ADDIU, SUB, SUBU, MULT, MULTU, DIV, DIVU, AND, ANDI, OR, ORI, XOR, XORI, NOR, SLT, SLTI, SLL, SRL, SRA
	
	Load/Store Instructions: 
	LW, LB, LH, LUI, SW, SB, SH, MFHI, MFLO, MTHI, MTLO
	
	Control Flow Instructions: 
	BEQ, BNE, BLEZ, BLTZ, BGEZ, BGTZ, J, JR, JAL, JALR
	
	System Call: 
	SYSCALL (you should implement it to exit the program. To exit the program, the value of 10 (0xA in hex) should be in $v0 when SYSCALL is executed.
	*/

	// Reading Instruction and Relevant Masks
	uint32_t instruction = mem_read_32(addr); 			// Get the 32-bit instruction from memory
	uint32_t specialMask = 0xFC000000; 					// Mask for bits 26-31
	uint32_t rsMask = 0x03E00000; 						// Mask for bits 21-25
	uint32_t rtMask = 0x001F0000; 						// Mask for bits 16-20
	uint32_t rdMask = 0x0000F800;						// Mask for bits 11-15
	uint32_t functMask = 0x0000003F;					// Mask for bits 1 - 6
	uint32_t immediateMask = 0x0000FFFF; 				// Mask for bits 0 -16
	uint32_t targetMask = 0x03FFFFFF; 					// Mask for bits  0-26
	uint32_t saMask = 0x000007C0; 						// Mask for bits  6-20

	// Variables for R-Type Instructions
	uint32_t special 	= (instruction & specialMask) 	>> 26; 	// Shifting to get correct digits
	uint32_t rs 		= (instruction & rsMask)  		>> 21;
	uint32_t rt 		= (instruction & rtMask)  		>> 16;
	uint32_t rd 		= (instruction & rdMask)  		>> 11;
	uint32_t function 	= instruction & functMask;
	uint32_t sa 		= (instruction & saMask)		>>  6;

	// Variables for I-Type Instructions
	uint32_t immediate 	= instruction & immediateMask;

	// Variables for J-Type Instructions
	uint32_t target 	= instruction & targetMask;

	char returnString[40];

	switch (special)
	{
	// Special case code
	case 0b000000:
		switch (function)
		{
		case 0b100000:
			sprintf(returnString, "ADD $r%d, $r%d, $r%d\n", rd, rs, rt);
			break;
		case 0b100001:
			sprintf(returnString, "ADDU $r%d, $r%d, $r%d\n", rd, rs, rt);
			break;
		case 0b100010:
			sprintf(returnString, "SUB $r%d, $r%d, $r%d\n", rd, rs, rt);
			break;
		case 0b100011:
			sprintf(returnString, "SUBU $r%d, $r%d, $r%d\n", rd, rs, rt);
			break;
		case 0b011000:
			sprintf(returnString, "MULT $r%d, $r%d\n", rs, rt);
			break;
		case 0b011001:
			sprintf(returnString, "MULTU $r%d, $rr%d\n", rs, rt);
			break;
		case 0b011010:
			sprintf(returnString, "DIV $r%d, $r%d\n", rs, rt);
			break;
		case 0b011011:
			sprintf(returnString, "DIVU $r%d, $r%d\n", rs, rt);
			break;
		case 0b100100:
			sprintf(returnString, "AND $r%d, $r%d, $r%d\n", rd, rs, rt);
			break;
		case 0b100101:
			sprintf(returnString, "OR $r%d, $r%d, $r%d\n", rd, rs, rt);
			break;
		case 0b100110:
			sprintf(returnString, "XOR $r%d, $r%d, $r%d\n", rd, rs, rt);
			break;
		case 0b100111:
			sprintf(returnString, "NOR $r%d, $r%d, $r%d\n", rd, rs, rt);
			break;
		case 0b101010:
			sprintf(returnString, "SLT $r%d, $r%d, $r%d\n", rd, rs, rt);
			break;
		case 0b000000:
			sprintf(returnString, "SLL $r%d, $r%d, 0x%x\n", rd, rt, sa);
			break;
		case 0b000010:
			sprintf(returnString, "SRL $r%d, $r%d, 0x%x\n", rd, rt, sa);
			break;
		case 0b000011:
			sprintf(returnString, "SRA $r%d, $r%d, 0x%d\n", rd, rt, sa);
			break;
		case 0b010000:
			sprintf(returnString, "MFHI $r%d\n", rd);
			break;
		case 0b010010:
			sprintf(returnString, "MFLO $r%d\n", rd);
			break;
		case 0b010001:
			sprintf(returnString, "MTHI $r%d\n", rs);
			break;
		case 0b010011:
			sprintf(returnString, "MTLO $r%d\n", rs);
			break;
		case 0b001000:
			sprintf(returnString, "JR $r%d\n", rs);
			break;
		case 0b001001:
			sprintf(returnString, "JALR $r%d, $r%d\n", rd, rs);
			break;
		case 0b001100:
			sprintf(returnString, "SYSCALL\n");
			break;
		default:
			printf("No Special Instruction Found\n");
			break;
		}
		break;
	
	// Register case code
	case 0b000110:
		sprintf(returnString, "BLEZ $r%d, 0x%x\n", rs, immediate);
		break;
	case 0b000001:
		switch (rt){
		case 0b00000:
			sprintf(returnString, "BLTZ $r%d, 0x%x\n", rs, immediate);
			break;
		case 0b00001:
			sprintf(returnString, "BGEZ $r%d, 0x%x\n", rs, immediate);
			break;
		default:
			printf("No Register Type Instruction Found\n");
			break;
	}
	case 0b000111:
		sprintf(returnString, "BGTZ $r%d, 0x%x\n", rs, immediate);
		break;

	// Normal case code
	case 0b001000:
		sprintf(returnString, "ADDI $r%d, $r%d, 0x%x\n", rt, rs, immediate);
		break;
	case 0b001001:
		sprintf(returnString, "ADDIU $r%d, $r%d, 0x%x\n", rt, rs, immediate);
		break;
	case 0b001100:
		sprintf(returnString, "ANDI $r%d, $r%d, 0x%x\n", rt, rs, immediate);
		break;
	case 0b001101:
		sprintf(returnString, "ORI $r%d, $r%d, 0x%x\n", rt, rs, immediate);
		break;
	case 0b001110:
		sprintf(returnString, "XORI $r%d, $r%d, 0x%x\n", rt, rs, immediate);
		break;
	case 0b001010:
		sprintf(returnString, "SLTI $r%d, $r%d, 0x%x\n", rt, rs, immediate);
		break;
	case 0b100011:
		sprintf(returnString, "LW $r%d, 0x%x($r%d)\n", rt, immediate, rs);
		break;
	case 0b100000:
		sprintf(returnString, "LB $r%d, 0x%x($r%d)\n", rt, immediate, rs);
		break;
	case 0b100001:
		sprintf(returnString, "LH $r%d, 0x%x($r%d)\n", rt, immediate, rs);
		break;
	case 0b001111:
		sprintf(returnString, "LUI $r%d, 0x%x\n", rt, immediate);
		break;
	case 0b101011:
		sprintf(returnString, "SW $r%d, 0x%x($r%d)\n", rt, immediate, rs);
		break;
	case 0b101000:
		sprintf(returnString, "SB $r%d, 0x%x($r%d)\n", rt, immediate, rs);
		break;
	case 0b101001:
		sprintf(returnString, "SH $r%d, 0x%x($r%d)\n", rt, immediate, rs);
		break;
	case 0b000100:
		sprintf(returnString, "BEQ $r%d, $r%d, 0x%x\n", rs, rt, immediate);
		break;
	case 0b000101:
		sprintf(returnString, "BNE $r%d, $r%d, 0x%x\n", rs, rt, immediate);
		break;
	case 0b000010:
		sprintf(returnString, "J %lu\n", (size_t)(target) << 2);
		break;
	case 0b000011:
		sprintf(returnString, "JAL %lu\n", (size_t)(target) << 2);
		break;
	
	default:
		printf("No Normal Type Instruction Found\n");
		break;
	}

	printf("%s", returnString);
	
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
