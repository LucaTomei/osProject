#define _GNU_SOURCE   /*Per fcloseall()*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "termFunc.h"
#include "editorFunc.h"
#include "utilities.h"
#include <sys/ioctl.h>

#define MODIFICA_CURSORE    write(STDOUT_FILENO, "\033[5 q", 5);    /*Setto il cursore stile editor*/
#define RESETTACURSORE      write(STDOUT_FILENO, "\033[1 q", 5);

#define COLOR_ALERT     "\x1b[1;31m"
#define COLOR_RESET     "\x1b[0m"

config Editor;

void pre_welcome(){
    int ret;
    char* BAR = "==================================================================";
    ret = system("tput reset && echo \033[41mThese scripts are made by:\033[0m && tput sgr0 && tput setaf 2 && tput blink");
    if(ret != 0)    handle_error("Errore");

    printf("\n\n\n");
    printf("###                 ###           ###     ################     ###############\n");
    printf("###                 ###           ###    #################    ###           ###\n");
    printf("###                 ###           ###   ###                   ###           ###\n");
    printf("###                 ###           ###   ###                   ###           ###\n");
    printf("###                 ###           ###   ###                   #################\n");
    printf("###                 ###           ###   ###                   ####LUCAS'MAC####\n");
    printf("###                 ###           ###   ###                   ###           ###\n");
    printf("###                 ###           ###   ###                   ###           ###\n");
    printf("###                 ###           ###   ###                   ###           ###\n");
    printf("################     ##############      #################    ###           ###\n");
    printf("################      ###########         ################   ####           ####\n\n\n\n");
    
    ret = system("tput sgr0");
    if(ret != 0)    handle_error("Errore");

    printf("\n\n\n\n\n");
    ret = system("tput setaf 6");
    if(ret != 0)    handle_error("Errore");

    int BAR_WIDTH = 60;
    unsigned int percentage=0;
    while(percentage<100){
        ++percentage;
        unsigned int lbar=percentage*BAR_WIDTH/100;
        unsigned int rbar=BAR_WIDTH-lbar;
        printf("\r[%.*s%*s] %d%% Completed.", lbar, BAR, rbar, "", percentage);
        fflush(stdout);
        nanosleep((const struct timespec[]){{0, 26000000L}}, NULL);
    }
    printf("\n");
    ret = system("tput sgr0 && tput reset");
    if(ret != 0)    handle_error("Errore");
}

int main(int argc, char *argv[]){
    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);
    if(w.ws_col == 80 && w.ws_row == 24)	pre_welcome();

    pulisciTerminale();
    abilitaRawMode();
    // leggo un byte alla volta dallo standard input
    // e lo salvo in una varisbile 'c'. Ritornerà 0 a EOF
    inizializzaEditor();
    /*MODIFICA_CURSORE;*/

    /*apriFileTest();*/
    int result = 0;
    if(argc >= 2){
        openFile(argv[1]);
        result = lockfile(argv[1], NULL);
    }

    if (result == 0){
    	setStatusMessage("Help: CTRL+s == Salva | CTRL+q == Esci | Ctrl+f = Cerca | Ctrl+n = Apri File");
        while(1){
            svuotaSchermo();
            processaChar();
        }
    }else{
    	setStatusMessage(COLOR_ALERT"Il file è già stato aperto. CTRL+q per uscire"COLOR_RESET);
        while(1){
            svuotaSchermo();
            processaChar();
        }
    }

    /*write(STDOUT_FILENO, "\033[48;5;148m ", 11);  COLORA LO SCHERMO*/
    /*Ho definito la macro -----> COLORASCHERMO;*/

    /*      LIBERO TUTTA LA MEMORIA     */
    free(Editor.row->color);
    free(Editor.row->chars);
    free(Editor.row);
    
    free(Editor.nomeFile);
    free(Editor.statusmsg);

    free(Editor.syntax->filetype);
    free(Editor.syntax->filematch);
    free(Editor.syntax->parole);
    free(Editor.syntax->commento_singolo);
    free(Editor.syntax->inizio_multilinea);
    free(Editor.syntax->fine_multilinea);
    free(Editor.syntax);
    free(&Editor);

    result = fcloseall();
    if(result != 0 && result == EOF)	handle_error("Errore nella chiusura degli stream aperti");
    RESETTACURSORE;
    disabilitaRawMode();        
    return 0;
}