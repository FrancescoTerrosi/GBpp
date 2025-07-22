# Game Boy++

A research Game Boy Emulator project in C++



A project to showcase my ***OUTSTANDING*** skills with C++.

Nothing more, nothing less. Nespresso, what else?



- [ ] VIDEO HANDLING

- [ ] AUDIO HANDLING

- [ ] INSTRUCTIONS LEFT TO BOOT:    25  (9.77% of ROM's unique instructions)

- [ ] INSTRUCTIONS LEFT TO PLAY TETRIS:    3285    (9.95% of ROM's unique instructions)

- [ ] INSTRUCTIONS LEFT TO PLAY SUPER MARIO LAND:  20854   (31.71% of ROM's unique instructions)

- [ ] INSTRUCTIONS LEFT TO PLAY POKEMON BLUE:  75592   (7.21% of ROM's unique instructions)


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


