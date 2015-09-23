#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "printRoutines.h"

#define ERROR_RETURN -1
#define SUCCESS 0

int conditional_mov(char upper, char lower, char Rs);
int	immediate_2reg(char upper, char lower, char Rs, char v1, char v2, char v3, char v4);
int	register_2memry(char upper, char lower, char Rs, char d1, char d2, char d3, char d4);
int	memry_2register(char upper, char lower, char Rs, char d1, char d2, char d3, char d4);
int	arithmetic_op(char upper, char lower, char Rs);

int	jump_instruct(char upper, char lower, char d1, char d2, char d3, char d4);
int	call_instruct(char upper, char lower, char d1, char d2, char d3, char d4);
int	return_instruct(char upper, char lower);
int	pushl_instruct(char upper, char lower, char Rs);
int	popl_instruct(char upper, char lower, char Rs);

char** getRegister(int rA, int rB, int firstDimension, int secondDimension);

FILE *f;
long currAddr = 0; 
char buff[256];
int bytesWritten;
int res;
int start;
	
int main(int argc, char **argv) {
  int machineCode = -1;
  // Verify that the command line has an appropriate number
  // of arguments
  if (argc < 3 || argc > 4) {
    printf("Usage: %s InputFilename OutputFilename [startingOffset]\n", argv[0]);
    return ERROR_RETURN;
  }
  // First argument is the file to open, attempt to open it 
  // for reading and verify that the open did occur.
  machineCode = open(argv[1], O_RDONLY);
  if (machineCode < 0) {
    printf("Failed to open: %s\n", argv[1]);
    return ERROR_RETURN;
  }
  // If there is a 3rd argument present it is an offset so
  // convert it to a value. 
  if (4 == argc) {
    // See man page for strtol() as to why
    // we check for errors by examining errno
    errno = 0;
    currAddr = strtol(argv[3], NULL, 0);
    if (errno != 0) {
      perror("Invalid offset on command line");
	  start = 0;
      return ERROR_RETURN;
    }
	start = strtol(argv[3], NULL, 10);
  } else
		start = 0;
  
    /* Comment or delete the following line and this comment before
   * handing in your final version. Note 2 is stderr
   */
  // Your code starts here don't forget to open the output file.
  
  // Open the output file.
  f = fopen(argv[2], "w");
  // If null print error. 
  if (f==NULL){
		printf("Error opening file specified in second argument.");
		exit(1);
	}
		
  // Get file descriptor of output.
  res = fileno(f);
  printf("Opened %s, starting offset 0x%08X\n", argv[1], currAddr);

  FILE *fileptr;
  char *buffer;
  long filelen;
  
  //open binary file to read. fileptr points to start of file.
  fileptr = fopen(argv[1], "rb");
  //go to the end of the file. fileptr points to end of file.
  fseek(fileptr,0, SEEK_END);
  //ftell gets the number of bytes from the beginning of file.
  filelen = ftell(fileptr);
  //fileptr points to start of instructions.
  rewind(fileptr);
  
  //allocate filelen bytes of space in an array of chars.  
  buffer = (char *)malloc((filelen+1)*sizeof(char));
  // Read in the entire file into the buffer array.
  fread(buffer, filelen, 1, fileptr); 
  //close the fileptr pointer.
  fclose(fileptr);  
  //Now we have an array of bytes. Convert them to assembly.
  bytesToAssembly(buffer, filelen+1, start);  
  free(buffer);
  //upon return, success. 
  return SUCCESS;
}


