#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "termFunc.h"
#include "editorFunc.h"

#define MODIFICA_CURSORE    write(STDOUT_FILENO, "\033[5 q", 5);    /*Setto il cursore stile editor*/
#define RESETTACURSORE      write(STDOUT_FILENO, "\033[1 q", 5);

#define COLOR_ALERT     "\x1b[1;31m"
#define COLOR_RESET     "\x1b[0m"

int main(int argc, char *argv[]){
    pulisciTerminale();
    abilitaRawMode();
    // leggo un byte alla volta dallo standard input
    // e lo salvo in una varisbile 'c'. Ritornerà 0 a EOF
    inizializzaEditor();
    MODIFICA_CURSORE;

    /*apriFileTest();*/
    int result;
    if(argc >= 2){
        openFile(argv[1]);
        result = lockfile(argv[1], NULL);
        if (result == 0)    goto BUONO;
        else    goto APERTO;
    }


    if(argv[2] == NULL) goto BUONO;
    /*write(STDOUT_FILENO, "\033[48;5;148m ", 11);  COLORA LO SCHERMO*/
    /*Ho definito la macro -----> COLORASCHERMO;*/

    BUONO:
        setStatusMessage("Help: CTRL+s == Salva | CTRL+q == Esci | Ctrl+f = Cerca | Ctrl+n = Apri File");
        while(1){
            svuotaSchermo();
            processaChar();
        }
        goto EXIT;
    APERTO:
        setStatusMessage(COLOR_ALERT"Il file è già stato aperto. Premi CTRL+q 3 volte per uscire"COLOR_RESET);
        while(1){
            svuotaSchermo();
            processaChar();
        }
        goto EXIT;

    // ripristino il terminale dei precedenti attributi
    EXIT:
        RESETTACURSORE;
        disabilitaRawMode();        
        return 0;
}