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

//TRUE RAM
//int RAM_SIZE = 8*(1 << 10);

//TEST RAM
//int RAM_SIZE = 256;

//ONE RAM FOR EVERYTHING
//int RAM_SIZE = 0xFFFFFF+1;

int VIDEO_RAM_SIZE = 8*(1 << 10);
int REGISTER_FILE_SIZE = 14;

unsigned char* VIDEO_RAM = new unsigned char[VIDEO_RAM_SIZE];

int PC = WRAM_BANK_0_ADDRESS_START;

unsigned char* INSTRUCTION_REGISTER = new unsigned char;
unsigned char* INTERRUPT_ENABLE = new unsigned char;

unsigned short* BUS = new unsigned short;

unsigned char* DATA_BUS = new unsigned char;

//RESOLUTION IN PIXELS
int SCREEN_WIDTH = 160;
int SCREEN_HEIGHT = 144;

int SPRITE_SIZE = 8;

unsigned char* REGISTER_FILE = new unsigned char[REGISTER_FILE_SIZE];
int A_REGISTER = 0x07;
int B_REGISTER = 0x00;
int C_REGISTER = 0x01;
int D_REGISTER = 0x02;
int E_REGISTER = 0x03;
int H_REGISTER = 0x04;
int L_REGISTER = 0x05;
//1 2 3 4 5  6  7 8
//S Z X H X P/V N C
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

void fetch()
{
    dispatchMemOp(PC, INSTRUCTION_REGISTER, 0);


    std::cout << INSTRUCTION_COUNTER << ": " << "RETRIEVED INST - 0x" << std::hex << (unsigned short)*INSTRUCTION_REGISTER << std::dec << " | 0b" << parseBytes(INSTRUCTION_REGISTER, 1) << std::endl << std::flush;
}