int bytesToAssembly(char *buffer, int length, int start){
	int i;
	int n;
	//read in byte by byte the binary file.
	for (i=start; i<length;i++){
		char b = buffer[i];
		//obtain upper order nibble
		char nu = b >> 4;
		//obtain low order nibble
		char nl = b & 15;
		
		switch(nu){
			case 0x0 :
				currAddr = currAddr+1;
				continue;
			case 0x1 :
				currAddr = currAddr+1;
				continue;
			case 0x2 :
				conditional_mov(nu, nl, buffer[i+1]);
				i = i+1;
				currAddr = currAddr+2;
				continue;
			case 0x3 :
				immediate_2reg(nu, nl, buffer[i+1], buffer[i+2], 
										buffer[i+3], buffer[i+4], buffer[i+5]);	
				i = i+5;	
				currAddr = currAddr+6;
				continue;
			case 0x4 :
				register_2memry(nu, nl, buffer[i+1], buffer[i+2], 
										buffer[i+3], buffer[i+4], buffer[i+5]);						
				i = i+5;
				currAddr = currAddr+6;			
				continue;
			case 0x5 :
				memry_2register(nu, nl, buffer[i+1], buffer[i+2], 
										buffer[i+3], buffer[i+4], buffer[i+5]);	
				i = i+5;
				currAddr = currAddr+6;
				continue;
			case 0x6 :
				n = arithmetic_op(nu, nl, buffer[i+1]);
				if (n>0){
					i = i+1;
					currAddr = currAddr+2;
					continue;
				} else { 
					printf("# Error encountered during read, aborting...\n");
					break;
				}
			case 0x7 :
				jump_instruct(nu, nl, buffer[i+1], buffer[i+2], 
										buffer[i+3], buffer[i+4]);
				i = i+4;
				currAddr = currAddr+5;
				continue;			
			case 0x8 :
				call_instruct(nu, nl, buffer[i+1], buffer[i+2], 
										buffer[i+3], buffer[i+4]);
				i = i+4;
				currAddr = currAddr+5;			
				continue;			
			case 0x9 :
				return_instruct(nu, nl);
				currAddr = currAddr+1;
				continue;
			case 0XA :
				pushl_instruct(nu, nl, buffer[i+1]);
				i = i+1;
				currAddr = currAddr+2;
				continue;
			case 0XB :
				popl_instruct(nu, nl, buffer[i+1]);
				i=i+1;
				currAddr = currAddr+2;
				continue;
			default:
				printf("# Invalid Instruction Encountered...\n");
				continue;
		}	
	}
	return 1;
}

