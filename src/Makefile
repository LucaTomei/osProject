CC=gcc
CFLAGS+=-Wall -Wextra -pedantic
ProgName=LighTextEditor
PROGS = main.c termFunc.c editorFunc.c highlight.c
INSTALL = autoInstall.c

help:
	@echo "Comandi Disponibili:"
	@echo "\tall - Compila il Programma"
	@echo "\tinstall - Installa il Programma sul tuo PC"
	@echo "\tclean - clean up"
	@echo "\thelp - Stampa questo Messaggio"

all: main.c
	$(CC) $(PROGS) -o $(ProgName) $(CFLAGS) 
install:
	$(CC) $(INSTALL) -o INSTALL && tput reset && ./INSTALL && rm INSTALL
test1:
	./LighTextEditor TestInProgress/fileDaleggereUnaRiga.txt
test2:	
	./LighTextEditor TestInProgress/fileTabbato.txt
test3:
	./LighTextEditor TestInProgress/fileC.c
nuovo:
	./LighTextEditor
clean: 
	$(RM) LighTextEditor && $(RM) INSTALL && tput reset
.PHONY: LighTeditor clean & tput reset	#Forzo la compilazione ogni volta