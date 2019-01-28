#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "termFunc.h"
#include "editorFunc.h"

#define MODIFICA_CURSORE 	write(STDOUT_FILENO, "\033[5 q", 5);	/*Setto il cursore stile editor*/
#define RESETTACURSORE		write(STDOUT_FILENO, "\033[1 q", 5);

int main(int argc, char *argv[]){
	pulisciTerminale();
	abilitaRawMode();
	// leggo un byte alla volta dallo standard input
	// e lo salvo in una varisbile 'c'. Ritornerà 0 a EOF
	inizializzaEditor();
	MODIFICA_CURSORE;

	/*apriFileTest();*/
	if(argc >= 2)	openFile(argv[1]);

	setStatusMessage("Help: CTRL+s == Salva | CTRL+q == Esci | Ctrl+F = Cerca");

	/*write(STDOUT_FILENO, "\033[48;5;148m ", 11);	COLORA LO SCHERMO*/
	/*Ho definito la macro -----> COLORASCHERMO;*/
	while(1){
		svuotaSchermo();
		processaChar();
	}

	// ripristino il terminale dei precedenti attributi
	RESETTACURSORE;
	disabilitaRawMode();
	
	return 0;
}