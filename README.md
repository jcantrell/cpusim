# cpusim

This project aims to imlement an emulator for the MMIX computer described
in the book *The Art of Computer Programming*

## Table of Contents

- [Install](#install)
- [Usage](#usage)
- [Notes](#Notes)
- [Todo](#todo)

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

## Notes
### Loader

The loader uses MMIX loader format. In this format, a file is divided into
four-byte chunks called 'tetras'. A tetra beginning with **0x98** is a 
loader command, the command is specified by the second byte of the tetra.
If a tetra is not a loader command, it is loaded into the current location,
and the current location is increased to the next higher multiple of 4.
The loader commands are now described.

`lop_loc`: X=0x00, YZ=1. Treat the next tetra as an ordinary tetrabyte,
even if it begins with the escape code.

`lop_fixo`: load value of location $\lambda$ into octabyte P, where P is
64-bit address defined by next Z tetras plus $2^56 y$. Y is the high byte.

`lop_fixrx`: x=0x05, y=0, z=16 or 24
load delta into tetrabytes $P = \lambda -4 \delta$ where $\delta$ is the tetrabye
following the `lop_fixrx` instruction.

 $\delta = (tet \ge 0x1000000 ? (tet \& 0xFFFFFF) - (1 << j) : tet )$

`mmo_load(incr(cur_loc, -` $\delta$ `<<2), tet)`

### Mem & Register Status after loading hello.mmo
| Address | Value |
| ------- |:-----:|
| 0x2000 0000 0000 0000 | 4865 6c6c |
| | 6F20 776F |
| | 726c 6421 |
| | 0A00 0000 |
| 0x0000 0000 0000 0100 | 23FF FE00 |
| | 0000 0701 |
| | 0000 0000 |

254 = 0x2000 0000 0000 0000
255 = 0x0000 0000 0100 0000

`lop_pre` 1 1 mmo version 1, (tetra meta info (time))

`lop_loc` 20 1 set address to next Z tetras+2^56 x Y

`lop_file` 0 3 next 3 tetras name file #0

`lop_line` 0 7 set line number to 7

`lop_post` 0 FE G=254, rG=254, next (256-G)x2 = 4 tetras define 254, 255

254 = 0x2000 0000 0000 0000

255 = 0x0000 0000 0000 0100

`lop_stab` 0 0 begin symbol table

`lop_end` 0 8 symbol table is 0x08 tetras

### hello.mmo sample listing
| Number | Code | Comments |
|:-:|:-:|:-:|
| 1 | 9809 0101 | lop_pre,1,1 |
| 2 | 5d09 bccd | creation time |
| 3 | 9801 2001 | lop_loc,0x20,1 |
| 4 | 0000 0000 | |
| 5 | 4865 6c6c | program code |
| 6 | 6f20 776f | program code | 
| 7 | 726c 6421 | program code |
| 8 | 0a00 0000 | program code |
| 9 | 9801 0001 | lop_loc,0x00,1 |
| 10 | 0000 0100 | |
| 11 | 9806 0003 | lop_file,00,3 |
| 12 | 6865 6c6c | filename |
| 13 | 6f2e 6d6d | "hello.mmo" |
| 14 | 7300 0000 | filename |
| 15 | 9807 0007 | lop_line,0,7 |
| 16 | 23ff fe00 | program code |
| 17 | 0000 0701 | program code |
| 18 | 0000 0000 | program code |
| 19 | 980a 00fe | lop_post,G=254 |
| 20 | 2000 0000 | |
| 21 | 0000 0000 | |
| 22 | 0000 0000 | |
| 23 | 0000 0100 | |
| 24 | 980b 0000 | lop_stab |
| 25 | 203a 4050 | |
| 26 | 1040 4020 | |
| 27 | 4d20 6120 | |
| 28 | 6902 6e01 | |
| 29 | 0081 1040 | |
| 30 | 4020 5420 | |
| 31 | 6520 7809 | |
| 32 | 7400 8200 | |
| 33 | 980c 0008 | lop_end, 8 tetras |

### test.mmo listing
| Number | Code | Comments |
|:-:|:-|:-:|
| 1 | 9809 0101 5de9 484a | |
| 2 | 9801 2001 0000 0000 | |
| 3 | 0000 0000 | |
| 4 | 0000 0000 | |
| 5 | 6162 0000 | |
| 6 | 9801 0002 0000 0001 2345 678c | |
| 7 | 9806 0002 7465 7374 2e6d 6d73 | |
| 8 | 9807 0006 | line number = 6 |
| 9 | f000 0000 | |
| 10 | 9802 4000 | $\lambda += 4000$ |
| 11 | 9807 0008 | line number = 8 |
| 12 | 8103 fe01 | |
| 13 | 4203 0000 | |
| 14 | 9807 0009 | line number = 9 |
| 15 | 0000 0000 | |
| 16 | 9801 0002 0000 0001 2345 a768 | |
| 17 | 9805 0010 0100 fff5 | |
| 18 | 9804 0ff7 | |
| 19 | 9803 2001 0000 0000 | |
| 20 | 9806 0102 666f 6f2e 6d6d 7300 | |
| 21 | 9807 0004 | line number = 4 |
| 22 | f000 000a | |
| 23 | 9808 0005 0000 0200 00fe 0000 | |
| 24 | 9801 2001 0000 000a | |
| 25 | 0000 6364 | |
| 26 | 9800 0001 9800 0000 | |
| 27 | 980a 00fe | |
| 28 | 2000 0000 0000 0008 | |
| 29 | 0000 0001 2345 678c | |
| 30 | 980b 0000 | |
| 31 | 203a 5040 5040 4020 | |
| 32 | 4120 4220 4309 4408 | |
| 33 | 8340 4020 4d20 6120 | |
| 34 | 6905 6e01 2345 678c | |
| 35 | 8140 0f61 fe82 0000 | |
| 36 | 980c 000a | |

#### State after running test.mmo
| Address | Contents |
|:-|:-:|
| 2000 0000 0000 0000 | 0000 0000 |
| 2000 0000 0000 0004 | 0000 0000 |
| 2000 0000 0000 0008 | 6162 0000 |
| 0000 0001 2345 678c | f000 0000 |
|                a790 | 8103 fe01 |
|                a794 | 4203 0000 |
|                a798 | 8100 0000 |
|                a794 | 0100 fff5 |
|                678c | 0000 0ff7 |
| 2000 0000 0000 0000 | 0000 0001 2345 a768 |
|                a768 | f000 000a |
| 2000 0000 0000 000a | 0000 6364 |
|                   c | 9800 0000 | 
g254 = 2000 0000 0000 0008
g255 = 0000 0001 2345 678c

### Misc

dump memory n
store memory n m
dump register n
store register n

word:
4 bit ip
20 bit address
20 bit address
20 bit address

dump(), store(), memory(), register(), address(), value()

dump_statement -> dump memory address | dump register address
store_statement -> store memory address value
    | store register address value
statement -> dump_statement | store_statement

logic:  constants   T,F
        propositions    lowercase & digit string
        connectives ~, &, V, ->, <->

formula ->  constant | proposition | unary_formula | binary_formula
constant -> "T" | "F"
proposition -> [a-zA-Z0-9]+
UnaryFormula -> leftparen unaryop formula rightparen
binaryFormula -> leftparen binaryop formula rightparen
leftparen -> "("
rightparen -> ")"
unaryip -> "$\neq$"
binop ->  "$\wedge$" | "$\rightarrow$" | "$\leftarrow$"

Morsel:
always unsigned

token types:
	"load"
	"store"
	hex values
	identifiers

parse tree nodes:
	statement_p
	load_p
	store_p
	token_p

## TODO

- [x] memdump is slow - clean up code and speed up
- [x] write tests for Loader
- [ ] write tests for subleq
- [ ] write tests for mmix
- [x] lop_fixrx currently functions similarly to other simulators, but the output is wrong compared to mmotype\
- [ ] convert uses of Addresss and UnsignedMorsel into uses of SignedMorsel
- [ ] Check that loading test.mmo properly sets registers
- [ ] Load and run hello.mmo
- [ ] Load and run test.mmo
