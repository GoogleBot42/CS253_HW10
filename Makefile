# List of files
SRC     = main.cpp histogram.cpp image.cpp cluster.cpp readfile.cpp perceptron.cpp
OBJS    = main.o   histogram.o   image.o   cluster.o   readfile.o   perceptron.o
HEADERS =          histogram.h   image.h   cluster.h   readfile.h   perceptron.h   array.h typedef.h

EXE		= PA10

# Compiler and loader commands and flags
# -pthread -funroll-loops -funit-at-a-time -fbranch-target-load-optimize
CC		= g++
CC_FLAGS  = -Wall -O3 --std=c++14 -c -g -o
LD_FLAGS	= -Wall -O3 -pthread -g

# The first, and hence default, target is the executable
$(EXE): $(OBJS)
	$(CC) $(LD_FLAGS) $(OBJS) -o $(EXE)

# Recompile C objects if headers change
${OBJS}: ${HEADERS}

# Compile .cpp files to .o files
%.o : %.cpp
	$(CC) $(CC_FLAGS) $@ $<

# Clean up the directory
clean:
	rm -f $(OBJS) $(EXE) $(EXE).tar

# Package the directory
package:
	tar -cvf $(EXE).tar $(SRC) $(HEADERS) Makefile