int conditional_mov(char upper, char lower, char Rs){
// obtain the bytes for the registers needed
	char rA = Rs >> 4;
	char rB = Rs & 15;
	
	// convert bytes to integer representation
	
	int n;	
	int r1 = rA;
	int r2 = rB;	
	char buffer[12];	
	int intup = (int) upper;
	int intlow = (int) lower;

	switch (lower){
		case 0x0:
		{
			// get the string representation of the registers
			char** regs = getRegister(r1, r2, 2, 4);
			// get the string representation of first rA
			char* regA = regs[0];
			// get the string representation of first rB
			char* regB = regs[1];
			// if the registers are valid, print to output file.
			if ((!strcmp(regA, "err")==0) && (!strcmp(regB, "err")==0)){
				n = sprintf(buffer, "%d%d%d%d", intup, intlow, r1, r2);
				if (n>0){
					printf("%08x: %-14s%-8s  %s, %s\n", currAddr, buffer, "rrmovl", regA, regB);
					n=snprintf(buff, 256, "%08x: %-14s%-8s  %s, %s\n", currAddr, buffer, "rrmovl", regA, regB);
					res = fileno(f);
					write(res, buff, n );
				} else return -1;
			} else {
				// One of the registers was invalid.
				printf("# Invalid Instruction Encountered...\n");
			}
			
			break;
		}
		case 0x1:
		{
			// get the string representation of the registers
			char** regs = getRegister(r1, r2,2,4);
			// get the string representation of first rA
			char* regA = regs[0];
			// get the string representation of first rB
			char* regB = regs[1];
			if ((!strcmp(regA, "err")==0) && (!strcmp(regB, "err")==0)){
				n = sprintf(buffer, "%d%d%d%d", intup, intlow, r1, r2);
				if (n>0){
					printf("%08x: %-14s%-8s  %s, %s\n", currAddr, buffer, "cmovle", regA, regB);
					n=snprintf(buff, 256, "%08x: %-14s%-8s  %s, %s\n", currAddr, buffer, "cmovle", regA, regB);
					res = fileno(f);
					write(res, buff, n );
				} else return -1;	
			} else {
				// One of the registers was invalid.
				printf("# Invalid Instruction Encountered...\n");
			}
			
			break;
		}
		case 0x2:
		{
			// get the string representation of the registers
			char** regs = getRegister(r1, r2,2,4);
			// get the string representation of first rA
			char* regA = regs[0];
			// get the string representation of first rB
			char* regB = regs[1];
			if ((!strcmp(regA, "err")==0) && (!strcmp(regB, "err")==0)){
				n = sprintf(buffer, "%d%d%d%d", intup, intlow, r1, r2);
				if (n>0){
					printf("%08x: %-14s%-8s  %s, %s\n", currAddr, buffer, "cmovl", regA, regB);
					n=snprintf(buff, 256, "%08x: %-14s%-8s  %s, %s\n", currAddr, buffer, "cmovl", regA, regB);
					res = fileno(f);
					write(res, buff, n );
				} else return -1;	
			} else {
				// One of the registers was invalid.
				printf("# Invalid Instruction Encountered...\n");
			}
			
			break;
		}
		case 0x3:
		{
			// get the string representation of the registers
			char** regs = getRegister(r1, r2,2,4);
			// get the string representation of first rA
			char* regA = regs[0];
			// get the string representation of first rB
			char* regB = regs[1];
			// if the registers are valid, print to output file.
			if ((!strcmp(regA, "err")==0) && (!strcmp(regB, "err")==0)){
				n = sprintf(buffer, "%d%d%d%d", intup, intlow, r1, r2);
				if (n>0){
					printf("%08x: %-14s%-8s  %s, %s\n", currAddr, buffer, "cmove", regA, regB);
					n=snprintf(buff, 256, "%08x: %-14s%-8s  %s, %s\n", currAddr, buffer, "cmove", regA, regB);
					res = fileno(f);
					write(res, buff, n );
				} else return -1;		
			} else {
				// One of the registers was invalid.
				printf("# Invalid Instruction Encountered...\n");
			}			
			
			break;
		}
		case 0x4:
		{
			// get the string representation of the registers
			char** regs = getRegister(r1, r2,2,4);
			// get the string representation of first rA
			char* regA = regs[0];
			// get the string representation of first rB
			char* regB = regs[1];
			if ((!strcmp(regA, "err")==0) && (!strcmp(regB, "err")==0)){
				n = sprintf(buffer, "%d%d%d%d", intup, intlow, r1, r2);
				if (n>0){
					printf("%08x: %-14s%-8s  %s, %s\n", currAddr, buffer, "cmovs", regA, regB);
					n=snprintf(buff, 256, "%08x: %-14s%-8s  %s, %s\n", currAddr, buffer, "cmovs", regA, regB);
					res = fileno(f);
					write(res, buff, n );
				} else return -1;	
			} else {
				// One of the registers was invalid.
				printf("# Invalid Instruction Encountered...\n");
			}
			
			break;
		}
		case 0x5:
		{
			// get the string representation of the registers
			char** regs = getRegister(r1, r2,2,4);
			// get the string representation of first rA
			char* regA = regs[0];
			// get the string representation of first rB
			char* regB = regs[1];
			if ((!strcmp(regA, "err")==0) && (!strcmp(regB, "err")==0)){
				n = sprintf(buffer, "%d%d%d%d", intup, intlow, r1, r2);
				if (n>0){
					printf("%08x: %-14s%-8s  %s, %s\n", currAddr, buffer, "cmovge", regA, regB);
					n=snprintf(buff, 256, "%08x: %-14s%-8s  %s, %s\n", currAddr, buffer, "cmovge", regA, regB);
					res = fileno(f);
					write(res, buff, n );
				} else return -1;	
			} else {
				// One of the registers was invalid.
				printf("# Invalid Instruction Encountered...\n");
			}
			
			break;
		}

		case 0x6:
		{
			// get the string representation of the registers
			char** regs = getRegister(r1, r2,2,4);
			// get the string representation of first rA
			char* regA = regs[0];
			// get the string representation of first rB
			char* regB = regs[1];
			if ((!strcmp(regA, "err")==0) && (!strcmp(regB, "err")==0)){
				n = sprintf(buffer, "%d%d%d%d", intup, intlow, r1, r2);
				if (n>0){
					printf("%08x: %-14s%-8s  %s, %s\n", currAddr, buffer, "cmovg", regA, regB);
					n=snprintf(buff, 256, "%08x: %-14s%-8s  %s, %s\n", currAddr, buffer, "cmovg", regA, regB);
					res = fileno(f);
					write(res, buff, n );
				} else return -1;
			} else {
				// One of the registers was invalid.
				printf("# Invalid Instruction Encountered...\n");
			}
			
			break;
		}
		
		default:
		{
			// Upper nibble is invalid.
			printf("# Invalid Instruction Encountered...\n");
			break;	
		}
	}
	return 1;
}
	
