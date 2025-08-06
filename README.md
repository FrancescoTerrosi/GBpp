# Game Boy++

A research Game Boy Emulator project in C++



A project to showcase my ***OUTSTANDING*** skills with C++.

Nothing more, nothing less. Nespresso, what else?

# ONGOING

1. [x] BOOTING, HOORAY!

2. [ ] DISPLAY LOGO

3. [ ] PLAY "TA-DING!"


# TODO

- Code refactoring and reorganization when first 3 steps are completed (e.g., use classes)


# HOW TO INSTALL

```
$ sudo apt update;

$ sudo apt install g++ cmake git;

$ git clone git@github.com:FrancescoTerrosi/GBpp.git;

$ cd GBpp;

$ mkdir build;

$ cd build;

$ cmake ../

$ cmake --build ./

#ATTEMPTS TO FIND BOOT ROM IN ./roms/
$ ./gbpp

#ATTEMPTS TO FIND BOOT ROM IN {BOOT_ROM_PATH}

$ ./gbbpp {BOOT_ROM_PATH}

#ATTEMPTS TO FIND BOOT ROM IN {BOOT_ROM_PATH} AND ROM IN {ROM_PATH}
$ ./gbpp {BOOT_ROM_PATH} {ROM_PATH} 

```

##REFS

- [1] https://ia803208.us.archive.org/9/items/GameBoyProgManVer1.1/GameBoyProgManVer1.1.pdf
- [2] https://gbdev.io/pandocs/
- [3] https://gekkio.fi/files/gb-docs/gbctr.pdf
- [4] https://www.zilog.com/docs/z80/um0080.pdf


