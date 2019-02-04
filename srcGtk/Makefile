RED='\033[1;91m'
GREEN=\033[0;32m
NC='\033[0m'
NCGREEN = \033[0m
BLUE=\033[1;96m

CFLAGS+=`pkg-config --cflags gtk+-2.0`
LIBS+=`pkg-config --libs gtk+-2.0`
PROG=LighTextEditorGTK

help:
	@echo  $(RED)"Comandi Disponibili:"$(NC)
	@echo  "\t$(GREEN)all$(NCGREEN) - Compila il Programma"
	@echo  "\tclean - clean up"
	@echo  "\thelp - Stampa questo Messaggio"

all:
	$(CC) $(CFLAGS) -o $(PROG) *.c $(LIBS)
test:
	./LighTextEditorGTK & tput reset
clean:
	rm -f $(PROG)