int	immediate_2reg(char upper, char lower, char Rs, char v1, char v2, char v3, char v4){
	// F and rB
	char rA = Rs >> 4;
	char rB = Rs & 15;
	
	char v1a = v1 >> 4;
	char v1b = v1 & 15;
	
	char v2a = v2 >> 4;
	char v2b = v2 & 15;
	
	char v3a = v3 >> 4;
	char v3b = v3 & 15;
	
	char v4a = v4 >> 4;
	char v4b = v4 & 15;
	
	// convert bytes to integer
	int n;
	int r1 = rA;
	int r2 = rB;
	int intup  = upper;
	int intlow = lower;
	char buffer1[255];
	char bufferrev[255];
	char buffer2[255];
	
	n = sprintf(bufferrev, "%d%d%d%d%d%d%d%d", v4a, v4b, v3a, v3b, v2a, v2b, v1a, v1b);	
	// create string for the immediate value being written to register
	n = sprintf(buffer1, "%d%d%d%d%d%d%d%d", v1a, v1b, v2a, v2b, v3a, v3b, v4a, v4b);
	// get the string representation of the registers
	char** regs = getRegister(8, r2, 2, 4);
	// get the string representation of first rA
	char* regA = regs[0];
	// get the string representation of first rB
	char* regB = regs[1];
	if ((!strcmp(regA, "err")==0) && (!strcmp(regB, "err")==0)){
		n = sprintf(buffer2, "%d%d%c%d%s", intup, intlow, 'F', r2, buffer1);
		if (n>0){
			printf("%08x: %-14s%-8s  %c%s, %s\n", currAddr, buffer2, "irmovl", '$', bufferrev, regB);
			n=snprintf(buff, 256, "%08x: %-14s%-8s  %c%s, %s\n", currAddr, buffer2, "irmovl", '$', bufferrev, regB);
			write(res, buff, n );
		} else return -1;
	} else {
		// One of the registers was invalid.
		printf("# Invalid Instruction Encountered...\n");
	}

	return 1;	
}
	
int	register_2memry(char upper, char lower, char Rs, char d1, char d2, char d3, char d4){
	// F and rB
	char rA = Rs >> 4;
	char rB = Rs & 15;
	
	char d1a = d1 >> 4;
	char d1b = d1 & 15;
	
	char d2a = d2 >> 4;
	char d2b = d2 & 15;
	
	char d3a = d3 >> 4;
	char d3b = d3 & 15;
	
	char d4a = d4 >> 4;
	char d4b = d4 & 15;
	
	// convert bytes to integer
	int n;
	int r1 = rA;
	int r2 = rB;
	int intup  = upper;
	int intlow = lower;
	char buffer1[255];
	char bufferrev[255];
	char buffer2[255];
	
	n = sprintf(bufferrev, "%d%d%d%d%d%d%d%d", d4a, d4b, d3a, d3b, d2a, d2b, d1a, d1b);
	// create string for the immediate value being written to register
	n = sprintf(buffer1, "%d%d%d%d%d%d%d%d", d1a, d1b, d2a, d2b, d3a, d3b, d4a, d4b);
	// get the string representation of the registers
	char** regs = getRegister(r1, r2, 2, 4);
	// get the string representation of first rA
	char* regA = regs[0];
	// get the string representation of first rB
	char* regB = regs[1];
	
	if ((!strcmp(regA, "err")==0) && (!strcmp(regB, "err")==0)){
		n = sprintf(buffer2, "%d%d%d%d%s", intup, intlow, r1, r2, buffer1);
		if (n>0){
			printf("%08x: %-14s%-8s  %s, %s(%s)\n", currAddr, buffer2, "rmmovl", regA, bufferrev, regB);
			n=snprintf(buff, 256,"%08x: %-14s%-8s  %s, %s(%s)\n", currAddr, buffer2, "rmmovl", regA, bufferrev, regB);
			res = fileno(f);
			write(res, buff, n );
		} else return -1;
	} else {
		// One of the registers was invalid.
		printf("# Invalid Instruction Encountered...\n");
	}

	return 1;		
}
	
