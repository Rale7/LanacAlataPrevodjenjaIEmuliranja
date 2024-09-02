# Tool for compiling and emulating

## Platform

U need to run this project on Linux operating system, and required tools
for compilation are gcc, make, flex and bison.

If you have all this tools just type

```
make all
```

Then there are some test files that use bash scripts, that will run some of the tests.
If you want to see it just navigate to some test folder and type

```
./start.sh
```

## Purpose

This project has 3 main parts:

- ### Assembly compiler
- ### Linker
- ### Emulator

## Assembly compiler

It will create relocatable ELF file, with relocation entries,
symbol table and machine language code. You can check every of these
section with readelf command

## Linker

It will take relocatable ELF files and create bigger relocatable file if u
give it option -relocatable, or executable file if u give option -hex. Executable
file contains segments, that emulator will load and run

## Emulator

It takes one executable file, load all memory segments, and then it starts emulating CPU.
It also support emulation of terminal and timer.

> [!NOTE]
> ELF files that are generated with assembly compiler and linker are binary files, but besides them
> there are also txt files that are human readable.
