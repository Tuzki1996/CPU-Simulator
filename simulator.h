#ifndef SIMULATOR_H_INCLUDED
#define SIMULATOR_H_INCLUDED
#include <stdint.h>
#include <stdio.h>

class simulator{
public:

    simulator();
    void readAndCreateFile();
    void opCategory(int index);
    void output( );

    bool END;
    int PC;
    int PC_start;
    int PC_last;
    int cycle;
    FILE *snapshot, *error;

private:

    bool writeReg0(int regNum);
    bool numOverflow(int a, int b);
    bool addOverflow(int address);
    bool misAlign(int c, int base);
    uint8_t imemory[1032];
    uint8_t dmemory[1032];
    int Register[32];


};


#endif // SIMULATOR_H_INCLUDED
