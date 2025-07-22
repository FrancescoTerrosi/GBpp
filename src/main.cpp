#include "core/cpu.h"

int main(int argc, char** argv)
{

    const char* const_boot_rom_path = "./roms/dmg.bin\0";
    int default_path_bytes = 14;
    int boot_rom_size = 256;

    char* rom_path = new char[1024];
    char* boot_rom_path;

    //CHECK IF ROM PROVIDED

    if (argc > 1)
    {
        int counter = 0;
        while (argv[1][counter] != '\0')
        {
            counter++;
        }
        boot_rom_path = (char*)calloc(counter, counter*sizeof(char));
        memcpy(boot_rom_path, argv[1], counter);
        std::cout << boot_rom_path << std::endl;
    }
    if (argc > 2)
    {
        int counter = 0;
        while (argv[2][counter] != '\0')
        {
            rom_path[counter] = argv[2][counter];
            counter++;
        }
        std::cout << rom_path << std::endl;
    }
    if (argc == 1) {
        std::cout << "Attempting to find boot ROM in ./roms/" << std::endl;
        boot_rom_path = (char*)(const_boot_rom_path);
    }
    std::ifstream test(boot_rom_path);
    if (!test.good() || !(boot_rom_path))
    {
        std::cout << "Could not find boot ROM. Exiting program." << std::endl << std::endl;
    } else
    {
        std::cout << "Boot ROM found!" << std::endl << std::endl;
    }
    test.close();

    boot(boot_rom_path, boot_rom_size, rom_path);

    loop();

    std::cout << "Total Instructions\t\tNOP Instructions\t\tFailed Instructions\t\tInst/Failed\t\tInst/Failed (NO NOP)\t\tUInst\t\tUFailed\t\tUInst/UFailed" << std::endl;
    std::cout << INSTRUCTION_COUNTER << "\t\t\t\t" << NOP_COUNTER << "\t\t\t\t" << INSTRUCTION_FAILS << "\t\t\t\t" << ((float)INSTRUCTION_FAILS/(float)INSTRUCTION_COUNTER)*100 << "%\t\t" << (INSTRUCTION_FAILS/(float)(INSTRUCTION_COUNTER-NOP_COUNTER)*100) << "%\t\t\t" << UNIQUE_INST << "\t\t" << UNIQUE_FAILS << "\t\t" << ((float)UNIQUE_FAILS/(float)UNIQUE_INST)*100 << "%" << std::endl;

    return 0;

}