int	memry_2register(char upper, char lower, char Rs, char d1, char d2, char d3, char d4){
		// F and rB
	char rA = Rs >> 4;
	char rB = Rs & 15;
	
	char d1a = d1 >> 4;
	char d1b = d1 & 15;
	
	char d2a = d2 >> 4;
	char d2b = d2 & 15;
	
	char d3a = d3 >> 4;
	char d3b = d3 & 15;
	
	char d4a = d4 >> 4;
	char d4b = d4 & 15;
	
	// convert bytes to integer
	int n;
	int r1 = rA;
	int r2 = rB;
	int intup  = upper;
	int intlow = lower;
	char buffer1[255];
	char bufferrev[255];
	char buffer2[255];
	
	n = sprintf(bufferrev, "%d%d%d%d%d%d%d%d", d4a, d4b, d3a, d3b, d2a, d2b, d1a, d1b);
	// create string for the immediate value being written to register
	n = sprintf(buffer1, "%d%d%d%d%d%d%d%d", d1a, d1b, d2a, d2b, d3a, d3b, d4a, d4b);
	// get the string representation of the registers
	char** regs = getRegister(r1, r2, 2, 4);
	// get the string representation of first rA
	char* regA = regs[0];
	// get the string representation of first rB
	char* regB = regs[1];
	
	if ((!strcmp(regA, "err")==0) && (!strcmp(regB, "err")==0)){
		n = sprintf(buffer2, "%d%d%d%d%s", intup, intlow, r1, r2, buffer1);
		if (n>0){
			printf("%08x: %-14s%-8s  %s(%s), %s\n", currAddr, buffer2, "mrmovl", bufferrev, regB, regA);
			n=snprintf(buff, 256,"%08x: %-14s%-8s  %s(%s), %s\n", currAddr, buffer2, "mrmovl", bufferrev, regB, regA);
			res = fileno(f);
			write(res, buff, n );
		} else return -1;
	} else {
		// One of the registers was invalid.
		printf("# Invalid Instruction Encountered...\n");
	}
	return 1;			
}
	
