#include <iostream>
#include <fstream>
#include <cstring>

int INSTRUCTION_FAILS = 0;

float CLOCK_FREQUENCY = 4.194304; //Mhz

//TRUE RAM
//int RAM_SIZE = 8*(1 << 10);

//TEST RAM
//int RAM_SIZE = 256;

//ONE RAM FOR EVERYTHING
int RAM_SIZE = 0xFFFF+1;

int VIDEO_RAM_SIZE = 8*(1 << 10);
int REGISTER_FILE_SIZE = 14;

unsigned char* RAM = new unsigned char[RAM_SIZE];
unsigned char* VIDEO_RAM = new unsigned char[VIDEO_RAM_SIZE];

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

int PC = 0;

unsigned char* INSTRUCTION_REGISTER = new unsigned char;
unsigned char* INTERRUPT_ENABLE = new unsigned char;

unsigned short* BUS = new unsigned short;

//RESOLUTION IN PIXELS
int SCREEN_WIDTH = 160;
int SCREEN_HEIGHT = 144;

int SPRITE_SIZE = 8;

//BUS MEMORY MAP
//16-bit BUS
int ROM_BANK_00_ADDRESS_START = 0;
int ROM_BANK_00_ADDRESS_END = 0x3FFF;

int ROM_BANK_01_ADDRESS_START = 0x4000;
int ROM_BANK_01_ADDRESS_END = 0x7FFF;

int VIDEO_RAM_ADDRESS_START = 0x8000;
int VIDEO_RAM_ADDRESS_END = 0x9FFF;

int EXTERNAL_RAM_ADDRESS_START = 0xA000;
int EXTERNAL_RAM_ADDRESS_END = 0xBFFF;

int WRAM_BANK_0_ADDRESS_START = 0xC000;
int WRAM_BANK_0_ADDRESS_END = 0xCFFF;

int WRAM_BANK_1_ADDRESS_START = 0xD000;
int WRAM_BANK_1_ADDRESS_END = 0xDFFF;

int WRAM_BANK_0_ECHO_ADDRESS_START = 0xE000;
int WRAM_BANK_0_ECHO_ADDRESS_END = 0xFDFF;

int SPRITE_ATTRIBUTE_TABLE_ADDRESS_START = 0xFE00;
int SPRITE_ATTRIBUTE_TABLE_ADDRESS_END = 0xFE9F;

int NOT_USABLE_ADDRESS_START = 0xFEA0;
int NOT_USABLE_ADDRESS_END = 0xFEFF;

int IO_PORTS_ADDRESS_START = 0xFF00;
int IO_PORTS_ADDRESS_END = 0xFF7F;

int HIGH_RAM_ADDRESS_START = 0xFF80;
int HIGH_RAM_ADDRESS_END = 0xFFFE;

int INTERRUPT_ENABLE_REGISTER_ADDRESS = 0xFFFF;

void parseBytes(char* bytes, int bytes_length)
{

    unsigned char bytes_copy[bytes_length];
    memcpy(&bytes_copy, bytes, bytes_length);

    char current_byte[8];
    int counter = 0;
    

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

        std::cout << current_byte << std::endl;
        
    }

}

void handleBusyClock(bool* cpu_busy, int* cpu_busy_counter, int cycles)
{
    if (*cpu_busy == false) {
        *cpu_busy = true;
        *cpu_busy_counter = cycles;
    } else if (*cpu_busy == true && *cpu_busy_counter == 0) {
        *cpu_busy = false;
    }

}

void fetch()
{
    *INSTRUCTION_REGISTER = RAM[PC];

    std::cout << PC << ": " << "RETRIEVED FUNC - ";
    parseBytes((char *)INSTRUCTION_REGISTER, 1);
}

