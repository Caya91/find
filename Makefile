SRC = find.cc 
OBJ = $(SRC:.cc=.o)
TGT = find
DEP = $(SRC:.cc=.d)

all: $(TGT) directory

$(TGT): $(OBJ)
	g++ -std=c++20 -o $@ $+

directory: directory.o
	g++ -std=c++20 -o $@ $+

%.o: %.c Makefile
	g++ -std=c++20 -Wall -Werror -MMD -c -o $@ $<


clean:
	rm -f $(OBJ) $(TGT) $(DEP) directory directory.o



-include $(DEP)


