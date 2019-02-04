RED='\033[1;91m'
GREEN=\033[0;32m
NC='\033[0m'
NCGREEN = \033[0m
BLUE=\033[1;96m

CC=gcc
CFLAGS+=-Wall -Wextra -pedantic
ProgName=LighTextEditor
PROGS = main.c termFunc.c editorFunc.c highlight.c
INSTALL = autoInstall.c

help:
	@echo $(RED)"Comandi Disponibili:"$(NC)
	@echo "\t$(GREEN)all$(NCGREEN) - Compila il Programma"
	@echo "\t$(BLUE)install$(NCGREEN) - Installa il Programma sul tuo PC"
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