void execute()
{

    unsigned char instruction = *INSTRUCTION_REGISTER;
    unsigned char data = (unsigned char)*BUS;

    unsigned char high_opcode = (unsigned char)((instruction & 0xC0) >> 6);
    unsigned char mid_opcode = (unsigned char)((instruction & 0b111000) >> 3);
    unsigned char low_opcode = (unsigned char)(instruction & 0b111);
    unsigned short SP_FULL_ADDRESS = ((REGISTER_FILE[SP_HI_REGISTER] << 8) | REGISTER_FILE[SP_LO_REGISTER]);
    unsigned short HL_FULL_ADDRESS = ((REGISTER_FILE[H_REGISTER] << 8) | REGISTER_FILE[L_REGISTER]);

    //11
    if (high_opcode == 0x03)
    {
        if (low_opcode == 0x02 && mid_opcode%2 == 0)
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
        }
        else if (low_opcode == 0x01 && mid_opcode%2 == 0)
        {
            //POP rr
            unsigned char select_reg = (mid_opcode >> 1);
            switch (select_reg)
            {
                case 0:
                    dispatchMemOp(SP_FULL_ADDRESS, &(REGISTER_FILE[B_REGISTER]), 0);
                    SP_FULL_ADDRESS++;
                    dispatchMemOp(SP_FULL_ADDRESS, &(REGISTER_FILE[C_REGISTER]), 0);
                break;
                case 1:
                    dispatchMemOp(SP_FULL_ADDRESS, &(REGISTER_FILE[D_REGISTER]), 0);
                    SP_FULL_ADDRESS++;
                    dispatchMemOp(SP_FULL_ADDRESS, &(REGISTER_FILE[E_REGISTER]), 0);
                break;

                case 2:
                    dispatchMemOp(SP_FULL_ADDRESS, &(REGISTER_FILE[H_REGISTER]), 0);
                    SP_FULL_ADDRESS++;
                    dispatchMemOp(SP_FULL_ADDRESS, &(REGISTER_FILE[L_REGISTER]), 0);
                break;

                case 3:
                    dispatchMemOp(SP_FULL_ADDRESS, &(REGISTER_FILE[A_REGISTER]), 0);
                    SP_FULL_ADDRESS++;
                    dispatchMemOp(SP_FULL_ADDRESS, &(REGISTER_FILE[F_REGISTER]), 0);
                break;
            }
            SP_FULL_ADDRESS++;
        }
        else if (mid_opcode == 0x06)
        {
            if (low_opcode == 0x00)
            {
                //LDH A, (n)
                dispatchMemOp(PC+1, DATA_BUS, 0);
                REGISTER_FILE[A_REGISTER] = (0xFF | *DATA_BUS);
                PC++;
            }
            else if (low_opcode == 0x02)
            {
                //LDH A, (C)
                dispatchMemOp((0xFF | REGISTER_FILE[C_REGISTER]), DATA_BUS, 0);
                REGISTER_FILE[A_REGISTER] = *DATA_BUS;
            }
        }
        else if (mid_opcode == 0x04)
        {
            if (low_opcode == 0x00)
            {
                //LDH (n), A
                dispatchMemOp(PC+1, &(REGISTER_FILE[A_REGISTER]), 1);
                PC++;
            }
            else if (low_opcode == 0x02)
            {
                //LDH (C), A
                dispatchMemOp((0xFF | REGISTER_FILE[C_REGISTER]), &(REGISTER_FILE[A_REGISTER]), 1);
            }
        } 
        else if (mid_opcode == 0x07)
        {
            if (low_opcode == 0x02)
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
                //INIT FLAGS
                REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] & 0xBD);

                //INIT VARS                
                signed char operand_e = 0;
                dispatchMemOp(PC+1, (unsigned char *)&operand_e, 0);
                PC++;

                REGISTER_FILE[H_REGISTER] = REGISTER_FILE[SP_HI_REGISTER];

                //SET FLAGS
                unsigned char check_bit_3 = (REGISTER_FILE[SP_LO_REGISTER] >> 2)%2;
                unsigned char check_bit_7 = (REGISTER_FILE[SP_LO_REGISTER] >> 6)%2;

                REGISTER_FILE[L_REGISTER] = (unsigned char)REGISTER_FILE[SP_LO_REGISTER] + (signed char)operand_e;

                //H BIT = 4
                if (check_bit_3 == 1 && ((REGISTER_FILE[L_REGISTER] >> 2)%2 == 0))
                {
                    REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] | 0x08);
                }
                else
                {
                    REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] & 0xF7);
                }

                //C BIT = 0
                if (check_bit_7 == 1 && ((REGISTER_FILE[L_REGISTER] >> 6)%2 == 0))
                {
                    REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] | 0x01);
                }
                else
                {
                    REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] & 0xFE);
                }

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
        }
        else if (mid_opcode == 0x02)
        {
            if (low_opcode == 0x06)
            {
                //SUB n
                //INIT FLAGS
                REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] | 0xFD);

                //INIT VARS
                unsigned char operand_immediate;
                dispatchMemOp(PC+1, &operand_immediate, 0);
                PC++;

                //SET FLAGS
                unsigned char check_bit_3 = (REGISTER_FILE[A_REGISTER] >> 2)%2;
                unsigned char check_bit_7 = (REGISTER_FILE[A_REGISTER] >> 6)%2;
                REGISTER_FILE[A_REGISTER] -= operand_immediate;

                if (REGISTER_FILE[A_REGISTER] == 0)
                {
                    REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] | 0x40);
                }
                else
                {
                    REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] & 0xBF);
                }
                //H BIT = 4
                if (check_bit_3 == 0 && ((REGISTER_FILE[A_REGISTER] >> 2)%2 == 1))
                {
                    REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] | 0x08);
                }
                else
                {
                    REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] & 0xF7);
                }

                //C BIT = 0
                if (check_bit_7 == 0 && ((REGISTER_FILE[A_REGISTER] >> 6)%2 == 1))
                {
                    REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] | 0x01);
                }
                else
                {
                    REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] & 0xFE);
                }
                
            }
        }
        else if (mid_opcode == 0x01)
        {
            if (low_opcode == 0x06)
            {
                //ADC n
                //INIT FLAGS
                REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] & 0xFD);

                //INIT VARS
                unsigned char operand_immediate;
                dispatchMemOp(PC+1, &operand_immediate, 0);
                PC++;

                //SET FLAGS
                unsigned char check_bit_3 = (REGISTER_FILE[A_REGISTER] >> 2)%2;
                unsigned char check_bit_7 = (REGISTER_FILE[A_REGISTER] >> 6)%2;
                REGISTER_FILE[A_REGISTER] += (operand_immediate + (REGISTER_FILE[F_REGISTER]%2));

                if (REGISTER_FILE[A_REGISTER] == 0)
                {
                    REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] | 0x40);
                }
                else
                {
                    REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] & 0xBF);
                }
                //H BIT = 4
                if (check_bit_3 == 1 && ((REGISTER_FILE[A_REGISTER] >> 2)%2 == 0))
                {
                    REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] | 0x08);
                }
                else
                {
                    REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] & 0xF7);
                }

                //C BIT = 0
                if (check_bit_7 == 1 && ((REGISTER_FILE[A_REGISTER] >> 6)%2 == 0))
                {
                    REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] | 0x01);
                }
                else
                {
                    REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] & 0xFE);
                }
            }
        }
        else if (mid_opcode == 0x00)
        {
            if (low_opcode == 0x06)
            {
                //ADD n
                //INIT FLAGS
                REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] & 0xFD);

                //INIT VARS
                signed char operand_immediate;
                dispatchMemOp(PC+1, (unsigned char*)&operand_immediate, 0);
                PC++;

                //SET FLAGS
                unsigned char check_bit_3 = (REGISTER_FILE[A_REGISTER] >> 2)%2;
                unsigned char check_bit_7 = (REGISTER_FILE[A_REGISTER] >> 6)%2;
                REGISTER_FILE[A_REGISTER] += operand_immediate;

                if (REGISTER_FILE[A_REGISTER] == 0)
                {
                    REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] | 0x40);
                }
                else
                {
                    REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] & 0xBF);
                }
                //H BIT = 4
                if (check_bit_3 == 1 && ((REGISTER_FILE[A_REGISTER] >> 2)%2 == 0))
                {
                    REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] | 0x08);
                }
                else
                {
                    REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] & 0xF7);
                }

                //C BIT = 0
                if (check_bit_7 == 1 && ((REGISTER_FILE[A_REGISTER] >> 6)%2 == 0))
                {
                    REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] | 0x01);
                }
                else
                {
                    REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] & 0xFE);
                }
            }
        }
    }

    //10
    if (high_opcode == 0x02)
    {
        if (mid_opcode == 0x00)
        {
            if (low_opcode == 0x06)
            {
                //ADD (HL)
                //SET FLAGS
                REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] | 0xFD);
                unsigned char check_bit_3 = (REGISTER_FILE[A_REGISTER] >> 2)%2;
                unsigned char check_bit_7 = (REGISTER_FILE[A_REGISTER] >> 6)%2;

                dispatchMemOp(HL_FULL_ADDRESS, DATA_BUS, 0);
                REGISTER_FILE[A_REGISTER] += *DATA_BUS;

                //Z BIT = 6
                if (REGISTER_FILE[A_REGISTER] == 0)
                {
                    REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] | 0x40);
                }
                else
                {
                    REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] & 0xBF);
                }

                //H BIT = 4
                if (check_bit_3 == 1 && ((REGISTER_FILE[L_REGISTER] >> 2)%2 == 0))
                {
                    REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] | 0x08);
                }
                else
                {
                    REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] & 0xF7);
                }

                //C BIT = 0
                if (check_bit_7 == 1 && ((REGISTER_FILE[L_REGISTER] >> 6)%2 == 0))
                {
                    REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] | 0x01);
                }
                else
                {
                    REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] & 0xFE);
                }
                ;
            }
            else if (mid_opcode == 0x01)
            {
                if (low_opcode == 0x06)
                {
                    //ADC (HL)
                    //SET FLAGS               
                    REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] & 0xFD);
                    unsigned char check_bit_3 = (REGISTER_FILE[A_REGISTER] >> 2)%2;
                    unsigned char check_bit_7 = (REGISTER_FILE[A_REGISTER] >> 6)%2;

                    dispatchMemOp(HL_FULL_ADDRESS, DATA_BUS, 0);
                    REGISTER_FILE[A_REGISTER] += (*DATA_BUS + (REGISTER_FILE[F_REGISTER]%2));

                    //Z BIT = 6
                    if (REGISTER_FILE[A_REGISTER] == 0)
                    {
                        REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] | 0x40);
                    }
                    else
                    {
                        REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] & 0xBF);
                    }

                    //H BIT = 4
                    if (check_bit_3 == 1 && ((REGISTER_FILE[L_REGISTER] >> 2)%2 == 0))
                    {
                        REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] | 0x08);
                    }
                    else
                    {
                        REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] & 0xF7);
                    }

                    //C BIT = 0
                    if (check_bit_7 == 1 && ((REGISTER_FILE[L_REGISTER] >> 6)%2 == 0))
                    {
                        REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] | 0x01);
                    }
                    else
                    {
                        REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] & 0xFE);
                    }
                    
                }
                else
                {
                    //ADC r
                    //SET FLAGS               
                    REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] & 0xFD);
                    unsigned char check_bit_3 = (REGISTER_FILE[A_REGISTER] >> 2)%2;
                    unsigned char check_bit_7 = (REGISTER_FILE[A_REGISTER] >> 6)%2;

                    REGISTER_FILE[A_REGISTER] += REGISTER_FILE[low_opcode] + (REGISTER_FILE[F_REGISTER]%2);

                    //Z BIT = 6
                    if (REGISTER_FILE[A_REGISTER] == 0)
                    {
                        REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] | 0x40);
                    }
                    else
                    {
                        REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] & 0xBF);
                    }

                    //H BIT = 4
                    if (check_bit_3 == 1 && ((REGISTER_FILE[L_REGISTER] >> 2)%2 == 0))
                    {
                        REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] | 0x08);
                    }
                    else
                    {
                        REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] & 0xF7);
                    }

                    //C BIT = 0
                    if (check_bit_7 == 1 && ((REGISTER_FILE[L_REGISTER] >> 6)%2 == 0))
                    {
                        REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] | 0x01);
                    }
                    else
                    {
                        REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] & 0xFE);
                    }
                }                
            }
            else if (mid_opcode == 0x02)
            {
                if (low_opcode == 0x06)
                {
                    //SUB (HL)
                    //SET FLAGS
                    REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] | 0x02);
                    unsigned char check_bit_3 = (REGISTER_FILE[A_REGISTER] >> 2)%2;
                    unsigned char check_bit_7 = (REGISTER_FILE[A_REGISTER] >> 6)%2;

                    dispatchMemOp(HL_FULL_ADDRESS, DATA_BUS, 0);
                    REGISTER_FILE[A_REGISTER] -= *DATA_BUS;

                    //Z BIT = 6
                    if (REGISTER_FILE[A_REGISTER] == 0)
                    {
                        REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] | 0x40);
                    }
                    else
                    {
                        REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] & 0xBF);
                    }

                    //H BIT = 4
                    if (check_bit_3 == 0 && ((REGISTER_FILE[L_REGISTER] >> 2)%2 == 1))
                    {
                        REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] | 0x08);
                    }
                    else
                    {
                        REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] & 0xF7);
                    }

                    //C BIT = 0
                    if (check_bit_7 == 0 && ((REGISTER_FILE[L_REGISTER] >> 6)%2 == 1))
                    {
                        REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] | 0x01);
                    }
                    else
                    {
                        REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] & 0xFE);
                    }
                }
                else if (mid_opcode == 0x03)
                {
                    //SBC r
                    //SET FLAGS
                    REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] | 0x02);
                    unsigned char check_bit_3 = (REGISTER_FILE[A_REGISTER] >> 2)%2;
                    unsigned char check_bit_7 = (REGISTER_FILE[A_REGISTER] >> 6)%2;

                    REGISTER_FILE[A_REGISTER] -= (REGISTER_FILE[low_opcode] - (REGISTER_FILE[F_REGISTER]%2));

                    //Z BIT = 6
                    if (REGISTER_FILE[A_REGISTER] == 0)
                    {
                        REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] | 0x40);
                    }
                    else
                    {
                        REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] & 0xBF);
                    }

                    //H BIT = 4
                    if (check_bit_3 == 0 && ((REGISTER_FILE[L_REGISTER] >> 2)%2 == 1))
                    {
                        REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] | 0x08);
                    }
                    else
                    {
                        REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] & 0xF7);
                    }

                    //C BIT = 0
                    if (check_bit_7 == 0 && ((REGISTER_FILE[L_REGISTER] >> 6)%2 == 1))
                    {
                        REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] | 0x01);
                    }
                    else
                    {
                        REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] & 0xFE);
                    }
                }
                else
                {
                    //SUB r
                    //SET FLAGS
                    REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] | 0x02);
                    unsigned char check_bit_3 = (REGISTER_FILE[A_REGISTER] >> 2)%2;
                    unsigned char check_bit_7 = (REGISTER_FILE[A_REGISTER] >> 6)%2;

                    REGISTER_FILE[A_REGISTER] -= REGISTER_FILE[low_opcode];

                    //Z BIT = 6
                    if (REGISTER_FILE[A_REGISTER] == 0)
                    {
                        REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] | 0x40);
                    }
                    else
                    {
                        REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] & 0xBF);
                    }

                    //H BIT = 4
                    if (check_bit_3 == 0 && ((REGISTER_FILE[L_REGISTER] >> 2)%2 == 1))
                    {
                        REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] | 0x08);
                    }
                    else
                    {
                        REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] & 0xF7);
                    }

                    //C BIT = 0
                    if (check_bit_7 == 0 && ((REGISTER_FILE[L_REGISTER] >> 6)%2 == 1))
                    {
                        REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] | 0x01);
                    }
                    else
                    {
                        REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] & 0xFE);
                    }
                }                                
            }
            else
            {
                //ADD r
                //SET FLAGS
                REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] & 0xFD);
                unsigned char check_bit_3 = (REGISTER_FILE[A_REGISTER] >> 2)%2;
                unsigned char check_bit_7 = (REGISTER_FILE[A_REGISTER] >> 6)%2;

                REGISTER_FILE[A_REGISTER] += REGISTER_FILE[low_opcode];

                //Z BIT = 6
                if (REGISTER_FILE[A_REGISTER] == 0)
                {
                    REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] | 0x40);
                }
                else
                {
                    REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] & 0xBF);
                }

                //H BIT = 4
                if (check_bit_3 == 1 && ((REGISTER_FILE[L_REGISTER] >> 2)%2 == 0))
                {
                    REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] | 0x08);
                }
                else
                {
                    REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] & 0xF7);
                }

                //C BIT = 0
                if (check_bit_7 == 1 && ((REGISTER_FILE[L_REGISTER] >> 6)%2 == 0))
                {
                    REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] | 0x01);
                }
                else
                {
                    REGISTER_FILE[F_REGISTER] = (REGISTER_FILE[F_REGISTER] & 0xFE);
                }
            }
        }
    }

    //01
    if (high_opcode == 0x01)
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
        if ((low_opcode == 1) && (mid_opcode%2 == 0))
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
        else if (mid_opcode == 0x06)
        {
            if (low_opcode == 0x06)
            {
                //LD (HL), n
                //LOAD IMMEDIATE IN HL
                dispatchMemOp(PC+1, DATA_BUS, 0);
                dispatchMemOp(HL_FULL_ADDRESS, DATA_BUS, 1);
                PC++;
            }
            else if (low_opcode == 0x02)
            {
                //LD (HL-), A
                dispatchMemOp(HL_FULL_ADDRESS, &REGISTER_FILE[A_REGISTER], 1);
                HL_FULL_ADDRESS--;
                REGISTER_FILE[H_REGISTER] = (char)(HL_FULL_ADDRESS >> 8);
                REGISTER_FILE[L_REGISTER] = (char)HL_FULL_ADDRESS;
            }
        }
        else if (low_opcode == 0x06)
        {
            //LD r, n
            dispatchMemOp(PC+1, &REGISTER_FILE[mid_opcode], 0);
            PC++;
        }

        else if (mid_opcode == 0x05)
        {
            if (low_opcode == 0x02)
            {
                //LD A, (HL+)
                unsigned short int hl_register_value = HL_FULL_ADDRESS;
                REGISTER_FILE[A_REGISTER] = hl_register_value;
                hl_register_value++;
                REGISTER_FILE[H_REGISTER] = (hl_register_value >> 8);
                REGISTER_FILE[L_REGISTER] = (char)hl_register_value;
                
            }
        }

        else if (mid_opcode == 0x07)
        {
            if (low_opcode == 0x02)
            {
                //LD A, (HL-)
                REGISTER_FILE[A_REGISTER] = HL_FULL_ADDRESS;
                HL_FULL_ADDRESS--;
                REGISTER_FILE[H_REGISTER] = (HL_FULL_ADDRESS >> 8);
                REGISTER_FILE[L_REGISTER] = (char)HL_FULL_ADDRESS;
            }
        }
        else if (mid_opcode == 0x01)
        {
            if (low_opcode == 0x02)
            {
                //LD A, (BC)
                dispatchMemOp((REGISTER_FILE[B_REGISTER] << 8) | REGISTER_FILE[C_REGISTER], &REGISTER_FILE[A_REGISTER], 1);
            }
            else if (low_opcode == 0x00)
            {
                //LD (nn), SP
                dispatchMemOp(PC+1, &REGISTER_FILE[SP_LO_REGISTER], 1);
                dispatchMemOp(PC+2, &REGISTER_FILE[SP_HI_REGISTER], 1);
                PC += 2;
            }
        }
        else if (mid_opcode == 0x02)
        {
            if (low_opcode == 0x02)
            {
                //LD (DE), A
                dispatchMemOp((REGISTER_FILE[D_REGISTER] << 8) | REGISTER_FILE[E_REGISTER], &REGISTER_FILE[A_REGISTER], 1);
            }
        }
        else if (mid_opcode == 0x03)
        {
            if (low_opcode == 0x02)
            {
                //LD A, (DE)
                dispatchMemOp((REGISTER_FILE[D_REGISTER] << 8) | REGISTER_FILE[E_REGISTER], &REGISTER_FILE[A_REGISTER], 0);
            }
        }
        else if (mid_opcode == 0x04)
        {
            if (low_opcode == 0x02)
            {
                //LD (HL+), A
                dispatchMemOp(HL_FULL_ADDRESS, &REGISTER_FILE[A_REGISTER], 1);
                HL_FULL_ADDRESS++;
                REGISTER_FILE[H_REGISTER] = (char)(HL_FULL_ADDRESS >> 8);
                REGISTER_FILE[L_REGISTER] = (char)HL_FULL_ADDRESS;
            }
        }
        else if (mid_opcode == 0x00)
        {
            if (low_opcode == 0x02)
            {
                //LD (BC), A
                dispatchMemOp((REGISTER_FILE[B_REGISTER] << 8) | REGISTER_FILE[C_REGISTER], &REGISTER_FILE[A_REGISTER], 1);
            }
        }

    }
    else
    {
        if (high_opcode != 0 && mid_opcode != 0 && low_opcode != 0) {
            if (!seen_fail[((high_opcode << 8)| (mid_opcode << 3) | low_opcode)])
            {
                UNIQUE_FAILS++;
            }
            else
            {
                seen_fail[((high_opcode << 8)| (mid_opcode << 3) | low_opcode)] = true;
            }
            INSTRUCTION_FAILS++;
            std::cout << "Error! Instruction not recognized" << std::endl << std::flush;
        } else {
            NOP_COUNTER++;
        }
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


void loop()
{
    //TODO
    //while (true)
    while (INSTRUCTION_COUNTER < PROGRAM_SIZE)
    {
        fetch();
        execute();
        PC++;
    }
}

void boot(char* boot_rom_path, int boot_rom_size, char* rom_path)
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

    loop();

}

