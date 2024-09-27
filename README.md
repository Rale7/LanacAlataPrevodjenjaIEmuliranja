# Tool for compiling and emulating

## Platform

You need to run this project on the Linux operating system and the required tools.
For compilation are GCC, Make, Flex, and Bison.

If you have all these tools, just type

```
make all
```

Then there are some test files that use bash scripts that will run some of the tests.
If you want to see it, just navigate to some test folder and type

```
./start.sh
```

## Purpose

This project has 3 main parts:

### Assembly compiler
### Linker
### Emulator

## Assembly compiler

It will create a relocatable ELF file with relocation entries.
symbol table and machine language code. You can check every one of these.
section with readelf command

## Linker

It will take relocatable ELF files and create a bigger relocatable file if
Give it option -relocatable, or executable file if you give option -hex. Executable
file contains segments that the emulator will load and run.

## Emulator

It takes one executable file, loads all memory segments, and then it starts emulating CPU.
It also supports emulation of terminals and timers.

> [!NOTE]
ELF files that are generated with the assembly compiler and linker are binary files, but besides them
Tools are also generating text files that are human-readable.
