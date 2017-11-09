#include "simulator.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <stdint.h>

using namespace std;

/* Constructor: initialize registers, imemory (store instructions) and dmemory (store data)*/
simulator::simulator(){
    int i=0;
    cycle=0;
    
    for(i=0;i<32;i++){
        Register[i]=0;
    }
    for(i=0;i<1024;i++){
        imemory[i]=0;
        dmemory[i]=0;
    }
}

/* It creates two files: 
 (1) snapshot.rpt: print the contents of 32 registers in each cycle in this file.
 (2) error_dump: print the error in this file.
  and reads in two files:
 (1) dimage.bin: memory that store data.
 (2) iimage.bin: memory that store instruction.
 */
void simulator::readAndCreateFile(){
    snapshot = fopen("snapshot.rpt", "w");
    error = fopen("error_dump.rpt", "w");
    FILE *Dimmage,*Iimage;
    long lSizeD, lSizeI;
    
    size_t resultD, resultI,I,D ;
    int i;
    
    Dimmage = fopen ( "dimage.bin" , "r" );
    Iimage = fopen ( "iimage.bin" , "r" );
    if (Dimmage==NULL) {fputs ("I File error",stderr); exit (1);}
    if (Iimage==NULL) {fputs ("D File error",stderr); exit (1);}
    
    uint8_t lengthD[8]={0},lengthI[8]={0};
   
    fseek (Dimmage , 0 , SEEK_END);
    fseek (Iimage , 0 , SEEK_END);
    lSizeD = ftell (Dimmage);
    lSizeI = ftell (Iimage);
    rewind (Dimmage);
    rewind (Iimage);
    
    fread (lengthD,1,8,Dimmage);
    fread (lengthI,1,8,Iimage);
    lSizeD=0;
    lSizeI=0;
    for(i=4;i<8;i++){
        lSizeD=lSizeD*256+lengthD[i];
        lSizeI=lSizeI*256+lengthI[i];
        
    }
    
    resultD = fread (this->dmemory,1,lSizeD*4,Dimmage);
    resultI = fread (this->imemory,1,lSizeI*4,Iimage);
    
    Register[29]=lengthD[0]*256*256*256+lengthD[1]*256*256+lengthD[2]*256+lengthD[3];
    PC=lengthI[0]*256*256*256+lengthI[1]*256*256+lengthI[2]*256+lengthI[3];
    PC_start=PC;

    fclose (Dimmage);
    fclose (Iimage);
    
}

