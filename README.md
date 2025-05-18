# Math Compiler

A simple compiler that converts mathematical expressions in Reverse Polish Notation (RPN) into x86-64 assembly language.

## Features

- Full RPN input support
- Floating-point operations
- Multiple operators: +, -, *, /, ^, %, !, abs, sin, cos, tan, sqrt
- Stack operations: swap, dup
- Built-in constants: e, pi
- Error handling: division by zero, stack underflow

## Building the Project

### Using CMAKE (Recommended)

```bash
# Create a build directory
mkdir build
cd build

# Configure and build
cmake ..
cmake --build .

# The executable will be in bin/math-compiler
```

### Using Make on Windows

```bash
# Build the project
make

# This creates math-compiler.exe in the current directory
```

## Usage

```bash
# Compile an expression directly
math-compiler "3 4 +"

# Compile from a file
math-compiler -f input.txt

# Specify an output file
math-compiler "3 4 +" output.asm
math-compiler -f input.txt output.asm
```

## Expression Syntax

The compiler uses Reverse Polish Notation (RPN) where operators follow their operands.

### Examples

- Addition: `3 4 +` (3 + 4 = 7)
- Subtraction: `5 2 -` (5 - 2 = 3)
- Multiplication: `3 4 *` (3 * 4 = 12)
- Division: `10 2 /` (10 / 2 = 5)
- Power: `2 3 ^` (2^3 = 8)
- Modulus: `5 2 %` (5 % 2 = 1)
- Factorial: `5 !` (5! = 120)
- Mixed operations: `3 4 + 2 *` ((3 + 4) * 2 = 14)
- Constants: `pi 2 *` (π * 2 ≈ 6.28)
- Functions: `pi 2 / sin` (sin(π/2) = 1)

### Supported Operators and Functions

- `+` - Addition (2 operands)
- `-` - Subtraction (2 operands)
- `*` - Multiplication (2 operands)
- `/` - Division (2 operands)
- `^` - Power (2 operands)
- `%` - Modulus (2 operands)
- `!` - Factorial (1 operand)
- `abs` - Absolute value (1 operand)
- `sin` - Sine (1 operand)
- `cos` - Cosine (1 operand)
- `tan` - Tangent (1 operand)
- `sqrt` - Square root (1 operand)
- `swap` - Swap top two stack items (2 operands)
- `dup` - Duplicate top stack item (1 operand)

### Constants

- `pi` - The constant π (3.14159...)
- `e` - The constant e (2.71828...)

## Generated Assembly

The compiler generates x86-64 assembly that can be assembled using NASM:

```bash
# Assemble the generated code
nasm -f elf64 output.asm

# Link the object file (requires the C standard library)
gcc -no-pie -o math_program output.o -lm

# Run the resulting program
./math_program
``` 