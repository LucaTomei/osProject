#define _DEFAULT_SOURCE		// Evita i warning della vfork()
#include "utilities.h"	// contiene le dichiarazioni di funzioni
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>	/* Per abilitare e disabilitare Raw Mode*/
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdarg.h>


#define CTRL_KEY(k) ((k) & 0x1f) // trucchetto per gestire tutti i ctrl-*
#define handle_error(msg)    do { perror(msg); exit(EXIT_FAILURE); } while (0)	// gestore errori

struct termios initialState;	// Salvo lo stato iniziale del terminale e tutti i suoi flag


static void pulisciTerminale(){
	char *cmd = "tput";
	char *args[3];
	args[0] = "tput";
	args[1] = "reset";
	args[2] = NULL;
	pid_t pulitore = vfork();
	if(pulitore == 0){
		int res = execvp(cmd, args);
		if(res == -1)	handle_error("Errore nella Exec");
	}else if(pulitore > 0){
		int status;
		wait(&status);
	}else 	handle_error("Errore nella vfork");
}

/*Metodo per testing*/
void testaCioCheScrivi(char c){
	while (1) {
    	c = '\0';
    	if(read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) handle_error("Errore nella perpetua read!");
    	/*if(iscntrl(c))	printf("%d\r\n", c);*/
    	else	printf("%d ('%c')\r\n", c, c);
    	if (c == CTRL_KEY('q')) break;
  	}
}


int main(int argc, char *argv[]){
	pulisciTerminale();
	abilitaRawMode();
	// leggo un byte alla volta dallo standard input
	// e lo salvo in una variabile 'c'. Ritornerà 0 a EOF
	
	/*while(1){
		processaChar();
	}*/
	char c;
	testaCioCheScrivi(c);


	// ripristino il terminale dei precedenti attributi
	disabilitaRawMode();
	return 0;
}





/*
	CORPO DI FUNZIONI
*/


// 1) 	Esco dalla modalità cooked del terminale, disabilitando la stampa a video per entrare in
//		raw mode
void abilitaRawMode(){

	if(tcgetattr(STDIN_FILENO, &initialState) == -1) 	handle_error("Errore nell'enableRawMode!");
	atexit(disabilitaRawMode);		// quando termina il programma, ripristino i flag
	struct termios raw = initialState;	// copia locale

	raw.c_iflag &= ~(IXON);		// disabilito ctrl-s e ctrl-q
	raw.c_iflag &= ~(ISIG);		// disabilito ctrl-v
	raw.c_iflag &= ~(IEXTEN);	// disabilito ctr-o
	raw.c_iflag &= ~(ICRNL);	// disabilito \n
	raw.c_oflag &= ~(OPOST);	// disabilito funzionalità di elaborazione dell'output

	// disabilito ECHO,
	// la modalità canonica per leggere byte a byte,
	// e ISIG --> Gestore dei segnali, disabilita ctrl-c e ctrl-z
	raw.c_lflag &= ~(ECHO | ICANON | ISIG);	

	// disabilito altri flag (prova a vedere se non servono)
	raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);	
	raw.c_cflag |= (CS8);
	
	raw.c_cc[VMIN] = 0;	// setto il numero minimo di byte prima che la read() ritorni
						// lo setto a 0 in modo tale che la read() ritorna non appena c'è un input da leggere
  	
  	raw.c_cc[VTIME] = 2;	// quantità massima di tempo di attesa della read()
	
	// TCSAFLUSH --> Elimino qualsiasi input non letto
	if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) handle_error("Errore nella TCSAFLUSH!");
}

// 2) Ripristino tutti i flag del terminale, all'uscita
void disabilitaRawMode(){
	if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &initialState) == -1) handle_error("Errore: non riesco a disabilitare la raw mode!");	
	char *cmd = "tput";
	char *args[3];
	args[0] = "tput";
	args[1] = "reset";
	args[2] = NULL;
	pid_t pulitore = vfork();
	if(pulitore == 0){
		int res = execvp(cmd, args);
		if(res == -1)	handle_error("Errore nella Exec");
	}else if(pulitore > 0){
		int status;
		wait(&status);
	}else 	handle_error("Errore nella vfork");
}

// 3) attende la pressione di un tasto e lo restituisce
char letturaPerpetua(){
	int byteLetti;
	char c;
	while((byteLetti = read(STDIN_FILENO, &c, 1)) != 1){
		if(byteLetti == -1 && errno != EAGAIN) 	handle_error("Errore nella lettura");
	}
	return c;
}

//	4) Attende la pressione di un tasto e lo gestisce, gestendo anche i ctrl-*
void processaChar(){
	char c = letturaPerpetua();
	if(c == CTRL_KEY('q'))	exit(0);
}