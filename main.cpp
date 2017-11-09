#include "simulator.h"

#include <iostream>
using namespace std;


int main(){
    int i;

    simulator b;

    b.readAndCreateFile();
    
    while(b.END==false&&b.cycle<=500000){
      
       b.output();
       b.opCategory((b.PC-b.PC_start));


    }
    fclose(b.snapshot);

return 0;

}
