#!/bin/bash

# Ensure the script exits if any command fails
set -e

# Perform make clean
make clean

# Perform make
make

# Run the compiled program
./ToDoList


# chmod +x todolist.sh ------ To make file executable
# ./todolist.sh