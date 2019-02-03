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
	@echo "Comandi Disponibili:"
	@echo "\tproject - Compila il Programma"
	@echo "\tgtkProject - Compila il Programma in versione GTK"
	@echo "\tinstall - Installa il Programma sul tuo PC"
	@echo "\tclean - clean up"
	@echo "\thelp - Stampa questo Messaggio"

project: 
	$(CC) $(PROGS1) -o $(ProgName1) $(CFLAGS1)
gtkProject:
	$(CC) $(CFLAGS2) -o $(ProgName2) $(PROGS2) $(LIBS)
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