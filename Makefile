
 #============================================================================
 # Name        : Makefile
 # Author      : Maleakhi Agung Wijaya <maleakhiw>
 # StudentID   : 784091
 # Description : Makefile for 2048 solver
 #============================================================================
 
CC = gcc

# CPPFLAGS are used for debugging, which will be submitted
CPPFLAGS = -Wall  -Werror  -g -lm

# CPPOPTIMISE are used for experimentation part
CPPOPTIMISE = -Wall -Werror -O3 -lm

SRC = src/utils.o src/priority_queue.o src/ai.o src/2048.o 
TARGET = 2048

# Tell dependency so that make will compile the .c file automatically
all: $(SRC)
	$(CC) -o $(TARGET) $(SRC) $(CPPFLAGS)

# Used to clean all of the .o files and executable
clean:
	rm -f $(TARGET) src/*.o *.txt
