## you have command.cc command.h tokenizer.cc tokenizer.h you need compile them and link
## them to one object file called  : "myshell" 
## so we should run make , that will create our object myshell ,
## ./myshell should run your application 

CXX := g++

# Set C++ version and warning outputs
CXXFLAGS := -std=c++17 -Wall -Wextra -g

# List of source files
SRCS := command.cc tokenizer.cc

# Replace every .cc extension with .o to differentiate objects from source
OBJS := $(SRCS:.cc=.o)

# Executable
TARGET := myshell

.DEFAULT_GOAL := all

all: $(TARGET) 

# $^ ensures all pre-requisites are met
$(TARGET) : $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile each .cc into a .o
%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Header dependencies
command.o:   command.h tokenizer.h
tokenizer.o: tokenizer.h

.PHONY: clean
clean: ## Clean up compiled files
	rm -f $(OBJS) $(TARGET)

.PHONY: rebuild
rebuild: ## Rebuild executable
	clean all