int	arithmetic_op(char upper, char lower, char Rs){
	// obtain the bytes for the registers needed
	char rA = Rs >> 4;
	char rB = Rs & 15;
	
	// convert bytes to integer representation
	
	int n;	
	int r1 = rA;
	int r2 = rB;	
	char buffer[12];	
	int intup = (int) upper;
	int intlow = (int) lower;

	switch (lower){
		case 0x0:
		{
			// get the string representation of the registers
			char** regs = getRegister(r1, r2, 2, 4);
			// get the string representation of first rA
			char* regA = regs[0];
			// get the string representation of first rB
			char* regB = regs[1];
			// if the registers are valid, print to output file.
			if ((!strcmp(regA, "err")==0) && (!strcmp(regB, "err")==0)){
				n = sprintf(buffer, "%d%d%d%d", intup, intlow, r1, r2);
				if (n>0){
					printf("%08x: %-14s%-8s  %s, %s\n", currAddr, buffer, "addl", regA, regB);
					n=snprintf(buff, 256, "%08x: %-14s%-8s  %s, %s\n", currAddr, buffer, "addl", regA, regB);
					res = fileno(f);
					write(res, buff, n );
				} else return -1;
			} else {
				// One of the registers was invalid.
				printf("# Invalid Instruction Encountered...\n");
			}
			
			break;
		}
		case 0x1:
		{
			// get the string representation of the registers
			char** regs = getRegister(r1, r2,2,4);
			// get the string representation of first rA
			char* regA = regs[0];
			// get the string representation of first rB
			char* regB = regs[1];
			if ((!strcmp(regA, "err")==0) && (!strcmp(regB, "err")==0)){
				n = sprintf(buffer, "%d%d%d%d", intup, intlow, r1, r2);
				if (n>0){
					printf("%08x: %-14s%-8s  %s, %s\n", currAddr, buffer, "subl", regA, regB);
					n=snprintf(buff, 256, "%08x: %-14s%-8s  %s, %s\n", currAddr, buffer, "subl", regA, regB);
					res = fileno(f);
					write(res, buff, n );
				} else return -1;	
			} else {
				// One of the registers was invalid.
				printf("# Invalid Instruction Encountered...\n");
			}
			
			break;
		}
		case 0x2:
		{
			// get the string representation of the registers
			char** regs = getRegister(r1, r2,2,4);
			// get the string representation of first rA
			char* regA = regs[0];
			// get the string representation of first rB
			char* regB = regs[1];
			if ((!strcmp(regA, "err")==0) && (!strcmp(regB, "err")==0)){
				n = sprintf(buffer, "%d%d%d%d", intup, intlow, r1, r2);
				if (n>0){
					printf("%08x: %-14s%-8s  %s, %s\n", currAddr, buffer, "andl", regA, regB);
					n=snprintf(buff, 256, "%08x: %-14s%-8s  %s, %s\n", currAddr, buffer, "andl", regA, regB);
					res = fileno(f);
					write(res, buff, n );
				} else return -1;	
			} else {
				// One of the registers was invalid.
				printf("# Invalid Instruction Encountered...\n");
			}
			
			break;
		}
		case 0x3:
		{
			// get the string representation of the registers
			char** regs = getRegister(r1, r2,2,4);
			// get the string representation of first rA
			char* regA = regs[0];
			// get the string representation of first rB
			char* regB = regs[1];
			// if the registers are valid, print to output file.
			if ((!strcmp(regA, "err")==0) && (!strcmp(regB, "err")==0)){
				n = sprintf(buffer, "%d%d%d%d", intup, intlow, r1, r2);
				if (n>0){
					printf("%08x: %-14s%-8s  %s, %s\n", currAddr, buffer, "xorl", regA, regB);
					n=snprintf(buff, 256, "%08x: %-14s%-8s  %s, %s\n", currAddr, buffer, "xorl", regA, regB);
					res = fileno(f);
					write(res, buff, n );
				} else return -1;				
			} else {
				// One of the registers was invalid.
				printf("# Invalid Instruction Encountered...\n");
			}			
			
			break;
		}
		case 0x4:
		{
			// get the string representation of the registers
			char** regs = getRegister(r1, r2,2,4);
			// get the string representation of first rA
			char* regA = regs[0];
			// get the string representation of first rB
			char* regB = regs[1];
			if ((!strcmp(regA, "err")==0) && (!strcmp(regB, "err")==0)){
				n = sprintf(buffer, "%d%d%d%d", intup, intlow, r1, r2);
				if (n>0){
					printf("%08x: %-14s%-8s  %s, %s\n", currAddr, buffer, "mull", regA, regB);
					n=snprintf(buff, 256, "%08x: %-14s%-8s  %s, %s\n", currAddr, buffer, "mull", regA, regB);
					res = fileno(f);
					write(res, buff, n );
				} else return -1;	
			} else {
				// One of the registers was invalid.
				printf("# Invalid Instruction Encountered...\n");
			}
			
			break;
		}
		case 0x5:
		{
			// get the string representation of the registers
			char** regs = getRegister(r1, r2,2,4);
			// get the string representation of first rA
			char* regA = regs[0];
			// get the string representation of first rB
			char* regB = regs[1];
			if ((!strcmp(regA, "err")==0) && (!strcmp(regB, "err")==0)){
				n = sprintf(buffer, "%d%d%d%d", intup, intlow, r1, r2);
				if (n>0){
					printf("%08x: %-14s%-8s  %s, %s\n", currAddr, buffer, "divl", regA, regB);
					n=snprintf(buff, 256, "%08x: %-14s%-8s  %s, %s\n", currAddr, buffer, "divl", regA, regB);
					res = fileno(f);
					write(res, buff, n );
				} else return -1;	
			} else {
				// One of the registers was invalid.
				printf("# Invalid Instruction Encountered...\n");
			}
			
			break;
		}

		case 0x6:
		{
			// get the string representation of the registers
			char** regs = getRegister(r1, r2,2,4);
			// get the string representation of first rA
			char* regA = regs[0];
			// get the string representation of first rB
			char* regB = regs[1];
			if ((!strcmp(regA, "err")==0) && (!strcmp(regB, "err")==0)){
				n = sprintf(buffer, "%d%d%d%d", intup, intlow, r1, r2);
				if (n>0){
					printf("%08x: %-14s%-8s  %s, %s\n", currAddr, buffer, "modl", regA, regB);
					n=snprintf(buff, 256, "%08x: %-14s%-8s  %s, %s\n", currAddr, buffer, "modl", regA, regB);
					res = fileno(f);
					write(res, buff, n );
				} else return -1;
			} else {
				// One of the registers was invalid.
				printf("# Invalid Instruction Encountered...\n");
			}
			
			break;
		}
		default:
		{
			// Upper nibble is invalid.
			printf("# Invalid Instruction Encountered...\n");
			break;	
		}
	}
	return 1;
}
	
