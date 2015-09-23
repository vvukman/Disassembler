
#include <stdio.h>
#include <unistd.h>
#include "printRoutines.h"

// You probably want to create a number of printing routines in this file.
// Put the prototypes in printRoutines.h

 
  /*********************************************************************
   Details on print formatting can be found by reading the man page for
   printf. The formatting rules for the disassembler output are given below.
   Basically it should be the case that if you take the ouput from your 
   disassembler, remove the initial address information, and then take
   the resulting file and load it into the simulator the object code 
   produced be the simulator should match what your program read. 
   (You may have to use a .pos directive to indicate the offset to 
    where the code starts.) If the simulator reports an assembler error 
    then you need to fix the output of your dissassembler so that it
    is acceptable to the simulator.) 

   The printing rules are as follows:
   
     1) Each line is to begin with the hex printed value of the address followed 
        immediately by a ": ". Leading zeros are to be printed for the address 
        which is 4 bytes long. 

     2) After the ": " there are to be 14 characters before the start of the 
        printing of the instruction mnemonic. In that 14 characters 
        the program is print, left justified, the hex representation of the 
        memory values corresponding to the assembler instruction and operands
        that were printed. 

     3) The instruction field is 8 characters. The instruction is to be 
        printed left justified in those 8 characters (%-8s). All instructions
        are to be printed in lower case. 

     4) After the instruction field the first operand is to be printed. 
 
     5) If there is a second operand then there is to be a comma immediately 
        after the first operand (no spaces) and then a single space followed
        by the second operand. 

     6) The rules for printing operands are as follows: 

         a) Registers:  A register is to be printed with the % sign and then
            its name. (.e.g. %esp, %eax etc) Register names are printed in lower case.
 
         b) All numbers are to be printed in hex with the appropriate leading 0x. Leading 
            zeros are to be suppressed.

         c) A base displacement address is to be printed as D(reg) where the 
            printing of D follows the rules in (b), and the printing of reg follows
            the rules in (a). Note there are no spaces between the D and "(" or 
            between reg and the "(" or ")".
        
         d) An address such as that used by a call or jump is to be printed as in (b).

         e) A constant, such as used by irmovl is to be printed as a number in (b) but 
            with a "$" immediately preceeding the 0x without any spaces. 
           
     7) The unconditional move instruction is to be printe as rrmovl. 

     8) The mnemonics for the instruction are to conform to those described in the 
        text and accepted by the simulator. 

     9) If there are any differences between what is accepted by the simulator and 
        what is described in these comments or the text, the differences will be 
        resolved in favour of the simulator.

    10) The arguments for the format string in the example printing are 
        strictly for illustrating the spacing. You are free to construct
        the output however you want.
 
  ********************************************************************************/
 
#define BUFFSIZE 256

/* This is a function to demonstrate how to do print formatting. It takes the file
 * descriptor the I/O is to be done to. 
 *
 * The funtion returns PRINTSUCCESS if there were no write problems, and PRINTERROR otherwise
 * 
 */
int samplePrint(int fd) {
  char buff[BUFFSIZE];
  int bytesWritten;
  int res;

  unsigned int addr = 0x1016;
  unsigned int offset = 3484;
  char * r1 = "%eax";
  char * r2 = "%edx";
  char * inst1 = "rrmovl";
  char * inst2 = "jne";
  char * inst3 = "irmovl";
  char * inst4 = "mrmovl";
  unsigned int destAddr = 8193;
  

  // NOTE: technically there is no requirement that the system write the number of bytes
  // requested. It could be less than that in which case an additional write(s) would be required.
  // It is probably a better idea to open the output file as a stream and then 
  // use stream I/O see the man pages for fopen() and fprintf()

  bytesWritten = snprintf(buff, BUFFSIZE, "%08x: %-14s%-8s  %s, %s\n", 
			  addr, "2002", inst1, r1, r2);
  res = write(fd, buff, bytesWritten);

  if (bytesWritten != res) return PRINTERROR;

  addr += 2;
  bytesWritten = snprintf(buff, BUFFSIZE, "%08x: %-14s%-8s  0x%x\n", 
			  addr, "7401200000", inst2, destAddr);
  res = write(fd, buff, bytesWritten);

  if (bytesWritten != res) return PRINTERROR;

  addr += 5;

  bytesWritten = snprintf(buff, BUFFSIZE, "%08x: %-14s%-8s  $0x%x, %s\n", 
			  addr, "30F210000000", inst3, 16, r2);
  res = write(fd, buff, bytesWritten);

  if (bytesWritten != res) return PRINTERROR;

  addr += 6;

  bytesWritten =  snprintf(buff, BUFFSIZE, "%08x: %-14s%-8s  0x%x(%s), %s\n", 
			   addr, "500200000100", inst4, 65536, r2, r1); 
  res = write(fd, buff, bytesWritten);
  if (bytesWritten != res) return PRINTERROR;
  
  addr += 6;

  bytesWritten = snprintf(buff, BUFFSIZE, "%08x: %-14s%-8s  %s, %s\n", 
			  addr, "2020", inst1, r2, r1);
  res = write(fd, buff, bytesWritten);

  if (bytesWritten != res) return PRINTERROR;


  return PRINTSUCCESS;
}  
  
