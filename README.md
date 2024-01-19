# Own-Shell

## Overview
Implemented the simplest functionalities of a standard shell in C.
The shell serves as a command-line interface, allowing users to interact with the underlying operating system.

## Standard commands implemented
cd, mkdir, pwd, touch, grep, clear, history, ls, exit, cp, rm, rmdir

## Extra
It can also withstand logical expressions such as && and ||.

Last but not least, the pipe ">" can also be used.

## Requirements to run/test the project
You have to install the package lreadline:
```bash
sudo apt-get  install libreadline-dev
```
To run the code you have to write this line of code:
```bash
gcc main.c -o main -lreadline
./main