int	jump_instruct(char upper, char lower, char d1, char d2, char d3, char d4){
		// F and rB
	char d1a = d1 >> 4;
	char d1b = d1 & 15;
	
	char d2a = d2 >> 4;
	char d2b = d2 & 15;
	
	char d3a = d3 >> 4;
	char d3b = d3 & 15;
	
	char d4a = d4 >> 4;
	char d4b = d4 & 15;
	
	// convert bytes to integer
	int intup  = upper;
	int intlow = lower;
	int n;
	char buffer1[255];
	char bufferrev[255];
	char buffer2[255];
	
	n = sprintf(bufferrev, "%d%d%d%d%d%d%d%d", d4a, d4b, d3a, d3b, d2a, d2b, d1a, d1b);
	// create string for the immediate value being written to register
	n = sprintf(buffer1, "%d%d%d%d%d%d%d%d", d1a, d1b, d2a, d2b, d3a, d3b, d4a, d4b);
	n = sprintf(buffer2, "%d%d%s", intup, intlow, buffer1);
	
	switch (lower){
		case 0x0:
		{
			printf("%08x: %-14s%-8s  %s\n", currAddr, buffer2, "jmp", bufferrev);
			n=snprintf(buff, 256,"%08x: %-14s%-8s  %s\n", currAddr, buffer2, "jmp", bufferrev);
			res = fileno(f);
			write(res, buff, n );
			break;
		}
		case 0x1:
		{
			printf("%08x: %-14s%-8s  %s\n", currAddr, buffer2, "jle", bufferrev);
			n=snprintf(buff, 256,"%08x: %-14s%-8s  %s\n", currAddr, buffer2, "jle", bufferrev);
			res = fileno(f);
			write(res, buff, n );
			break;		
		}
		case 0x2:
		{
			printf("%08x: %-14s%-8s  %s\n", currAddr, buffer2, "jl", bufferrev);
			n=snprintf(buff, 256,"%08x: %-14s%-8s  %s\n", currAddr, buffer2, "jl", bufferrev);
			res = fileno(f);
			write(res, buff, n );
			break;
		}
		case 0x3:
		{
			printf("%08x: %-14s%-8s  %s\n", currAddr, buffer2, "je", bufferrev);
			n=snprintf(buff, 256,"%08x: %-14s%-8s  %s\n", currAddr, buffer2, "je", bufferrev);
			res = fileno(f);
			write(res, buff, n );
			break;
		}
		case 0x4:
		{
			printf("%08x: %-14s%-8s  %s\n", currAddr, buffer2, "jn", bufferrev);
			n=snprintf(buff, 256,"%08x: %-14s%-8s  %s\n", currAddr, buffer2, "jn", bufferrev);
			res = fileno(f);
			write(res, buff, n );
			break;
		}
		case 0x5:
		{
			printf("%08x: %-14s%-8s  %s\n", currAddr, buffer2, "jge", bufferrev);
			n=snprintf(buff, 256,"%08x: %-14s%-8s  %s\n", currAddr, buffer2, "jge", bufferrev);
			res = fileno(f);
			write(res, buff, n );
			break;
		}
		case 0x6:
		{
			printf("%08x: %-14s%-8s  %s\n", currAddr, buffer2, "jg", bufferrev);
			n=snprintf(buff, 256,"%08x: %-14s%-8s  %s\n", currAddr, buffer2, "jg", bufferrev);
			res = fileno(f);
			write(res, buff, n );
			break;
		}
		default:
		{
			// Upper nibble is invalid.
			printf("# Invalid Instruction Encountered...\n");
			break;	
		}
		
	}
	return 1;
}
	
int	call_instruct(char upper, char lower, char d1, char d2, char d3, char d4){
	
	char d1a = d1 >> 4;
	char d1b = d1 & 15;
	
	char d2a = d2 >> 4;
	char d2b = d2 & 15;
	
	char d3a = d3 >> 4;
	char d3b = d3 & 15;
	
	char d4a = d4 >> 4;
	char d4b = d4 & 15;
	
	// convert bytes to integer
	char buffer1[255];
	char bufferrev[255];
	char buffer2[255];	
	int intup  = upper;
	int intlow = lower;
	int n;
	
	n = sprintf(bufferrev, "%d%d%d%d%d%d%d%d", d4a, d4b, d3a, d3b, d2a, d2b, d1a, d1b);
	// create string for the immediate value being written to register
	n = sprintf(buffer1, "%d%d%d%d%d%d%d%d", d1a, d1b, d2a, d2b, d3a, d3b, d4a, d4b);
	n = sprintf(buffer2, "%d%d%s", intup, intlow, buffer1);
	
	if (n>0){
		printf("%08x: %-14s%-8s  %s, %s(%s)\n", currAddr, buffer2, "call", bufferrev);
		n=snprintf(buff, 256, "%08x: %-14s%-8s  %s, %s(%s)\n", currAddr, buffer2, "call", bufferrev);
		res = fileno(f);
		write(res, buff, n );
	} else return -1;
	
	return 1;		
}
	
