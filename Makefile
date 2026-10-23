## you have command.cc command.h tokenizer.cc tokenizer.h you need compile them and link
## them to one object file called  : "myshell" 
## so we should run make , that will create our object myshell ,
## ./myshell should run your application 

CXX := g++

# Set C++ version and warning outputs
# Flags:
#	-Wall > enables all warnings flags
#   -Wextra > prints additional warnings not covered by -Wall
# 	-g > debugging information
CXXFLAGS := -std=c++17 -Wall -Wextra -g

# List of source files
SRCS := command.cc tokenizer.cc

# List of object files
OBJS := command.o tokenizer.o

# Executable
TARGET := myshell

.DEFAULT_GOAL := all

all: $(TARGET) 

# Header dependencies
command.o:   command.h tokenizer.h
tokenizer.o: tokenizer.h

# Referenced from: https://www.geeksforgeeks.org/cpp/makefile-in-c-and-its-applications/
# $^ ensures all pre-requisites are met
# Creates command like: g++ CXXFLAGS -o myshell command.o tokenizer.o
# To link object files with executable
$(TARGET) : $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile each .cc into a .o
%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean
clean: ## Clean up compiled files
	rm -f $(OBJS) $(TARGET)

.PHONY: rebuild
rebuild: ## Rebuild executable
	clean all