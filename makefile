# this section defines variables
# compiler exe
CC = clang
# compiler flags
CFLAGS = -g -O2 -std=c99 -pedantic -Wall
# all object files to link (src file name with .o extension)
OBJECTS = update_util.o update.o cutil.o
# program executable and main file name
PRGM = update
# libraries to link with
LIBS = -llibcrypto -llibssl

all: $(PRGM).exe

# compiles the main program
$(PRGM).exe : $(OBJECTS) *.h
	$(CC) $(CFLAGS) -o $(PRGM).exe $(OBJECTS) $(LIBS)
	
# % is a wildcard for compiling all files
# $< means "whatever the dependencies are"
# note that $@ means "the target"
# so, this produces object files from all source files based on their dependencies
%.o : %.c
	$(CC) $(CFLAGS) -c $<
	
clean:
	rm *.o
	