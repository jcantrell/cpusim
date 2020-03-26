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
four-byte chunks called 'tetras'. A tetra beginning with $0x98$ is a loader
command, the command is specified by the second byte of the tetra.
If a tetra is not a loader command, it is loaded into the current location,
and the current location is increased to the next higher multiple of 4.
The loader commands are now described.

lop_loc: X=0x00, YZ=1. Treat the next tetra as an ordinary tetrabyte, even if it
begins with the escape code.

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
unaryip -> "\neq"
binaryop -> "\Vec" | "\wedge" | "\rightarrow" | "\leftarrow"

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
