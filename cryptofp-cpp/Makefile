CC  = g++
CXX = g++

INCLUDES =

CFLAGS   = -g -Wall $(INCLUDES)
CXXFLAGS = -g -Wall $(INCLUDES)

LDFLAGS = -g
LDLIBS =

.PHONY: all
executables: main example

main: main.o fingerprint.o fp_funcs.o utils.o

main.o: main.cpp utils.cpp fingerprint.h fp_funcs.h utils.h

fp_funcs.o: fp_funcs.cpp fp_funcs.h utils.h

fingerprint.o: fingerprint.cpp fingerprint.h utils.h

example: example.c
	gcc example.c -o example -lgmp -lcifer

.PHONY: clean
clean:
	rm -f *.o a.out core main example

.PHONY: all
all: clean main

