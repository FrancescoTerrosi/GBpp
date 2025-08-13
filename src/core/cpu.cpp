#include <iostream>
#include <fstream>
#include <cstring>
#include "cpu.h"
#include "mmu.h"
#include "../interfaces/mmumemoryinterface.h"

//FOR STATISTICS
int PROGRAM_SIZE, INSTRUCTION_COUNTER, NOP_COUNTER, INSTRUCTION_FAILS, UNIQUE_INST, UNIQUE_FAILS;
bool* seen = new bool[255];
bool* seen_fail = new bool[255];

float CLOCK_FREQUENCY = 4.194304; //Mhz

int REGISTER_FILE_SIZE = 14;

unsigned short PC = 0;

unsigned char* INSTRUCTION_REGISTER = new unsigned char;

unsigned short* BUS = new unsigned short;

unsigned char* DATA_BUS = new unsigned char;

unsigned char* REGISTER_FILE = new unsigned char[REGISTER_FILE_SIZE];
int A_REGISTER = 0x07;
int B_REGISTER = 0x00;
int C_REGISTER = 0x01;
int D_REGISTER = 0x02;
int E_REGISTER = 0x03;
int H_REGISTER = 0x04;
int L_REGISTER = 0x05;

//1 2 3 4 5 6 7 8
//Z N H C X X X X
int F_REGISTER = 0x06;
int SP_HI_REGISTER = 0x0A;
int SP_LO_REGISTER = 0x0B;
int PC_HI_REGISTER = 0x0C;
int PC_LO_REGISTER = 0x0D;

std::string parseBytes(unsigned char* bytes, int bytes_length)
{

    unsigned char bytes_copy[bytes_length];
    memcpy(&bytes_copy, bytes, bytes_length);

    char current_byte[8];
    int counter = 0;

    std::string result = "";
    
    for (int i = 0; i < bytes_length; i++)
    {
        counter = 7;
        while (bytes_copy[i] > 0)
        {
            if (bytes_copy[i]%2 == 1)
            {
                current_byte[counter] = '1';
            }
            else
            {
                current_byte[counter] = '0';
            }
            bytes_copy[i] = (bytes_copy[i] >> 1);
            counter--;
        }

        while (counter >= 0)
        {
            current_byte[counter] = '0';
            counter--;
        }

        result += current_byte;

        if (i != bytes_length-1) {
            result += " ";
        }

    }

    return result;

}

void updateMetrics(unsigned char high_opcode, unsigned char mid_opcode, unsigned char low_opcode)
{
    if (high_opcode == 0 && mid_opcode == 0 && low_opcode == 0)
    {
        NOP_COUNTER++;
    }
    if (!seen[((high_opcode << 8)| (mid_opcode << 3) | low_opcode)])
    {
        UNIQUE_INST++;
    }
    else
    {
        seen[((high_opcode << 8)| (mid_opcode << 3) | low_opcode)] = true;
    }
    INSTRUCTION_COUNTER++;
    
}

void updateFailMetrics(unsigned char high_opcode, unsigned char mid_opcode, unsigned char low_opcode)
{
    if (high_opcode != 0 | mid_opcode != 0 | low_opcode != 0) {
        if (!seen_fail[((high_opcode << 8) | (mid_opcode << 3) | low_opcode)])
        {
            UNIQUE_FAILS++;
        }
        else
        {
            seen_fail[((high_opcode << 8)| (mid_opcode << 3) | low_opcode)] = true;
        }
        INSTRUCTION_FAILS++;
        //std::cout << "Error! Instruction not recognized" << std::endl << std::flush;
    }
}

/*

    mode: 0 ADD
          1 SUB

    FLAG REGISTER
    
    7  6  5  4  3  2  1  0
    Z  N  H  CY X  X  X  X

    set Z to 1 if result=0, else set Z to 0
    set N to 1 if is SUB, else set Z to 0
    set H to 1 if carry bit 3
    set CY to 1 if carry byt 7

*/
void doALUOp(unsigned char* dst, unsigned char operand1, unsigned char operand2, unsigned char bitmask, unsigned char mode)
{

    unsigned char check_bit_3 = (REGISTER_FILE[A_REGISTER] >> 3)%2;
    unsigned char check_bit_7 = (REGISTER_FILE[A_REGISTER] >> 7)%2;

    //ADD
    if (mode == 0)
    {
        REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] & 0xBF);

        //SET FLAGS
        unsigned char check_bit_3 = (REGISTER_FILE[A_REGISTER] >> 3)%2;
        unsigned char check_bit_7 = (REGISTER_FILE[A_REGISTER] >> 7)%2;

        *dst = operand1 + operand2;

        if (*dst == 0)
        {
            REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] | 0x80);
        }
        else
        {
            REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] & 0x7F);
        }
        //H BIT = 4
        if (check_bit_3 == 1 && ((*dst >> 3)%2 == 0))
        {
            REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] | 0x20);
        }
        else
        {
            REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] & 0xDF);
        }

        //CY BIT = 0
        if (check_bit_7 == 1 && (*dst >> 7)%2 == 0)
        {
            REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] | 0x10);
        }
        else
        {
            REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] & 0xEF);
        }
    }
    //SUB
    else if (mode == 1)
    {
        REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] | 0x40);

        *dst = operand1 - operand2;

        //Z BIT
        if (*dst == 0)
        {
            REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] | 0x80);
        }
        else
        {
            REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] & 0x7F);
        }

        //H BIT
        if (check_bit_3 == 0 && (*dst >> 3)%2 == 1)
        {
            REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] | 0x20);
        }
        else
        {
            REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] & 0xDF);
        }

        //CY BIT = 0
        if (check_bit_7 == 0 && (*dst >> 7)%2 == 1)
        {
            REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] | 0x10);
        }
        else
        {
            REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] & 0xEF);
        }
    }
    else
    {
        //std::cout << "This should never happen." << std::endl;
    }

    REGISTER_FILE[F_REGISTER] = REGISTER_FILE[F_REGISTER] & bitmask;

}

void fetch()
{
    //Mode 2 is fetch
    //#TODO better handling of this behaviour
    dispatchMemOp(PC, INSTRUCTION_REGISTER, 0);

    std::cout << std::hex << "$" << PC << ": " << "0x" << (unsigned short)*INSTRUCTION_REGISTER << std::dec << " | 0b" << parseBytes(INSTRUCTION_REGISTER, 1) << std::endl;
}

