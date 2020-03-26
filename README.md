# cpusim

This project aims to imlement an emulator for the MMIX computer described
in the book *The Art of Computer Programming*

## Table of Contents

- [Install](#install)
- [Usage](#usage)

## Install

```sh
```

## Usage

```sh
```

### Commands

create
:  Creates a new virtual cpu

memdump
:  prints contents of memory

exit
:  exits simulator

load
:  load a value into memory address

view
:  view the contents of a given memory address

viewip
:  view the instruction pointer

setip
:  set the instruction pointer

step
:  step through one instruction

## Loader

The loader uses MMIX loader format. In this format, a file is divided into
four-byte chunks called 'tetras'. A tetra beginning with $0x98$ is a loader
command, the command is specified by the second byte of the tetra.
If a tetra is not a loader command, it is loaded into the current location,
and the current location is increased to the next higher multiple of 4.
The loader commands are now described.

lop_loc: X=0x00, YZ=1. Treat the next tetra as an ordinary tetrabyte, even if it
begins with the escape code.
