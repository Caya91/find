SRC = find.cc 
OBJ = $(SRC:.cc=.o)
TGT = find
DEP = $(SRC:.cc=.d)
CXXFLAGS = -std=c++20


all: $(TGT) directory xdev

$(TGT): $(OBJ)
	g++ -std=c++20 -o $@ $+

directory: directory.o
	g++ -std=c++20 -o $@ $+

xdev : xdev.o
	g++ -std=c++20 -o $@ $+

# the following lines cause errors with argparse.h
%.o: %.c Makefile
	g++ -std=c++20 -Wall -Werror -MMD -c -o $@ $<


clean:
	rm -f $(OBJ) $(TGT) $(DEP) directory directory.o xdev xdev.o



-include $(DEP)