void execute()
{

    unsigned char instruction = *INSTRUCTION_REGISTER;
    unsigned short data = (unsigned short)*BUS;

    unsigned char high_opcode = (unsigned char)((instruction & 0xC0) >> 6);
    unsigned char mid_opcode = (unsigned char)((instruction & 0b111000) >> 3);
    unsigned char low_opcode = (unsigned char)(instruction & 0b111);
    unsigned short SP_FULL_ADDRESS = ((REGISTER_FILE[SP_HI_REGISTER] << 8) | REGISTER_FILE[SP_LO_REGISTER]);
    unsigned short HL_FULL_ADDRESS = ((REGISTER_FILE[H_REGISTER] << 8) | REGISTER_FILE[L_REGISTER]);

    //11
    if (high_opcode == 0x03)
    {
        if (low_opcode == 0x05 && mid_opcode%2 == 0)
        {
            //PUSH rr
            unsigned char select_reg = (mid_opcode >> 1);
            SP_FULL_ADDRESS--;
            switch (select_reg)
            {
                case 0:
                    dispatchMemOp(SP_FULL_ADDRESS, &(REGISTER_FILE[B_REGISTER]), 1);
                    SP_FULL_ADDRESS--;
                    dispatchMemOp(SP_FULL_ADDRESS, &(REGISTER_FILE[C_REGISTER]), 1);
                break;
                case 1:
                    dispatchMemOp(SP_FULL_ADDRESS, &(REGISTER_FILE[D_REGISTER]), 1);
                    SP_FULL_ADDRESS--;
                    dispatchMemOp(SP_FULL_ADDRESS, &(REGISTER_FILE[E_REGISTER]), 1);
                break;

                case 2:
                    dispatchMemOp(SP_FULL_ADDRESS, &(REGISTER_FILE[H_REGISTER]), 1);
                    SP_FULL_ADDRESS--;
                    dispatchMemOp(SP_FULL_ADDRESS, &(REGISTER_FILE[L_REGISTER]), 1);
                break;

                case 3:
                    dispatchMemOp(SP_FULL_ADDRESS, &(REGISTER_FILE[A_REGISTER]), 1);
                    SP_FULL_ADDRESS--;
                    dispatchMemOp(SP_FULL_ADDRESS, &(REGISTER_FILE[F_REGISTER]), 1);
                break;
            }
            REGISTER_FILE[SP_HI_REGISTER] = (unsigned char)(SP_FULL_ADDRESS >> 8);
            REGISTER_FILE[SP_LO_REGISTER] = (unsigned char)(SP_FULL_ADDRESS);
        }
        else if (low_opcode == 0x01 && mid_opcode%2 == 0)
        {
            //POP rr
            unsigned char select_reg = (mid_opcode >> 1);
            switch (select_reg)
            {
                case 0:
                    dispatchMemOp(SP_FULL_ADDRESS, &(REGISTER_FILE[C_REGISTER]), 0);
                    SP_FULL_ADDRESS++;
                    dispatchMemOp(SP_FULL_ADDRESS, &(REGISTER_FILE[B_REGISTER]), 0);
                break;
                case 1:
                    dispatchMemOp(SP_FULL_ADDRESS, &(REGISTER_FILE[E_REGISTER]), 0);
                    SP_FULL_ADDRESS++;
                    dispatchMemOp(SP_FULL_ADDRESS, &(REGISTER_FILE[D_REGISTER]), 0);
                break;

                case 2:
                    dispatchMemOp(SP_FULL_ADDRESS, &(REGISTER_FILE[L_REGISTER]), 0);
                    SP_FULL_ADDRESS++;
                    dispatchMemOp(SP_FULL_ADDRESS, &(REGISTER_FILE[H_REGISTER]), 0);
                break;

                case 3:
                    dispatchMemOp(SP_FULL_ADDRESS, &(REGISTER_FILE[F_REGISTER]), 0);
                    SP_FULL_ADDRESS++;
                    dispatchMemOp(SP_FULL_ADDRESS, &(REGISTER_FILE[A_REGISTER]), 0);
                break;
            }
            SP_FULL_ADDRESS++;
            REGISTER_FILE[SP_HI_REGISTER] = (unsigned char)(SP_FULL_ADDRESS >> 8);
            REGISTER_FILE[SP_LO_REGISTER] = (unsigned char)(SP_FULL_ADDRESS);
        }
        else if (low_opcode == 0x01 && mid_opcode == 0x01)
        {
            //RET
            dispatchMemOp(SP_FULL_ADDRESS, DATA_BUS, 0);
            SP_FULL_ADDRESS++;
            data = *DATA_BUS;
            dispatchMemOp(SP_FULL_ADDRESS, DATA_BUS, 0);
            SP_FULL_ADDRESS++;
            data = (data | (*DATA_BUS << 8));
            PC = data;
            REGISTER_FILE[SP_HI_REGISTER] = (unsigned char)(SP_FULL_ADDRESS >> 8);
            REGISTER_FILE[SP_LO_REGISTER] = (unsigned char)(SP_FULL_ADDRESS);
        }
        else if ((low_opcode == 0) && ((mid_opcode >> 2) == 0))
        {
            //RET cc
            unsigned char cc = mid_opcode%4;
            switch (cc)
            {
                //NZ
                case 0:
                    if ((REGISTER_FILE[F_REGISTER] >> 7) == 0)
                    {
                        //Load lsb
                        dispatchMemOp(SP_FULL_ADDRESS, DATA_BUS, 0);
                        SP_FULL_ADDRESS++;
                        data = *DATA_BUS;

                        //Load msb
                        dispatchMemOp(SP_FULL_ADDRESS, DATA_BUS, 0);
                        SP_FULL_ADDRESS++;
                        data = (data | (*DATA_BUS << 8));
                        PC = data;
                    }
                break;
                //Z
                case 1:
                    if ((REGISTER_FILE[F_REGISTER] >> 7) == 1)
                    {
                        //Load lsb
                        dispatchMemOp(SP_FULL_ADDRESS, DATA_BUS, 0);
                        SP_FULL_ADDRESS++;
                        data = *DATA_BUS;

                        //Load msb
                        dispatchMemOp(SP_FULL_ADDRESS, DATA_BUS, 0);
                        SP_FULL_ADDRESS++;
                        data = (data | (*DATA_BUS << 8));
                        PC = data;
                    }
                break;
                //NC
                case 2:
                    if ((REGISTER_FILE[F_REGISTER] >> 4)%2 == 0)
                    {
                        //Load lsb
                        dispatchMemOp(SP_FULL_ADDRESS, DATA_BUS, 0);
                        SP_FULL_ADDRESS++;
                        data = *DATA_BUS;

                        //Load msb
                        dispatchMemOp(SP_FULL_ADDRESS, DATA_BUS, 0);
                        SP_FULL_ADDRESS++;
                        data = (data | (*DATA_BUS << 8));
                        PC = data;
                    }
                break;
                //C
                case 3:
                    if ((REGISTER_FILE[F_REGISTER] >> 4)%2 == 1)
                    {
                        //Load lsb
                        dispatchMemOp(SP_FULL_ADDRESS, DATA_BUS, 0);
                        SP_FULL_ADDRESS++;
                        data = *DATA_BUS;

                        //Load msb
                        dispatchMemOp(SP_FULL_ADDRESS, DATA_BUS, 0);
                        SP_FULL_ADDRESS++;
                        data = (data | (*DATA_BUS << 8));
                        PC = data;
                    }
                break;
            }
            REGISTER_FILE[SP_HI_REGISTER] = (unsigned char)(SP_FULL_ADDRESS >> 8);
            REGISTER_FILE[SP_LO_REGISTER] = (unsigned char)(SP_FULL_ADDRESS);
        }
        else if (mid_opcode == 0x07)
        {
            if (low_opcode == 0x06)
            {
                //CP n
                dispatchMemOp(PC+1, DATA_BUS, 0);
                PC++;
                //std::cout << "CP " << std::hex << (unsigned short)*DATA_BUS << " " << (unsigned short)REGISTER_FILE[A_REGISTER] << std::endl;
                doALUOp(DATA_BUS, REGISTER_FILE[A_REGISTER], *DATA_BUS, 0xFF, 1);
                //std::cout << "Result " << std::hex << (unsigned short)*DATA_BUS << std::endl;
                //std::cout << (unsigned short)(REGISTER_FILE[F_REGISTER] >> 7) << std::endl;
            }
            else if (low_opcode == 0x03)
            {
                //EI
                *DATA_BUS = 1;
                dispatchMemOp(0xFFFF, DATA_BUS, 1);
            }
            else if (low_opcode == 0x02)
            {
                //LD A, (nn)
                unsigned short address = 0;
                dispatchMemOp(PC+2, DATA_BUS, 0);
                address = (*DATA_BUS << 8);
                dispatchMemOp(PC+1, DATA_BUS, 0);
                address = (address | *DATA_BUS);
                dispatchMemOp(address, &(REGISTER_FILE[A_REGISTER]), 0);
                PC += 2;
            }
            else if (low_opcode == 0x01)
            {
                //LD SP, HL
                REGISTER_FILE[SP_HI_REGISTER] = REGISTER_FILE[H_REGISTER];
                REGISTER_FILE[SP_LO_REGISTER] = REGISTER_FILE[L_REGISTER];
            }
            else if (low_opcode == 0x00)
            {
                //LD HL, SP+e

                //INIT VARS                
                dispatchMemOp(PC+1, DATA_BUS, 0);
                PC++;

                doALUOp(&REGISTER_FILE[L_REGISTER], REGISTER_FILE[SP_LO_REGISTER], (*DATA_BUS & 0x7F), 0x3F, (*DATA_BUS & 0x80));
                REGISTER_FILE[H_REGISTER] = REGISTER_FILE[SP_HI_REGISTER];
            }
            else
            {
                updateFailMetrics(high_opcode, mid_opcode, low_opcode);
            }
        }
        else if (mid_opcode == 0x06)
        {
            if (low_opcode == 0x00)
            {
                //LDH A, (n)
                dispatchMemOp(PC+1, DATA_BUS, 0);
                PC++;
                dispatchMemOp((0xFF00 | *DATA_BUS), &REGISTER_FILE[A_REGISTER], 0);
                //std::cout << "Loaded value: " << std::hex << (unsigned short)REGISTER_FILE[A_REGISTER] << std::endl;
            }
            else if (low_opcode == 0x02)
            {
                //LDH A, (C)
                dispatchMemOp((0xFF00 | REGISTER_FILE[C_REGISTER]), DATA_BUS, 0);
                REGISTER_FILE[A_REGISTER] = *DATA_BUS;
            }
            else if (low_opcode == 0x06)
            {
                //OR n
                dispatchMemOp(PC+1, DATA_BUS, 0);
                PC++;
                REGISTER_FILE[A_REGISTER] = (REGISTER_FILE[A_REGISTER] | (*DATA_BUS));
                if (REGISTER_FILE[A_REGISTER] == 0)
                {
                    REGISTER_FILE[F_REGISTER] = 0x80;
                }
                else
                {
                    REGISTER_FILE[F_REGISTER] = 0x00;
                }
            }
            else
            {
                updateFailMetrics(high_opcode, mid_opcode, low_opcode);
            }
        }
        else if (mid_opcode == 0x04)
        {
            if (low_opcode == 0x00)
            {
                //LDH (n), A
                dispatchMemOp(PC+1, DATA_BUS, 0);
                PC++;
                dispatchMemOp((0xFF00 | *DATA_BUS), &REGISTER_FILE[A_REGISTER], 1);
            }
            else if (low_opcode == 0x02)
            {
                //LDH (C), A
                dispatchMemOp((0xFF00 | REGISTER_FILE[C_REGISTER]), &(REGISTER_FILE[A_REGISTER]), 1);
            }
            else if (low_opcode == 0x06)
            {
                //AND n
                dispatchMemOp(PC+1, DATA_BUS, 0);
                PC++;
                REGISTER_FILE[A_REGISTER] = (REGISTER_FILE[A_REGISTER] & (*DATA_BUS));
            }
            else
            {
                updateFailMetrics(high_opcode, mid_opcode, low_opcode);
            }
        } 
        else if (mid_opcode == 0x05)
        {
            if (low_opcode == 0x02)
            {
                //LD (nn), A
                unsigned short address = 0;
                dispatchMemOp(PC+2, DATA_BUS, 0);
                address = (*DATA_BUS << 8);
                dispatchMemOp(PC+1, DATA_BUS, 0);
                address = (address | *DATA_BUS);
                dispatchMemOp(address, &REGISTER_FILE[A_REGISTER], 1);
                PC += 2;
            }
            else if (low_opcode == 0x06)
            {
                //XOR n
                dispatchMemOp(PC+1, DATA_BUS, 0);
                PC++;
                REGISTER_FILE[A_REGISTER] = (REGISTER_FILE[A_REGISTER] ^ (*DATA_BUS));
                if (REGISTER_FILE[A_REGISTER] == 0)
                {
                    REGISTER_FILE[F_REGISTER] = 0x80;
                }
                else
                {
                    REGISTER_FILE[F_REGISTER] = 0x00;
                }
            }
            else if (low_opcode == 0x00)
            {
                //ADD SP, e
                dispatchMemOp(PC+1, DATA_BUS, 0);
                PC++;

                doALUOp(&REGISTER_FILE[SP_LO_REGISTER], REGISTER_FILE[SP_LO_REGISTER], (*DATA_BUS & 0x7F), 0x3F, (*DATA_BUS & 0x80));
            }
            else
            {
                updateFailMetrics(high_opcode, mid_opcode, low_opcode);
            }
        }
        else if (mid_opcode == 0x03)
        {
            if (low_opcode == 0x06)
            {
                //SBC n
                dispatchMemOp(PC+1, DATA_BUS, 0);
                PC++;
                doALUOp(&REGISTER_FILE[A_REGISTER], REGISTER_FILE[A_REGISTER], *DATA_BUS, 0xFF, 1);
            }
            else
            {
                updateFailMetrics(high_opcode, mid_opcode, low_opcode);
            }
        }
        else if (mid_opcode == 0x02)
        {
            if (low_opcode == 0x06)
            {
                //SUB n
                dispatchMemOp(PC+1, DATA_BUS, 0);
                PC++;
                doALUOp(&REGISTER_FILE[A_REGISTER], REGISTER_FILE[A_REGISTER], *DATA_BUS, 0xFF, 1);
            }
            else
            {
                updateFailMetrics(high_opcode, mid_opcode, low_opcode);
            }
        }
        else if (mid_opcode == 0x01)
        {
            //CB PREFIX
            if (low_opcode == 0x03)
            {
                dispatchMemOp(PC+1, DATA_BUS, 0);
                PC++;
                high_opcode = (unsigned char)((*DATA_BUS & 0xC0) >> 6);
                mid_opcode = (unsigned char)((*DATA_BUS & 0b111000) >> 3);
                low_opcode = (unsigned char)(*DATA_BUS & 0b111);

                if (high_opcode == 0x01)
                {
                    //BIT b, r
                    if ((REGISTER_FILE[low_opcode] >> (mid_opcode-1))%2 == 0)
                    {
                        REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] | 0xA0);
                    }
                    else
                    {
                        REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] & 0x20);
                    }
                }
                else if (high_opcode == 0x00)
                {
                    if (mid_opcode == 0x02)
                    {
                        //RL r
                        *DATA_BUS = (REGISTER_FILE[low_opcode] >> 7);
                        REGISTER_FILE[low_opcode] = ((REGISTER_FILE[low_opcode] << 1) | ((REGISTER_FILE[F_REGISTER] >> 4)%2));
                        REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] | (*DATA_BUS << 4));
                        REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] & 0x10);
                    }
                    else
                    {
                        updateFailMetrics(high_opcode, mid_opcode, low_opcode);
                    }
                }
                else
                {
                    updateFailMetrics(high_opcode, mid_opcode, low_opcode);
                }
            }
            //END CB PREFIX
            else if (low_opcode == 0x06)
            {
                //ADC n
                dispatchMemOp(PC+1, DATA_BUS, 0);
                PC++;
                doALUOp(&REGISTER_FILE[A_REGISTER], REGISTER_FILE[A_REGISTER], *DATA_BUS + ((REGISTER_FILE[F_REGISTER] >> 4)%2), 0xFF, 0);
            }
            else if (low_opcode == 0x05)
            {
                //CALL nn
                unsigned char pc_reg = 0;
                dispatchMemOp(PC+1, DATA_BUS, 0);
                PC++;
                data = *DATA_BUS;
                dispatchMemOp(PC+1, DATA_BUS, 0);
                PC++;
                data = (data | (*DATA_BUS << 8));
                pc_reg = (PC >> 8);
                SP_FULL_ADDRESS--;
                dispatchMemOp(SP_FULL_ADDRESS, &pc_reg, 1);

                pc_reg = (unsigned char)PC;
                SP_FULL_ADDRESS--;
                dispatchMemOp(SP_FULL_ADDRESS, &pc_reg, 1);

                REGISTER_FILE[SP_HI_REGISTER] = (unsigned char)(SP_FULL_ADDRESS >> 8);
                REGISTER_FILE[SP_LO_REGISTER] = (unsigned char)(SP_FULL_ADDRESS);

                //We increase PC at the end so must decrement
                //#TODO better handling of this behaviour
                PC = data-1;
            }
            else
            {
                updateFailMetrics(high_opcode, mid_opcode, low_opcode);
            }
        }
        else if (mid_opcode == 0x00)
        {
            if (low_opcode == 0x06)
            {
                //ADD n
                dispatchMemOp(PC+1, DATA_BUS, 0);
                PC++;

                doALUOp(&REGISTER_FILE[A_REGISTER], REGISTER_FILE[A_REGISTER], *DATA_BUS, 0xFF, 0);
            }
            else if (low_opcode == 0x03)
            {
                //JP nn
                dispatchMemOp(PC+1, DATA_BUS, 0);
                PC++;
                data = *DATA_BUS;
                dispatchMemOp(PC+1, DATA_BUS, 0);
                PC++;
                data = (data | (*DATA_BUS << 8));
                PC = data;
            }
            else
            {
                updateFailMetrics(high_opcode, mid_opcode, low_opcode);
            }
        }
    }

    //10
    else if (high_opcode == 0x02)
    {
        if (mid_opcode == 0x00)
        {
            if (low_opcode == 0x06)
            {
                //ADD (HL)
                dispatchMemOp(HL_FULL_ADDRESS, DATA_BUS, 0);
                doALUOp(&REGISTER_FILE[A_REGISTER], REGISTER_FILE[A_REGISTER], *DATA_BUS, 0xFF, 0);
            }
            else
            {
                //ADD r
                doALUOp(&REGISTER_FILE[A_REGISTER], REGISTER_FILE[A_REGISTER], REGISTER_FILE[low_opcode], 0xFF, 0);
            }
        }
        else if (mid_opcode == 0x01)
        {
            if (low_opcode == 0x06)
            {
                //ADC (HL)
                dispatchMemOp(HL_FULL_ADDRESS, DATA_BUS, 0);
                doALUOp(&REGISTER_FILE[A_REGISTER], REGISTER_FILE[A_REGISTER], *DATA_BUS + ((REGISTER_FILE[F_REGISTER] >> 4)%2), 0xFF, 0);
            }
            else
            {
                //ADC r
                //SET FLAGS               
                doALUOp(&REGISTER_FILE[A_REGISTER], REGISTER_FILE[A_REGISTER], REGISTER_FILE[low_opcode]+((REGISTER_FILE[F_REGISTER] >> 4)%2), 0xFF, 0);
            }                
        }
        else if (mid_opcode == 0x02)
        {
            if (low_opcode == 0x06)
            {
                //SUB (HL)
                dispatchMemOp(HL_FULL_ADDRESS, DATA_BUS, 0);
                doALUOp(&REGISTER_FILE[A_REGISTER], REGISTER_FILE[A_REGISTER], *DATA_BUS, 0xFF, 1);
            }
            else
            {
                //SUB r
                doALUOp(&REGISTER_FILE[A_REGISTER], REGISTER_FILE[A_REGISTER], REGISTER_FILE[low_opcode], 0xFF, 1);
            }
        }
        else if (mid_opcode == 0x03)
        {
            if (low_opcode == 0x06)
            {
                //SBC (HL)
                dispatchMemOp(HL_FULL_ADDRESS, DATA_BUS, 0);
                doALUOp(&REGISTER_FILE[A_REGISTER], REGISTER_FILE[A_REGISTER], *DATA_BUS - ((REGISTER_FILE[F_REGISTER] >> 4) % 2), 0xFF, 1);
            }
            else
            {
                //SBC r
                doALUOp(&REGISTER_FILE[A_REGISTER], REGISTER_FILE[A_REGISTER], REGISTER_FILE[low_opcode] - ((REGISTER_FILE[F_REGISTER] >> 4) %2), 0xFF, 1);
            }
        }
        else if (mid_opcode == 0x04)
        {
            if (low_opcode == 0x06)
            {
                //AND (HL)
                dispatchMemOp(HL_FULL_ADDRESS, DATA_BUS, 0);
                REGISTER_FILE[A_REGISTER] = (REGISTER_FILE[A_REGISTER] & (*DATA_BUS));
                if (REGISTER_FILE[A_REGISTER] == 0)
                {
                    REGISTER_FILE[F_REGISTER] = 0xA0;
                }
                else
                {
                    REGISTER_FILE[F_REGISTER] = 0x20;
                }
            }
            else
            {
                //AND r
                REGISTER_FILE[A_REGISTER] = (REGISTER_FILE[A_REGISTER] & REGISTER_FILE[low_opcode]);
                if (REGISTER_FILE[A_REGISTER] == 0)
                {
                    REGISTER_FILE[F_REGISTER] = 0xA0;
                }
                else
                {
                    REGISTER_FILE[F_REGISTER] = 0x20;
                }
            }
        }
        else if (mid_opcode == 0x05)
        {
            if (low_opcode == 0x06)
            {
                //XOR (HL)
                dispatchMemOp(HL_FULL_ADDRESS, DATA_BUS, 0);
                REGISTER_FILE[A_REGISTER] = (REGISTER_FILE[A_REGISTER] ^ (*DATA_BUS));
                if (REGISTER_FILE[A_REGISTER] == 0)
                {
                    REGISTER_FILE[F_REGISTER] = 0x80;
                }
                else
                {
                    REGISTER_FILE[F_REGISTER] = 0x00;
                }
            }
            else
            {
                //XOR r
                REGISTER_FILE[A_REGISTER] = (REGISTER_FILE[A_REGISTER] ^ REGISTER_FILE[low_opcode]);
                if (REGISTER_FILE[A_REGISTER] == 0)
                {
                    REGISTER_FILE[F_REGISTER] = 0x80;
                }
                else
                {
                    REGISTER_FILE[F_REGISTER] = 0x00;
                }
            }
        }
        else if (mid_opcode == 0x06)
        {
            if (low_opcode == 0x06)
            {
                //OR (HL)
                dispatchMemOp(HL_FULL_ADDRESS, DATA_BUS, 0);
                REGISTER_FILE[A_REGISTER] = (REGISTER_FILE[A_REGISTER] | (*DATA_BUS));
                if (REGISTER_FILE[A_REGISTER] == 0)
                {
                    REGISTER_FILE[F_REGISTER] = 0x80;
                }
                else
                {
                    REGISTER_FILE[F_REGISTER] = 0x00;
                }
            }
            else
            {
                //OR r
                REGISTER_FILE[A_REGISTER] = (REGISTER_FILE[A_REGISTER] | REGISTER_FILE[low_opcode]);
                if (REGISTER_FILE[A_REGISTER] == 0)
                {
                    REGISTER_FILE[F_REGISTER] = 0x80;
                }
                else
                {
                    REGISTER_FILE[F_REGISTER] = 0x00;
                }
            }
        }
        else if (mid_opcode == 0x07)
        {
            if (low_opcode == 0x06)
            {
                //CP (HL)
                dispatchMemOp(HL_FULL_ADDRESS, DATA_BUS, 0);
                doALUOp(DATA_BUS, REGISTER_FILE[A_REGISTER], *DATA_BUS, 0xFF, 1);
            }
            else
            {
                //CP r
                doALUOp(DATA_BUS, REGISTER_FILE[A_REGISTER], REGISTER_FILE[low_opcode], 0xFF, 1); 
            }
        }
        else
        {
            updateFailMetrics(high_opcode, mid_opcode, low_opcode);
        }
    }

    //01
    else if (high_opcode == 0x01)
    {
        if (mid_opcode == 0x06)
        {
            //LD (HL), r
            dispatchMemOp(HL_FULL_ADDRESS, &(REGISTER_FILE[low_opcode]), 1);
        }
        else if (low_opcode == 0x06)
        {
            //LD r, (HL)
            dispatchMemOp(HL_FULL_ADDRESS, &(REGISTER_FILE[mid_opcode]), 0);
        }
        else
        {
            //LD r, r'
            REGISTER_FILE[mid_opcode] = REGISTER_FILE[low_opcode];
        }
    }

    //00
    else if (high_opcode == 0x00)
    {
        if (low_opcode == 0x07)
        {
            if (mid_opcode == 0x07)
            {
                //CCF
                //Clear N and H
                REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] & 0x90);
                //Invert C
                REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] ^ 0x10);
            }
            else if (mid_opcode == 0x06)
            {
                //SCF
                //Clear N and H
                REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] & 0x80);
                //Set C
                REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] | 0x10);
            }
            else if (mid_opcode == 0x05)
            {
                //CPL
                REGISTER_FILE[A_REGISTER] = (REGISTER_FILE[A_REGISTER] ^ 0xFF);
                REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] | 0x60);
            }
            else if (mid_opcode == 0x04)
            {
                //DAA
                //Last ALU op was ADD
                if ((REGISTER_FILE[F_REGISTER] >> 6)%2 == 0)
                {
                    //There was no Carry at bit 7 CY
                    if ((REGISTER_FILE[F_REGISTER] >> 4)%2 == 0)
                    {
                        //There was no Carry at bit 3 H
                        if ((REGISTER_FILE[F_REGISTER] >> 5)%2 == 0)
                        {
                            //Handling cases
                            if ((REGISTER_FILE[A_REGISTER] >> 4) < 0x09)
                            {
                                if ((REGISTER_FILE[A_REGISTER] & 0x0F) > 0x09)
                                {
                                    REGISTER_FILE[A_REGISTER] = (REGISTER_FILE[A_REGISTER] + 0x06);
                                }
                            }
                            else
                            {
                                if ((REGISTER_FILE[A_REGISTER] >> 4) > 0x09)
                                {
                                    if ((REGISTER_FILE[A_REGISTER] & 0x0F) < 0x0A)
                                    {
                                        REGISTER_FILE[A_REGISTER] = (REGISTER_FILE[A_REGISTER] + 0x60);
                                        REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] | 0x10);
                                    }
                                }
                                else
                                {
                                    if ((REGISTER_FILE[A_REGISTER] & 0x0F) > 0x09)
                                    {
                                        REGISTER_FILE[A_REGISTER] = (REGISTER_FILE[A_REGISTER] + 0x66);
                                        REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] | 0x10);
                                    }
                                }
                            }
                            
                        }
                        //There was Carry at bit 3 H
                        else
                        {
                            //Handling cases
                            if ((REGISTER_FILE[A_REGISTER] >> 4) < 0x0A)
                            {
                                if ((REGISTER_FILE[A_REGISTER] & 0x0F) < 0x04)
                                {
                                    REGISTER_FILE[A_REGISTER] = (REGISTER_FILE[A_REGISTER] + 0x06);
                                }
                            }
                            else
                            {
                                if ((REGISTER_FILE[A_REGISTER] & 0x0F) < 0x04)
                                {
                                    REGISTER_FILE[A_REGISTER] = (REGISTER_FILE[A_REGISTER] + 0x66);
                                    REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] | 0x10);
                                }
                            }
                        }
                    }
                    //There was Carry at bit 7
                    else
                    {
                        //There was no Carry at bit 3 H
                        if ((REGISTER_FILE[F_REGISTER] >> 5)%2 == 0)
                        {
                            //Handling cases
                            if ((REGISTER_FILE[A_REGISTER] >> 4) < 0x03)
                            {
                                if ((REGISTER_FILE[A_REGISTER] & 0x0F) < 0x0A)
                                {
                                    REGISTER_FILE[A_REGISTER] = (REGISTER_FILE[A_REGISTER] + 0x60);
                                    REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] | 0x10);
                                }
                                else
                                {
                                    REGISTER_FILE[A_REGISTER] = (REGISTER_FILE[A_REGISTER] + 0x66);
                                    REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] | 0x10);
                                }
                            }
                        }
                        //There was Carry at bit 3 H
                        else
                        {
                            //Handling cases
                            if ((REGISTER_FILE[A_REGISTER] >> 4) < 0x04)
                            {
                                if ((REGISTER_FILE[A_REGISTER] & 0x0F) < 0x04)
                                {
                                    REGISTER_FILE[A_REGISTER] = (REGISTER_FILE[A_REGISTER] + 0x66);
                                    REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] | 0x10);
                                }
                            }
                        }
                    }
                }
                //Last ALU op was SUB
                else
                {
                    //There was no Carry at bit 7 CY
                    if ((REGISTER_FILE[F_REGISTER] >> 4)%2 == 0)
                    {
                        //We ignore if Carry at bit 3 H is 0
                        //There was Carry at bit 3 H
                        if ((REGISTER_FILE[F_REGISTER] >> 5)%2 == 1)
                        {
                            //Handling cases
                            if ((REGISTER_FILE[A_REGISTER] >> 4) < 0x09)
                            {
                                if ((REGISTER_FILE[A_REGISTER] & 0x0F) > 0x05)
                                {
                                    REGISTER_FILE[A_REGISTER] = (REGISTER_FILE[A_REGISTER] + 0xFA);
                                }
                            }
                        }
                    }
                    //There was Carry at bit 7
                    else
                    {
                        //There was no Carry at bit 3 H
                        if ((REGISTER_FILE[F_REGISTER] >> 5)%2 == 0)
                        {
                            //Handling cases
                            if ((REGISTER_FILE[A_REGISTER] >> 4) > 0x06)
                            {
                                if ((REGISTER_FILE[A_REGISTER] & 0x0F) < 0x0A)
                                {
                                    REGISTER_FILE[A_REGISTER] = (REGISTER_FILE[A_REGISTER] + 0xA0);
                                    REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] | 0x10);
                                }
                            }
                        }
                        //There was Carry at bit 3 H
                        else
                        {
                            //Handling cases
                            if ((REGISTER_FILE[A_REGISTER] >> 4) > 0x05)
                            {
                                if ((REGISTER_FILE[A_REGISTER] & 0x0F) > 0x05)
                                {
                                    REGISTER_FILE[A_REGISTER] = (REGISTER_FILE[A_REGISTER] + 0x9A);
                                    REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] | 0x10);
                                }
                            }
                        }
                    }
                }
                REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] & 0xD0);
            }
            else if (mid_opcode == 0x02)
            {
                //RLA
                *DATA_BUS = (REGISTER_FILE[A_REGISTER] >> 7);
                REGISTER_FILE[A_REGISTER] = ((REGISTER_FILE[A_REGISTER] << 1) | ((REGISTER_FILE[F_REGISTER] >> 4)%2));
                REGISTER_FILE[F_REGISTER] = (*DATA_BUS << 4);
            }
            else
            {
                updateFailMetrics(high_opcode, mid_opcode, low_opcode);
            }
        }
        else if (low_opcode == 0x06)
        {
            if (mid_opcode == 0x06)
            {
                //LD (HL), n
                //LOAD IMMEDIATE IN HL
                dispatchMemOp(PC+1, DATA_BUS, 0);
                PC++;
                dispatchMemOp(HL_FULL_ADDRESS, DATA_BUS, 1);
            }
            else
            {
                //LD r, n
                dispatchMemOp(PC+1, &REGISTER_FILE[mid_opcode], 0);
                PC++;
                //std::cout << "Stored value: " << (unsigned short)REGISTER_FILE[mid_opcode] << std::endl;
            }
        }
        else if (low_opcode == 0x05)
        {
            if (mid_opcode == 0x06)
            {
                //DEC (HL)
                dispatchMemOp(HL_FULL_ADDRESS, DATA_BUS, 0);
                doALUOp(DATA_BUS, *DATA_BUS, 1, 0xFF, 1);
                dispatchMemOp(HL_FULL_ADDRESS, DATA_BUS, 1);
            }
            else
            {
                //DEC r
                doALUOp(&REGISTER_FILE[mid_opcode], REGISTER_FILE[mid_opcode], 1, 0xFF, 1);
            }
        }
        else if (low_opcode == 0x04)
        {
            if (mid_opcode == 0x06)
            {
                //INC (HL)
                dispatchMemOp(HL_FULL_ADDRESS, DATA_BUS, 0);
                doALUOp(DATA_BUS, *DATA_BUS, 1, 0xFF, 0);
                dispatchMemOp(HL_FULL_ADDRESS, DATA_BUS, 1);
            }
            else
            {
                //INC r
                doALUOp(&REGISTER_FILE[mid_opcode], REGISTER_FILE[mid_opcode], 1, 0xFF, 0);
            }
        }
        else if (low_opcode == 0x03)
        {
            if (mid_opcode%2 == 0x00)
            {
                //INC rr
                unsigned char select_reg = (mid_opcode >> 1);

                switch (select_reg)
                {
                    case 0:
                        data = ((REGISTER_FILE[B_REGISTER] << 8) | REGISTER_FILE[C_REGISTER]) + 1;
                        REGISTER_FILE[B_REGISTER] = (char)(data >> 8);
                        REGISTER_FILE[C_REGISTER] = (char)(data);
                    break;
                    case 1:
                        data = ((REGISTER_FILE[D_REGISTER] << 8) | REGISTER_FILE[E_REGISTER]) + 1;
                        REGISTER_FILE[D_REGISTER] = (char)(data >> 8);
                        REGISTER_FILE[E_REGISTER] = (char)(data);
                    break;

                    case 2:
                        data = ((REGISTER_FILE[H_REGISTER] << 8) | REGISTER_FILE[L_REGISTER]) + 1;
                        REGISTER_FILE[H_REGISTER] = (char)(data >> 8);
                        REGISTER_FILE[L_REGISTER] = (char)(data);
                    break;

                    case 3:
                        data = ((REGISTER_FILE[SP_HI_REGISTER] << 8) | REGISTER_FILE[SP_LO_REGISTER]) + 1;
                        REGISTER_FILE[SP_HI_REGISTER] = (char)(data >> 8);
                        REGISTER_FILE[SP_LO_REGISTER] = (char)(data);
                    break;
                }
            }
            else
            {
                //DEC rr
                unsigned char select_reg = (mid_opcode >> 1);

                switch (select_reg)
                {
                    case 0:
                        data = ((REGISTER_FILE[B_REGISTER] << 8) | REGISTER_FILE[C_REGISTER]) - 1;
                        REGISTER_FILE[B_REGISTER] = (char)(data >> 8);
                        REGISTER_FILE[C_REGISTER] = (char)(data);
                    break;
                    case 1:
                        data = ((REGISTER_FILE[D_REGISTER] << 8) | REGISTER_FILE[E_REGISTER]) - 1;
                        REGISTER_FILE[D_REGISTER] = (char)(data >> 8);
                        REGISTER_FILE[E_REGISTER] = (char)(data);
                    break;

                    case 2:
                        data = ((REGISTER_FILE[H_REGISTER] << 8) | REGISTER_FILE[L_REGISTER]) - 1;
                        REGISTER_FILE[H_REGISTER] = (char)(data >> 8);
                        REGISTER_FILE[L_REGISTER] = (char)(data);
                    break;

                    case 3:
                        data = ((REGISTER_FILE[SP_HI_REGISTER] << 8) | REGISTER_FILE[SP_LO_REGISTER]) - 1;
                        REGISTER_FILE[SP_HI_REGISTER] = (char)(data >> 8);
                        REGISTER_FILE[SP_LO_REGISTER] = (char)(data);
                    break;
                }
            }
        }
        else if (low_opcode == 0x02)
        {
            if (mid_opcode == 0x07)
            {
                //LD A, (HL-)
                REGISTER_FILE[A_REGISTER] = HL_FULL_ADDRESS;
                HL_FULL_ADDRESS--;
                REGISTER_FILE[H_REGISTER] = (HL_FULL_ADDRESS >> 8);
                REGISTER_FILE[L_REGISTER] = (char)HL_FULL_ADDRESS;
            }
            else if (mid_opcode == 0x06)
            {
                //LD (HL-), A
                dispatchMemOp(HL_FULL_ADDRESS, &REGISTER_FILE[A_REGISTER], 1);
                HL_FULL_ADDRESS--;
                REGISTER_FILE[H_REGISTER] = (char)(HL_FULL_ADDRESS >> 8);
                REGISTER_FILE[L_REGISTER] = (char)HL_FULL_ADDRESS;
            }
            else if (mid_opcode == 0x05)
            {
                //LD A, (HL+)
                REGISTER_FILE[A_REGISTER] = HL_FULL_ADDRESS;
                HL_FULL_ADDRESS++;
                REGISTER_FILE[H_REGISTER] = (HL_FULL_ADDRESS >> 8);
                REGISTER_FILE[L_REGISTER] = (char)HL_FULL_ADDRESS;
            }
            else if (mid_opcode == 0x04)
            {
                //LD (HL+), A
                dispatchMemOp(HL_FULL_ADDRESS, &REGISTER_FILE[A_REGISTER], 1);
                HL_FULL_ADDRESS++;
                REGISTER_FILE[H_REGISTER] = (char)(HL_FULL_ADDRESS >> 8);
                REGISTER_FILE[L_REGISTER] = (char)HL_FULL_ADDRESS;
            }
            else if (mid_opcode == 0x03)
            {
                //LD A, (DE)
                dispatchMemOp((REGISTER_FILE[D_REGISTER] << 8) | REGISTER_FILE[E_REGISTER], &REGISTER_FILE[A_REGISTER], 0);
            }
            else if (mid_opcode == 0x02)
            {
                //LD (DE), A
                dispatchMemOp((REGISTER_FILE[D_REGISTER] << 8) | REGISTER_FILE[E_REGISTER], &REGISTER_FILE[A_REGISTER], 1);
            }
            else if (mid_opcode == 0x01)
            {
                //LD A, (BC)
                dispatchMemOp((REGISTER_FILE[B_REGISTER] << 8) | REGISTER_FILE[C_REGISTER], &REGISTER_FILE[A_REGISTER], 1);
            }
            else if (mid_opcode == 0x00)
            {
                //LD (BC), A
                dispatchMemOp((REGISTER_FILE[B_REGISTER] << 8) | REGISTER_FILE[C_REGISTER], &REGISTER_FILE[A_REGISTER], 1);
            }
            else
            {
                updateFailMetrics(high_opcode, mid_opcode, low_opcode);
            }
        }
        else if (low_opcode == 0x01)
        {
            if (mid_opcode%2 == 0x00)
            {
                //LD rr, nn
                unsigned char select_reg = (mid_opcode >> 1);
                dispatchMemOp(PC+2, DATA_BUS, 0);
                data = (*DATA_BUS << 8);
                dispatchMemOp(PC+1, DATA_BUS, 0);
                data = (data | *DATA_BUS);
                PC += 2;

                switch (select_reg)
                {
                    case 0:
                        REGISTER_FILE[B_REGISTER] = (char)(data >> 8);
                        REGISTER_FILE[C_REGISTER] = (char)(data);
                    break;
                    case 1:
                        REGISTER_FILE[D_REGISTER] = (char)(data >> 8);
                        REGISTER_FILE[E_REGISTER] = (char)(data);
                    break;

                    case 2:
                        REGISTER_FILE[H_REGISTER] = (char)(data >> 8);
                        REGISTER_FILE[L_REGISTER] = (char)(data);
                    break;

                    case 3:
                        REGISTER_FILE[SP_HI_REGISTER] = (char)(data >> 8);
                        REGISTER_FILE[SP_LO_REGISTER] = (char)(data);
                    break;
                }
            }
            else
            {
                //ADD HL, rr
                //#TODO better handling of F_REGISTER
                unsigned char select_reg = (mid_opcode >> 1);

                switch (select_reg)
                {
                    case 0:
                        data = ((REGISTER_FILE[B_REGISTER] << 8) | REGISTER_FILE[C_REGISTER]);
                    break;
                    case 1:
                        data = ((REGISTER_FILE[D_REGISTER] << 8) | REGISTER_FILE[E_REGISTER]);
                    break;

                    case 2:
                        data = ((REGISTER_FILE[H_REGISTER] << 8) | REGISTER_FILE[L_REGISTER]);
                    break;

                    case 3:
                        data = ((REGISTER_FILE[SP_HI_REGISTER] << 8) | REGISTER_FILE[SP_LO_REGISTER]);
                    break;
                }
                doALUOp(&REGISTER_FILE[L_REGISTER], REGISTER_FILE[L_REGISTER], (unsigned char)(data), 0xFF, 0);
                doALUOp(&REGISTER_FILE[H_REGISTER], REGISTER_FILE[H_REGISTER], (unsigned char)(data >> 8), 0xFF, 0);
            }
        }
        else if (low_opcode == 0x00)
        {
            if ((mid_opcode >> 2)%2 == 1)
            {
                //JR cc, e
                dispatchMemOp(PC+1, DATA_BUS, 0);
                PC++;
                unsigned char cc = mid_opcode%4;
                switch (cc)
                {
                    //NZ
                    case 0:
                        if ((REGISTER_FILE[F_REGISTER] >> 7) == 0)
                        {
                            //std::cout << (unsigned short) *DATA_BUS << std::endl;
                            PC = (PC + (signed char)(*DATA_BUS));
                        }
                    break;
                    //Z
                    case 1:
                        if ((REGISTER_FILE[F_REGISTER] >> 7) == 1)
                        {
                            PC = (PC + (signed char)(*DATA_BUS));
                        }
                    break;
                    //NC
                    case 2:
                        if ((REGISTER_FILE[F_REGISTER] >> 4)%2 == 0)
                        {
                            PC = (PC + (signed char)(*DATA_BUS));
                        }
                    break;
                    //C
                    case 3:
                        if ((REGISTER_FILE[F_REGISTER] >> 4)%2 == 1)
                        {
                            PC = (PC + (signed char)(*DATA_BUS));
                        }
                    break;
                }
            }
            else if (mid_opcode == 0x03)
            {
                //JR e
                dispatchMemOp(PC+1, DATA_BUS, 0);
                PC++;
                PC = (PC + (signed char)(*DATA_BUS));
            }
            else if (mid_opcode == 0x01)
            {
                //LD (nn), SP
                dispatchMemOp(PC+1, &REGISTER_FILE[SP_LO_REGISTER], 1);
                dispatchMemOp(PC+2, &REGISTER_FILE[SP_HI_REGISTER], 1);
                PC += 2;
            }
            else
            {
                updateFailMetrics(high_opcode, mid_opcode, low_opcode);
            }
        }

    }

    updateMetrics(high_opcode, mid_opcode, low_opcode);

}

void cpuloop()
{
    fetch();
    execute();

    PC++;
}

void cpuboot(char* boot_rom_path, int boot_rom_size, char* rom_path)
{
    //DEBUG VARIABLES
    INSTRUCTION_FAILS = 0;
    NOP_COUNTER = 0;
    INSTRUCTION_COUNTER = 0;
    PROGRAM_SIZE = 0;

    initWRAM(boot_rom_path, boot_rom_size);

    PROGRAM_SIZE = boot_rom_size;
    int bytes_read = 0;
    if (*rom_path != 0)
    {
        bytes_read = initCartridgeROM(rom_path);
        PROGRAM_SIZE += bytes_read;
    }

    REGISTER_FILE[F_REGISTER] = 0x80;

    //printVRAM();
}

