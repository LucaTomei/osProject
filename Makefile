RED='\033[1;91m'
GREEN=\033[0;32m
NC='\033[0m'
NCGREEN = \033[0m
BLUE=\033[1;96m

CC=gcc

ProgName1=LighTextEditor
CFLAGS1+=-Wall -Wextra -pedantic
PROGS1 = src/main.c src/termFunc.c src/editorFunc.c src/highlight.c

ProgName2=LighTextEditorGTK
CFLAGS2+=`pkg-config --cflags gtk+-2.0`
LIBS+=`pkg-config --libs gtk+-2.0`
PROGS2 = srcGtk/click.c srcGtk/constructorAux.c srcGtk/findText.c srcGtk/main.c

INSTALL = src/autoInstall.c

help:
	@echo $(RED)"Comandi Disponibili:"$(NC)
	@echo "\t$(BLUE)project$(NCGREEN) - Compila il Programma"
	@echo "\t$(GREEN)gtkProject$(NCGREEN) - Compila il Programma in versione GTK"
	@echo "\t$(GREEN)all$(NCGREEN) - Compila Entrambe le versioni del Programma"
	@echo "\t$(BLUE)install$(NCGREEN) - Installa l'Editor sul tuo PC"
	@echo "\t$(GREEN)clean$(NCGREEN) - clean up"
	@echo "\t$(GREEN)help$(NCGREEN) - Stampa questo Messaggio di Aiuto"

project: 
	$(CC) $(PROGS1) -o $(ProgName1) $(CFLAGS1)

gtkProject:
	$(CC) $(CFLAGS2) -o $(ProgName2) $(PROGS2) $(LIBS)

all:
	$(CC) $(PROGS1) -o $(ProgName1) $(CFLAGS1) && $(CC) $(CFLAGS2) -o $(ProgName2) $(PROGS2) $(LIBS)

install:
	$(CC) $(INSTALL) -o INSTALL && tput reset && ./INSTALL && rm INSTALL

test1:
	./$(ProgName1) src/TestInProgress/fileDaleggereUnaRiga.txt

test2:	
	./LighTextEditor src/TestInProgress/fileTabbato.txt

test3:
	./LighTextEditor src/TestInProgress/fileC.c

nuovo:
	./LighTextEditor

clean: 
	$(RM) $(ProgName1) &&  $(RM) $(ProgName2) && tput reset 

.PHONY: src/$(ProgName1) clean & tput reset		#Forzo la compilazione ogni volta