/* Decode the machine codes and do the operation */
void simulator::opCategory(int index){
    int i;
    int16_t half;
    int8_t byte;
    unsigned int maCo=0;
    bool error=false;
    cycle++;
    
    PC=PC+4;
    
    if(index<0||index>1024){
        return;
    }
    for(i=0;i<4;i++){
        maCo=imemory[index+i]+maCo*256;
    }
    
    /* Decode the machine code*/
    unsigned int opCode=maCo>>26;
    unsigned int funct = (maCo<<26)>>26;
    unsigned int rs = (maCo<<6)>>27;
    unsigned int rt = (maCo<<11)>>27;
    unsigned int rd = (maCo<<16)>>27;
    unsigned int shamt = (maCo<<21)>>27;
    unsigned short int imm = maCo & 0x03FFFFFF;
    unsigned int add=(maCo<<6)>>6;
    
    int ptr=(Register[rs]+(short)imm);
    
    PC_last=PC;
    
    /* detect error*/
    if(opCode==0X00||opCode==0X08||opCode==0X09||opCode==0X23||opCode==0X21||opCode==0X25||opCode==0X20||opCode==0X24||opCode==0X0F||opCode==0x0C||opCode==0x0D||opCode==0x0E||opCode==0x0A){
        
        if(((opCode==0X00)&&(funct!=0X00||rt!=0||shamt!=0)&&(funct!=0X08))){
            
            if(writeReg0(rd)==true){
                error=true;
            }
        }
        else if(opCode!=0X00){
            if(writeReg0(rt)==true){
                error=true;
            }
        }
    }
    if(opCode==0X00||opCode==0X08||opCode==0X23||opCode==0X21||opCode==0X25||opCode==0X20||opCode==0X24||opCode==0X2B||opCode==0X29||opCode==0X28||opCode==0X04||opCode==0X05){
        if(opCode==0X04||opCode==0X05){
            
            numOverflow( PC,4*(short)imm);
        }
        else if(opCode==0X00){
            
            if(funct==0X20){
                
                numOverflow(Register[rs],Register[rt]);
                
            }
            else if(funct==0X22){
                numOverflow(Register[rs],-Register[rt]);
                
            }
        }
        else{
            
            numOverflow(Register[rs],(short)imm);
            
        }
    }
    if(opCode==0X23||opCode==0X21||opCode==0X25||opCode==0X20||opCode==0X24||opCode==0X2B||opCode==0X29||opCode==0X28){
        if(opCode==0X23||opCode==0X2B){
            if( addOverflow(ptr+3)){
                END=true;
                error=true;
            }
        }
        else if(opCode==0X21||opCode==0X25||opCode==0X29){
            if( addOverflow(ptr+1)){
                END=true;
                error=true;
            }
        }
        else{
            if( addOverflow(ptr)){
                END=true;
                error=true;
            }
            
        }
        
    }
    if(opCode==0X23||opCode==0X21||opCode==0X25||opCode==0X2B||opCode==0X29){
        
        if(opCode==0X23||opCode==0X2B){
            
            if(misAlign( ptr, 4)){
                
                END=true;
                error=true;
            }
        }
        else{
            
            
            if(misAlign( ptr, 2)){
                END=true;
                error=true;
            }
            
        }
    }
    
    /* if threre is no error, do the operation */
    if(error==false){
        
        if(opCode==0X00){
            
            if(funct==0X20){//add
                Register[rd]=Register[rs]+Register[rt];
                
            }
            else if(funct==0X21){//addu
                Register[rd]=Register[rs]+(unsigned int)Register[rt];
                
            }
            else if(funct==0X22){//sub
                Register[rd]=Register[rs]-Register[rt];
                
            }
            else if(funct==0X24){//and
                Register[rd]=Register[rs]&Register[rt];
                
            }
            else if(funct==0X25){//or
                Register[rd]=Register[rs]|Register[rt];
                
            }
            else if(funct==0X26){//xor
                Register[rd]=Register[rs]^Register[rt];
                
            }
            else if(funct==0X27){//nor
                Register[rd]=~(Register[rs]|Register[rt]);
                
            }
            else if(funct==0X28){//nand
                Register[rd]=~(Register[rs]&Register[rt]);
                
            }
            else if(funct==0X2A){//slt
                Register[rd]=(Register[rs]<Register[rt]);
                
            }
            else if(funct==0X00){//sll
                Register[rd]=(unsigned int)Register[rt]<<shamt;
                
            }
            else if(funct==0X02){//srl
                Register[rd]=(unsigned int)Register[rt]>>shamt;
                
            }
            else if(funct==0X03){//sra
                
                Register[rd]=Register[rt]>>shamt;
            }
            else if(funct==0X08){//jr
                
                PC=Register[rs];
                return;
            }
            
            
        }
        else if(opCode==0X08){//addi
            
            
            Register[rt]=Register[rs]+(short)imm;
        }
        else if(opCode==0X09){//addiu
            
            Register[rt]=Register[rs]+imm;
        }
        else if(opCode==0X23){//lw
            if(misAlign( ptr, 4)){
                END=true;
                return;
            }
            
            int a=1;
            
            Register[rt]=0;
            
            for(i=0;i<4;i++){
                Register[rt]=Register[rt]*256+dmemory[ptr+i];
                a=a*256;
            }
            
        }
        else if(opCode==0X21){//lh
            
            if(misAlign( ptr, 2)){
                END=true;
                return;
            }
            
            half=dmemory[ptr]*256+dmemory[ptr+1];
            Register[rt]=half;
            
            
        }
        else if(opCode==0X25){//lhu
            
            if(misAlign( ptr, 2)){
                END=true;
                return;
            }
            
            Register[rt]=dmemory[ptr]*256+dmemory[ptr+1];
            
        }
        else if(opCode==0X20){//lb
            
            byte=dmemory[ptr];
            Register[rt]=byte;
            
        }
        else if(opCode==0X24){//lbu
            
            Register[rt]=dmemory[ptr];
        }
        else if(opCode==0X2B){//sw
            
            if(misAlign( ptr, 4)){
                END=true;
                return;
            }
            
            int rtemp=Register[rt];
            
            uint8_t value;
            for(i=3;i>=0;i--){
                value=(unsigned)rtemp%256;
                rtemp=(unsigned)rtemp/256;
                dmemory[ptr+i]=value;
                
            }
            
            
        }
        else if(opCode==0X29){//sh
            
            if(misAlign( ptr, 2)){
                END=true;
                return;
            }
            
            half=(int16_t)Register[rt];
            
            dmemory[ptr]=(uint16_t)half/256;
            dmemory[ptr+1]=(uint16_t)half%256;
            
            
        }
        else if(opCode==0X28){//sb
            
            byte=(int8_t)Register[rt];
            
            dmemory[ptr]=byte;
            
        }
        else if(opCode==0X0F){//lui
            
            Register[rt]=(int)imm<<16;
            
        }
        else if(opCode==0X0C){//andi
            
            Register[rt]=Register[rs]&(int)imm;
            
        }
        else if(opCode==0X0D){//ori
            
            Register[rt]=Register[rs]|(int)imm;
        }
        else if(opCode==0X0E){//nori
            
            Register[rt]=~(Register[rs]|(int)imm);
        }
        else if(opCode==0X0A){//slti
            
            Register[rt]=(Register[rs]<(short)imm);
            
        }
        else if(opCode==0X04){//beq
            
            if(Register[rs]==Register[rt]){
                PC=PC+4*(short)imm;
            }
            
            
        }
        else if(opCode==0X05){//bne
            
            if(Register[rs]!=Register[rt]){
                PC=PC+4*(short)imm;
            }
            
            
        }
        else if(opCode==0X07){//bgtz
            
            if(Register[rs]>0){
                PC=PC+4*(short)imm;
            }
            
        }
        else if(opCode==0X02){//j
            
            PC=((PC>>28)<<28)|(add*4);
            
        }
        else if(opCode==0X03){//jal
            
            Register[31]=PC;
            PC=((PC>>28)<<28)|(add*4);
        }
        else if(opCode==0X3F){//halt
            
            END=true;
        }
    }
    return;
    
}
/* check whether the register that is being written is register0 */
bool simulator::writeReg0(int reg)
{
    
    if (reg == 0) {
        
        fprintf( error , "In cycle %d: Write $0 Error\n", cycle);
        return true;
    }else return false;
}
/* check if number overflow occurs */
bool simulator::numOverflow(int a, int b)
{
    
    if(a > 0 && b > 0){
        if((a + b) <= 0){
            fprintf(error , "In cycle %d: Number Overflow\n", cycle);
            return true;
        }
    }else if(a < 0 && b < 0){
        if((a + b) >= 0){
            fprintf(error , "In cycle %d: Number Overflow\n", cycle);
            
            return true;
        }
    }
    return  false;
}

/*check if address overflow occurs*/
bool simulator::addOverflow(int address)
{
    if(address > 1023|| address < 0){
        fprintf(error , "In cycle %d: Address Overflow\n", cycle);
        return true;
    }
    else
    return false;
}
/* check if misalignment error occurs*/
bool simulator::misAlign(int c, int base)
{
    if(c%base != 0){
        
        fprintf(error , "In cycle %d: Misalignment Error\n", cycle);
        return true;
    }
    return false;
}
/* print the content of registers and program counter in snapshot.rpt */
void simulator::output(){
    
    fprintf(snapshot, "cycle %d\n", cycle);
    for(int i = 0; i < 32; i++){
        fprintf(snapshot, "$%02d: 0x%08X\n", i, Register[i]);
    }
    fprintf(snapshot, "PC: 0x%08X\n\n\n", PC);
    
}


