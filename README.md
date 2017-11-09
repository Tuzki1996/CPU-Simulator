# CPU-Simulator
This project is a CPU simulator for the reduced MIPS R3000 ISA.

To compile and run the program please go to the directory and type the following lines in your terminal.

    make
    ./single_cycle

It will generate two output files.


Input files:
(1)dimage.bin : data
(2)iimage.bin : machine code instructions
For more details on the format of input files and instructions in MIPS please refer to Input_Sample.pdf and Reduced_MIPS_R3000_ISA.pdf

Output files:
(1)snapshot.rpt : print the contents of registers and prgram counter of each cycle.
(2)error_dump.rpt: print errors

This program reads instructions and data from the inpute files. In each cycle, it execute one instruction and print the contents of registers in snapshot.rpt after executing it. 