void execute()
{

    unsigned char instruction = *INSTRUCTION_REGISTER;
    unsigned char data = (unsigned char)*BUS;

    unsigned char high_opcode = (instruction & 0xC0) >> 6;
    unsigned char mid_opcode = (instruction & 0b111000) >> 3;
    unsigned char low_opcode = (instruction & 0b111);
    unsigned short SP_FULL_ADDRESS = ((RAM[REGISTER_FILE[SP_HI_REGISTER]] << 8) | (RAM[REGISTER_FILE[SP_LO_REGISTER]]));
    unsigned short HL_FULL_ADDRESS = ((REGISTER_FILE[H_REGISTER] << 8) | REGISTER_FILE[L_REGISTER]);

    //11
    if (high_opcode == 0x03)
    {
        if (low_opcode == 0x02 && mid_opcode%2 == 0)
        {
            //PUSH rr
            unsigned char select_reg = (mid_opcode >> 1);
            SP_FULL_ADDRESS--;
            RAM[REGISTER_FILE[SP_FULL_ADDRESS]];
            switch (select_reg)
            {
                case 0:
                    RAM[SP_FULL_ADDRESS] = REGISTER_FILE[B_REGISTER];
                    SP_FULL_ADDRESS--;
                    RAM[SP_FULL_ADDRESS] = REGISTER_FILE[C_REGISTER];
                break;
                case 1:
                    RAM[SP_FULL_ADDRESS] = REGISTER_FILE[D_REGISTER];
                    SP_FULL_ADDRESS--;
                    RAM[SP_FULL_ADDRESS] = REGISTER_FILE[E_REGISTER];
                break;

                case 2:
                    RAM[SP_FULL_ADDRESS] = REGISTER_FILE[H_REGISTER];
                    SP_FULL_ADDRESS--;
                    RAM[SP_FULL_ADDRESS] = REGISTER_FILE[L_REGISTER];
                break;

                case 3:
                    RAM[SP_FULL_ADDRESS] = REGISTER_FILE[A_REGISTER];
                    SP_FULL_ADDRESS--;
                    RAM[SP_FULL_ADDRESS] = REGISTER_FILE[F_REGISTER];
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
                    RAM[SP_FULL_ADDRESS] = REGISTER_FILE[B_REGISTER];
                    SP_FULL_ADDRESS++;
                    RAM[SP_FULL_ADDRESS] = REGISTER_FILE[C_REGISTER];
                break;
                case 1:
                    RAM[SP_FULL_ADDRESS] = REGISTER_FILE[D_REGISTER];
                    SP_FULL_ADDRESS++;
                    RAM[SP_FULL_ADDRESS] = REGISTER_FILE[E_REGISTER];
                break;

                case 2:
                    RAM[SP_FULL_ADDRESS] = REGISTER_FILE[H_REGISTER];
                    SP_FULL_ADDRESS++;
                    RAM[SP_FULL_ADDRESS] = REGISTER_FILE[L_REGISTER];
                break;

                case 3:
                    RAM[SP_FULL_ADDRESS] = REGISTER_FILE[A_REGISTER];
                    SP_FULL_ADDRESS++;
                    RAM[SP_FULL_ADDRESS] = REGISTER_FILE[F_REGISTER];
                break;
            }
            SP_FULL_ADDRESS++;
        }
        else if (mid_opcode == 0x06)
        {
            if (low_opcode == 0x00)
            {
                //LDH A, (n)
                REGISTER_FILE[A_REGISTER] = (0xFF | RAM[PC+1]);
                PC++;
            }
            else if (low_opcode == 0x02)
            {
                //LDH A, (C)
                REGISTER_FILE[A_REGISTER] = RAM[(0xFF | REGISTER_FILE[C_REGISTER])];
            }
        }
        else if (mid_opcode == 0x04)
        {
            if (low_opcode == 0x00)
            {
                //LDH (n), A
                RAM[PC+1] = REGISTER_FILE[A_REGISTER];
                PC++;
            }
            else if (low_opcode == 0x02)
            {
                //LDH (C), A
                RAM[(0xFF | REGISTER_FILE[C_REGISTER])] = REGISTER_FILE[A_REGISTER];
            }
        } 
        else if (mid_opcode == 0x07)
        {
            if (low_opcode == 0x02)
            {
                //LD A, (nn)
                REGISTER_FILE[A_REGISTER] = RAM[((PC+2) << 8) | (PC+1)]; 
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
                signed char operand_e = RAM[PC+1];
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
                RAM[HL_FULL_ADDRESS] = RAM[((PC+2) << 8) | (PC+1)];
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
                signed char operand_immediate = RAM[PC+1];
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
                signed char operand_immediate = RAM[PC+1];
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
                signed char operand_immediate = RAM[PC+1];
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

                REGISTER_FILE[A_REGISTER] += RAM[REGISTER_FILE[HL_FULL_ADDRESS]];

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

                    REGISTER_FILE[A_REGISTER] += RAM[HL_FULL_ADDRESS] + (REGISTER_FILE[F_REGISTER]%2);

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

                    REGISTER_FILE[A_REGISTER] -= RAM[HL_FULL_ADDRESS];

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
            RAM[HL_FULL_ADDRESS] = REGISTER_FILE[low_opcode];
        }
        else if (low_opcode == 0x06)
        {
            //LD r, (HL)
            REGISTER_FILE[mid_opcode] = RAM[HL_FULL_ADDRESS];
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
        if (low_opcode == 1 && mid_opcode%2 != 1)
        {
            //LD rr, nn
            unsigned char select_reg = mid_opcode >> 1;
            unsigned short data = RAM[((PC+2) << 8) | PC+1];
            PC++;
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
        if (mid_opcode == 0x06)
        {
            if (low_opcode == 0x06)
            {
                //LD (HL), n
                //LOAD IMMEDIATE IN HL
                RAM[HL_FULL_ADDRESS] = RAM[PC+1];
                PC++;
            }
            else if (low_opcode == 0x02)
            {
                //LD (HL-), A
                unsigned short int hl_register_value = HL_FULL_ADDRESS;
                RAM[hl_register_value] = REGISTER_FILE[A_REGISTER];
                hl_register_value--;
                REGISTER_FILE[H_REGISTER] = (char)(hl_register_value >> 8);
                REGISTER_FILE[L_REGISTER] = (char)hl_register_value;
            }
        }
        else if (low_opcode == 0x06)
        {
            //LD r, n
            REGISTER_FILE[mid_opcode] = RAM[PC+1];
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
                unsigned short int hl_register_value = HL_FULL_ADDRESS;
                REGISTER_FILE[A_REGISTER] = hl_register_value;
                hl_register_value--;
                REGISTER_FILE[H_REGISTER] = (hl_register_value >> 8);
                REGISTER_FILE[L_REGISTER] = (char)hl_register_value;
            }
        }
        else if (mid_opcode == 0x01)
        {
            if (low_opcode == 0x02)
            {
                //LD A, (BC)
                REGISTER_FILE[A_REGISTER] = RAM[(REGISTER_FILE[B_REGISTER] << 8) | REGISTER_FILE[C_REGISTER]];
            }
            else if (low_opcode == 0x00)
            {
                //LD (nn), SP
                RAM[(PC+1)] = REGISTER_FILE[SP_LO_REGISTER];
                RAM[(PC+2)] = REGISTER_FILE[SP_HI_REGISTER];
                PC += 2;
            }
        }
        else if (mid_opcode == 0x02)
        {
            if (low_opcode == 0x02)
            {
                //LD (DE), A
                RAM[(REGISTER_FILE[D_REGISTER] << 8) | REGISTER_FILE[E_REGISTER]];
            }
        }
        else if (mid_opcode == 0x03)
        {
            if (low_opcode == 0x02)
            {
                //LD A, (DE)
                REGISTER_FILE[A_REGISTER] = RAM[(REGISTER_FILE[D_REGISTER] << 8) | REGISTER_FILE[E_REGISTER]];
            }
        }
        else if (mid_opcode == 0x04)
        {
            if (low_opcode == 0x02)
            {
                //LD (HL+), A
                unsigned short int hl_register_value = HL_FULL_ADDRESS;
                RAM[REGISTER_FILE[hl_register_value]] = REGISTER_FILE[A_REGISTER];
                hl_register_value++;
                REGISTER_FILE[H_REGISTER] = (char)(hl_register_value >> 8);
                REGISTER_FILE[L_REGISTER] = (char)hl_register_value;
            }
        }
        else if (mid_opcode == 0x00)
        {
            if (low_opcode == 0x02)
            {
                //LD (BC), A
                RAM[(REGISTER_FILE[B_REGISTER] << 8) | REGISTER_FILE[C_REGISTER]] = REGISTER_FILE[A_REGISTER];
            }
        }

    }
    else
    {
        if (high_opcode != 0 && mid_opcode != 0 && low_opcode != 0) {
            std::cout << "Error! Instruction not recognized" << std::endl;
            INSTRUCTION_FAILS++;
        }
    }

}


void loop()
{

    while (PC < RAM_SIZE)
    {
        fetch();
        execute();
        PC++;
    }
}

int main(int argc, char** argv)
{

    char* file_path = new char;

    if (argc < 1) {
        std::cout << "Provide a boot ROM path!!!" << std::endl;
        return 1;
    } else {
        file_path = argv[1];
        file_path = "./roms/dmg-bin";
        std::ifstream gameBoyBootRom(file_path);
        char current_char = 0;
        int bytes_length = 0;

        char* bytes = new char[8*1024];

        while (gameBoyBootRom.get(current_char))
        {
            bytes[bytes_length] = current_char;
            bytes_length++;
        }

        gameBoyBootRom.close();

        parseBytes(bytes, bytes_length);

        for (int i = 0; i < bytes_length; i++)
        {
            RAM[i] = bytes[i];
        }

        std::cout << "PROGRAM SIZE: " << bytes_length << " BYTES" << std::endl;
        

        loop();

        delete[] bytes;
        
        std::cout << "Instructions failed: " << INSTRUCTION_FAILS <<std::endl;

        parseBytes((char *)REGISTER_FILE, REGISTER_FILE_SIZE);

    }

    return 0;

}

