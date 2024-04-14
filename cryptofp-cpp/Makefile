CC  = g++
CXX = g++

INCLUDES =

CFLAGS   = -g -Wall $(INCLUDES)
CXXFLAGS = -g -Wall $(INCLUDES)

LDFLAGS = -g
LDLIBS = -lgmp -lcifer

.PHONY: executables
executables: main format

main: main.o fingerprint.o fp_funcs.o utils.o PRNG/prng.o crypto.o

main.o: main.cpp utils.cpp fingerprint.h fp_funcs.h utils.h crypto.h

fp_funcs.o: fp_funcs.cpp fp_funcs.h utils.h

fingerprint.o: fingerprint.cpp fingerprint.h utils.h PRNG/prng.h

PRNG/prng.o: PRNG/prng.c

crypto.o: crypto.c crypto.h
	gcc -g -Wall -c crypto.c -o crypto.o

.PHONY: format
format:
	clang-format -i -style=WebKit *.cpp *.h *.c

.PHONY: clean
clean:
	rm -f *.o PRNG/*.o a.out core main

.PHONY: all
all: clean executables

