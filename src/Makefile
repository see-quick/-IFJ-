SRC=string.c symtable.c scanner.c scanner_list.c list.c instr_parse.c stack.c prece.c parser.c main.c
CC=gcc
CFLAGS=-std=c99 -Wall -Wextra -g -pedantic
EXE=ifj

ALL: $(SRC)

	$(CC) -o $(EXE) $^ $(CFLAGS)

memory:
	valgrind ./$(EXE) -v

clean:
	rm $(EXE)

interpret:
	$(ALL)
	./$(EXE) < vstup.code > vystupny.code
	sudo ./ic18int vystupny.code > interpret.out

test:
	@cd ../tests/; bash succ.sh; cd ../src/
