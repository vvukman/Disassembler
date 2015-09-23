.pos 0x100

main:	irmovl bottom,  %esp     # initialize stack
	irmovl a,       %edi     # address of the first element of a
	irmovl alen,    %esi	  
	mrmovl (%esi),  %esi     # number of elements of a
	irmovl $0x1,    %eax
	subl   %eax,    %esi     # last index in a
   # ready to call findmax: a --> edi, last --> esi
	call findmax
	halt


.pos 0x150

#
# Find position of the maximum element in an array.
#
findmax:
	.long 0x818181
	# PUT HERE 
	# YOUR CODE FOR FIND-MAX
	# TO TEST IT

	
#
# Array to sort
#
.pos 0x1540
a:	.long 30
      .long 9
      .long 21
      .long 13
      .long 6
	.long 26
	.long 35
	.long 32
	.long 15
	.long 17
alen:	.long 10

	#
# Stack (256 thirty-two bit words is more than enough here).
#
.pos 0x3000
top:	            .long 0x00000000, 0x100    # top of stack.
bottom:           .long 0x00000000          # bottom of stack.

