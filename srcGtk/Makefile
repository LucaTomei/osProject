CFLAGS+=-Wall 
CFLAGS+=`pkg-config --cflags gtk+-2.0`
LIBS+=`pkg-config --libs gtk+-2.0`
PROG=LighTGtkEditor

help:
	@echo  "Comandi Disponibili:"
	@echo  "\tall - Compila il Programma"
	@echo  "\tclean - clean up"
	@echo  "\thelp - Stampa questo Messaggio"

all:
	$(CC) $(CFLAGS) -o $(PROG) *.c $(LIBS)
test:
	./LighTGtkEditor & tput reset
clean:
	rm -f $(PROG)