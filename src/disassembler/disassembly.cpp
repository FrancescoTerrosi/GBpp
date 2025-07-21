#include <iostream>
#include <fstream>
#include <cstring>

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

int main(int argc, char** argv)
{

    const char* default_boot_path = "./roms/dmg.bin";
    const int default_path_bytes = 14;

    char* file_path = new char[1024];

    if (argc < 2) {
        std::cout << "Using default ROM dmg.bin" << std::endl;
        memcpy(file_path, default_boot_path, default_path_bytes);
    } else {
        file_path = argv[1];
    }

    std::ifstream gameBoyBootRom(file_path);
    char current_char = 0;
    int bytes_length = 0;

    char* bytes = new char[100*1024];

    while (gameBoyBootRom.get(current_char))
    {
        bytes[bytes_length] = current_char;
        bytes_length++; 
    }

    gameBoyBootRom.close();

    parseBytes(bytes, bytes_length);

    std::cout << "PROGRAM SIZE: " << bytes_length << " BYTES" << std::endl;

    return 0;
}

