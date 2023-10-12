SRC = find.cc 
OBJ = $(SRC:.cc=.o)
TGT = find
DEP = $(SRC:.cc=.d)

$(TGT): $(OBJ)
	g++ -std=c++17 -o $@ $+

%.o: %.c Makefile
	g++ -std=c++17 -Wall -Werror -MMD -c -o $@ $<

clean:
	rm -f $(OBJ) $(TGT) $(DEP)

-include $(DEP)