int	return_instruct(char upper, char lower){
	// convert bytes to integer representation
	int intup = (int) upper;
	int intlow = (int) lower;
	int n;
	char buffer[12];	
	n = sprintf(buffer, "%d%d", intup, intlow);
	printf("%08x: %-14s%-8s \n", currAddr, buffer, "ret");
	n=snprintf(buff, 256, "%08x: %-14s%-8s \n", currAddr, buffer, "ret");
	res = fileno(f);
	write(res, buff, n );	
	return 1;	
}
	
int	pushl_instruct(char upper, char lower, char Rs){
	char rA = Rs >> 4;
	char rB = Rs & 15;
	
	// convert bytes to integer
	int n;	
	int r1 = rA;
	int r2 = rB;	
	char buffer[255];	
	int intup  = upper;
	int intlow = lower;

	// get the string representation of the registers
	char** regs = getRegister(r1, 8, 2, 4);
	// get the string representation of first rA
	char* regA = regs[0];
	// get the string representation of first rB
	char* regB = regs[1];
	if ((!strcmp(regA, "err")==0) && (!strcmp(regB, "err")==0)){
		n = sprintf(buffer, "%d%d%d%c", intup, intlow, r1, 'F');
		if (n>0){
			printf("%08x: %-14s%-8s %s\n", currAddr, buffer, "pushl", regA);
			n=snprintf(buff, 256, "%08x: %-14s%-8s %s\n", currAddr, buffer, "pushl", regA);
			res = fileno(f);
			write(res, buff, n );	
		} else return -1;
	} else {
		// One of the registers was invalid.
		printf("# Invalid Instruction Encountered...\n");
	}	
	
	return 1;	
}
	
int	popl_instruct(char upper, char lower, char Rs){
	char rA = Rs >> 4;
	char rB = Rs & 15;
	
	// convert bytes to integer
	int n;	
	int r1 = rA;
	int r2 = rB;	
	char buffer[255];	
	int intup  = upper;
	int intlow = lower;

	// get the string representation of the registers
	char** regs = getRegister(r1, 8, 2, 4);
	// get the string representation of first rA
	char* regA = regs[0];
	// get the string representation of first rB
	char* regB = regs[1];
	if ((!strcmp(regA, "err")==0) && (!strcmp(regB, "err")==0)){
		n = sprintf(buffer, "%d%d%d%c", intup, intlow, r1, 'F');
		if (n>0){
			printf("%08x: %-14s%-8s %s\n", currAddr, buffer, "popl", regA);
			n=snprintf(buff, 256, "%08x: %-14s%-8s %s\n", currAddr, buffer, "popl", regA);
			res = fileno(f);
			write(res, buff, n );	
		} else return -1;
	} else {
		// One of the registers was invalid.
		printf("# Invalid Instruction Encountered...\n");	
	}
	
	return 1;	
}

char** getRegister(int rA, int rB, int firstDimension, int secondDimension) {

	int i;
	char** str;
	
	str = malloc(firstDimension*sizeof(char*));	
	for(i=0;i<2;i++){	
		str[i]=malloc(secondDimension*sizeof(char*));
	}

	switch(rA){
		
		case 0:
			strcpy(str[0],"%eax");
			break;
		case 1:
			strcpy(str[0],"%ecx");
			break;
		case 2:
			strcpy(str[0],"%edx");
			break;
		case 3:
			strcpy(str[0],"%ebx");
			break;
		case 4:
			strcpy(str[0],"%esp");
			break;
		case 5:
			strcpy(str[0],"%ebp");
			break;
		case 6:
			strcpy(str[0],"%esi");
			break;
		case 7:
			strcpy(str[0],"%edi");
		case 8:
			strcpy(str[1],"F");
			break;	
		default:
			strcpy(str[1], "err");
			break;
	}
	
	switch(rB){
		
		case 0:
			strcpy(str[1],"%eax");
			break;
		case 1:
			strcpy(str[1],"%ecx");
			break;
		case 2:
			strcpy(str[1],"%edx");
			break;
		case 3:
			strcpy(str[1],"%ebx");
			break;
		case 4:
			strcpy(str[1],"%esp");
			break;
		case 5:
			strcpy(str[1],"%ebp");
			break;
		case 6:
			strcpy(str[1],"%esi");
			break;
		case 7:
			strcpy(str[1],"%edi");
			break;
		case 8:
			strcpy(str[1],"F");
			break;
		default:
			strcpy(str[1], "err");
			break;
	}
	return str;
}

