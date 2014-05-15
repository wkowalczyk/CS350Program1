# Specify the compiler
GXX=gcc -g -Wall -lrt -lpthread

# Specify the target
all: program1

# Specify the object files that the target depends on
# Also specify the object files needed to create the executable
program1: program1.o timing.o
	$(GXX) program1.o timing.o -o program1

# Specify how the object files should be created from source files
program1.o: program1.c
	$(GXX)  -c  program1.c -o program1.o

timing.o: timing.c
	$(GXX)  -c  timing.c -o timing.o

# Specify the object files and executables that are generated
# and need to be removed to re-compile the whole thing
clean:
	rm -f *.o *~ core program1

