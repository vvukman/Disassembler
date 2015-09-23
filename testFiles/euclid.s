.pos 0x4
main:            xorl   %edx, %edx        # %edx = 0
                 mrmovl a(%edx), %eax     # %eax = a
                 mrmovl b(%edx), %ebx     # %ebx = b
loop:            subl   %edx, %ebx        # is b <= 0?
                 jle    end               # if so, done
                 modl   %ebx, %eax        # now %eax contains r
                 rrmovl %eax, %ecx        # save r
                 rrmovl %ebx, %eax        # a = b
                 rrmovl %ecx, %ebx        # b = r
                 jmp    loop              
end:             rmmovl %eax, gcd(%edx)   # gcd = a
                 rmmovl %eax, a(%edx)     # update a
                 rmmovl %ebx, b(%edx)     # update b
                 halt                     
.pos 0x1000
a:               .long 0x78a              
b:               .long 0x46b              
gcd:             .long 0x0                
