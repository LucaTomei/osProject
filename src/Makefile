CC=gcc 
CFLAGS+=-Wall -Wextra -pedantic
ProgName=LighTextEditor
PROGS = main.c termFunc.c editorFunc.c

help:
	@echo "Comandi Disponibili:"
	@echo "\tall - Compila il Programma"
	@echo "\tclean - clean up"
	@echo "\thelp - Stampa questo Messaggio"

all: main.c
	$(CC) $(PROGS) -o $(ProgName) $(CFLAGS) 
test1:
	./LighTextEditor TestInProgress/fileDaleggereUnaRiga.txt
test2:	
	./LighTextEditor TestInProgress/fileTabbato.txt
test3:
	./LighTextEditor TestInProgress/fileC.c
nuovo:
	./LighTextEditor
clean: 
	$(RM) LighTextEditor && tput reset
.PHONY: LighTeditor clean	& tput reset	#Forzo la compilazione ogni volta