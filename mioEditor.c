#define _DEFAULT_SOURCE		// Evita i warning della vfork()
#include "utilities.h"	// contiene le dichiarazioni di funzioni
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>	/* Per abilitare e disabilitare Raw Mode*/
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/ioctl.h>	/*Per la struct winsize e ottenere le dimensioni dello schermo*/
#include <sys/wait.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdarg.h>

#define CTRL_KEY(k) ((k) & 0x1f) // trucchetto per gestire tutti i ctrl-*
#define handle_error(msg)    do { perror(msg); exit(EXIT_FAILURE); } while (0)	// gestore errori


typedef struct config{
  struct termios initialState;	// Salvo lo stato iniziale del terminale e tutti i suoi flag
  int righe, colonne;
}config;

#define StringBuffer_INIT {NULL, 0}
struct StringBuffer {
  char *b;
  int len;
};

config Editor;

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
	inizializzaEditor();
	while(1){
		svuotaSchermo();
		processaChar();
	}


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

	if(tcgetattr(STDIN_FILENO, &Editor.initialState) == -1) 	handle_error("Errore nell'enableRawMode!");
	atexit(disabilitaRawMode);		// quando termina il programma, ripristino i flag
	struct termios raw = Editor.initialState;	// copia locale

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
	if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &Editor.initialState) == -1) handle_error("Errore: non riesco a disabilitare la raw mode!");	
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


// 5) 	Occorre svuotare lo schermo del terminale dopo ogni pressione di un tasto e fare in modo 
// 		che il cursore sia posizionato sempre in alto a sinistra dello schermo
// 		(Studia bene il link nel file VT100EscapeSequenze.txt)
void svuotaSchermo() {
	/* Per fare questo bisogna scrivere sullo standard output 4 byte, di cui
		* il primo \x1b == 27 in decimale è il carattere di escape
		* il secondo [ è un carattere di escape
		* il terzo == 2 -> Indica che voglio cancellare l'intero schermo
		* il quarto J -> Indica che voglio eliminare <esc>
		Le sequenze di escape su terminale iniziano sempre con un carattere escape (27), seguito da
		[. In questo modo istruisco al terminale di spostare il cursore, cambiare il colore del font,
		cancellare parti dello schermo,...Guarda (https://vt100.net/docs/vt100-ug/chapter3.html#ED)
	*/
	struct StringBuffer sb = StringBuffer_INIT;
	sbAppend(&sb, "\x1b[2J", 4);
	
	sbAppend(&sb, "\x1b[H", 3);
	disegnaRighe(&sb);
	sbAppend(&sb, "\x1b[H", 3);
	write(STDOUT_FILENO, sb.b, sb.len);
	sbFree(&sb);

  	/*	OBSOLETE, GUARDA SU ^ ^
   	write(STDOUT_FILENO, "\x1b[2J", 4);
  	write(STDOUT_FILENO, "\x1b[H", 3);	// H == posizionamento del cursore
  	disegnaRighe();
  	write(STDOUT_FILENO, "\x1b[H", 3);*/
}

// 6) è l'ora di disegnare le righe del terminale
void disegnaRighe(struct StringBuffer *sb){
	int i;
	/*char* base = ".";
	char out[32];*/
  	for (i = 0; i < Editor.righe; i++) {	// disegno ad ogni inizio riga del terminale le mie iniziali
  		/*if(i%2 != 0)	write(STDOUT_FILENO, "Ⓛ\r\n", 5);
    	else	write(STDOUT_FILENO, "Ⓣ\r\n", 5);*/
    	/*sprintf(out, "%d%s", i, base);		// prova a mettere i numeri
    	write(STDOUT_FILENO, out, 4);*/
    	
    	/*write(STDOUT_FILENO, "Ⓛ", 3);
    	if(i <= Editor.righe)	write(STDOUT_FILENO, "\r\n", 2);*/

    	sbAppend(sb, "~", 1);
    	if(i < Editor.righe -1)	 sbAppend(sb, "\r\n", 2);
  	}
}

// 7) prendo le dimensioni dello schermo per disegnare tante righe quant'è lo schermo
/*

*/
/*
Per sapere le dimensioni dello schermo, la strategia è posizionare il cursore in basso a destra 
sullo schermo, quindi utilizzare le sequenze di escape che ci permettono di interrogare la posizione 
del cursore. 

Invio questa sequenza di due escape concatenati "\x1b[999C\x1b[999B", in cui
-	\x1b[999C 	sposta il cursore a destra di 999
-	\x1b[999B	sposta il cursore in basso nello schermo di 999
Uso 999 perché un valore così grande dovrebbe garantirmi di esplorare fino alla fine dei bordi
del terminale.
Non uso 999H perché la documentazione non specifica cosa succede quando si prova a spostare il cursore
fuori dallo schermo.
Poi effettuo una chiamata alla funzione letturaPerpetua per mostrare i risultati, prima che venga 
cancellato lo schermo.
*/
int prendiDimensioni(int *righe, int *colonne){
	struct winsize size;
	// ioctl non funziona su tuttii sistemi!!!
	/*if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0 || ws.ws_row == 0)return -1;
	else {
		// aggiungo tutti i dati alla struct config
		*colonne = ws.ws_col;	// numero di colonne...
	    *righe = ws.ws_row;	// e numero di righe
	    return 0;
	}*/

	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &size) == -1 || size.ws_col == 0) {
	    if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) return -1;
	    return posizioneCursore(righe, colonne);
  	}else {
	    *colonne = size.ws_col;
	    *righe = size.ws_row;
	    /*printf("%ls\n", righe);*/
	    return 0;
  	}
}

// 8) funzione d'appoggio per inizializzare editor
void inizializzaEditor(){
	if(prendiDimensioni(&Editor.righe, &Editor.colonne) == -1)	handle_error("Errore: nella size!");
}

// 9) 	Ora è il momento di prendere la posizione del cursore, dopo averlo spostato in basso 
int posizioneCursore(int* righe, int* colonne){
	char buf[32];
	/*
	buf è un' array in cui:
	- la prima cella contiene i caratteri escape
	- la seconda cella contiene la seconda sequenza di caratteri escape
	- dalla terza cella in poi va tutto bene
	*/
  	unsigned int i = 0;
	if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return -1;
  	
  	while (i < sizeof(buf) - 1) {
    	if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
    	if (buf[i] == 'R') break;	// devo arrivare fino ad 'R' (https://vt100.net/docs/vt100-ug/chapter3.html#CPR)
    	i++;
  	}
  	buf[i] = '\0';
  	if (buf[0] != '\x1b' || buf[1] != '[') return -1;
  	if (sscanf(&buf[2], "%d;%d", righe, colonne) != 2) return -1;
  	return -1;
}


/*10) per creare una write dinamica e fare append*/
void sbAppend(struct StringBuffer *sb, const char *s, int len) {
  char *new = realloc(sb->b, sb->len + len);
  if (new == NULL) return;
  memcpy(&new[sb->len], s, len);
  sb->b = new;
  sb->len += len;
}
/*11) Distruttore*/
void sbFree(struct StringBuffer *sb) {
  free(sb->b);
}