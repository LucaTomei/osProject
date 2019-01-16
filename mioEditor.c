#define _DEFAULT_SOURCE		// Evita i warning della vfork()
#define _BSD_SOURCE
#define _GNU_SOURCE
#include "utilities.h"	// contiene le dichiarazioni di funzioni
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>	/* Per sbilitare e dissbilitare Raw Mode*/
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/ioctl.h>	/*Per la struct winsize e ottenere le dimensioni dello schermo*/
#include <sys/wait.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdarg.h>	/*Per va_start() e va_end()*/

#define COLORASCHERMO write(STDOUT_FILENO, "\033[48;5;148m ", 11);
	
	/*#define colore "\x1b[attr1;attr2;attr3m*/
#define COLOR_GREEN   "\x1b[1;32m"	/*Bold Verde*/
#define COLOR_RESET   "\x1b[0m"

#define STOP_TAB 8

#define CTRL_KEY(k) ((k) & 0x1f) // trucchetto per gestire tutti i ctrl-*



typedef struct config{
	/*Coordinate orizzantali (colonne) e verticali (righe*/
	int x, y;
	int rx;	/*indice del campo di rendering, se non vi sono TAB rx == x, se ci sono rx > x*/
	int offsetRiga;		/*tiene traccia della riga/colonna in cui sono x lo scorrimento*/
	int offsetColonna; 	/*orizzontale e verticale dell'editor. Sarà l'indice dei caratteri*/
  	int righe, colonne;				
  	int numRighe;
  	EditorR* row;	/*Mi serve un puntatore ai dati di carattere da scrivere*/
  	char* nomeFile;
  	char statusmsg[80];	/*Stringa che mi serve per abilitare la ricerca nella barra di stato*/
  	time_t statusmsg_time;	/*Timestamp per messaggio, in modo in poco tempo posso cancellarlo*/
  	struct termios initialState;	// Salvo lo stato iniziale del terminale e tutti i suoi flag
}config;

config Editor;


#define StringBuffer_INIT {NULL, 0}	// inizializza la struct 
struct StringBuffer {
  	char *b;
  	int len;
};




enum editorKey {
	FRECCIA_SINISTRA = 1000,	/*Dalla prossima kiave in poi i numeri incrementeranno di uno*/
	FRECCIA_DESTRA,
	FRECCIA_SU,
	FRECCIA_GIU,
	CANC,	/*<esc> [3 ~*/
	HOME,	/*Fn + ←*/
	END,	/*Fn + →*/
	PAGINA_SU,
	PAGINA_GIU
};


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
	// e lo salvo in una varisbile 'c'. Ritornerà 0 a EOF
	inizializzaEditor();

	/*apriFileTest();*/
	if(argc >= 2)	openFile(argv[1]);

	setStatusMessage("Help CTR-q == quit");

	/*write(STDOUT_FILENO, "\033[48;5;148m ", 11);	COLORA LO SCHERMO*/
	/*Ho definito la macro -----> COLORASCHERMO;*/
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

// 1) 	Esco dalla modalità cooked del terminale, dissbilitando la stampa a video per entrare in
//		raw mode
void abilitaRawMode(){

	if(tcgetattr(STDIN_FILENO, &Editor.initialState) == -1) 	handle_error("Errore nell'ensbleRawMode!");
	atexit(disabilitaRawMode);		// quando termina il programma, ripristino i flag
	struct termios raw = Editor.initialState;	// copia locale

	raw.c_iflag &= ~(IXON);		// dissbilito ctrl-s e ctrl-q
	raw.c_iflag &= ~(ISIG);		// dissbilito ctrl-v
	raw.c_iflag &= ~(IEXTEN);	// dissbilito ctr-o
	raw.c_iflag &= ~(ICRNL);	// dissbilito \n
	raw.c_oflag &= ~(OPOST);	// dissbilito funzionalità di elsborazione dell'output

	// dissbilito ECHO,
	// la modalità canonica per leggere byte a byte,
	// e ISIG --> Gestore dei segnali, dissbilita ctrl-c e ctrl-z
	raw.c_lflag &= ~(ECHO | ICANON | ISIG);	

	// dissbilito altri flag (prova a vedere se non servono)
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
	if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &Editor.initialState) == -1) handle_error("Errore: non riesco a dissbilitare la raw mode!");	
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
int letturaPerpetua(){
	int byteLetti;
	char c;
	while((byteLetti = read(STDIN_FILENO, &c, 1)) != 1){
		if(byteLetti == -1 && errno != EAGAIN) 	handle_error("Errore nella lettura");
	}

	/*Sostituisco i tasti wasd con i tasti freccia. Muovere i tasti freccia equivale a muovere il cursore
	di  '\ x1b' concatenato con A, B, C o D*/
	if (c == '\x1b') {
	    char seq[3];	// per gestire le sequenze di escape
	    if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1b';
	    if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b';

	    if(seq[0] == '['){
	    	if(seq[1] >= '0' && seq[1] <= '9'){
	    		if(read(STDIN_FILENO, &seq[2], 1) != 1)	return '\x1b';
	    		if(seq[2] == '~'){
	    			switch (seq[1]){
	    				case '1': return HOME;		/*Il tasto home e il tasto end differiscono*/
            			case '4': return END;		/*a seconda del sistema. Possono valere...*/
            			case '3': return CANC;		/*<esc> [3 ~*/
	    				case '5': return PAGINA_SU;
	    				case '6': return PAGINA_GIU;
	    				case '7': return HOME;		/*... 1, 4 oppure 7 e 8 */
            			case '8': return END;
	    			}
	    		}
	    	}else {
			    switch (seq[1]) {
			    	case 'A': 	return FRECCIA_SU;
			    	case 'B':	return FRECCIA_GIU;
			    	case 'C':	return FRECCIA_DESTRA; 
			    	case 'D':	return FRECCIA_SINISTRA;
			    	case 'H':	return HOME;
			    	case 'F': 	return END;
		    	}
	    	}
	    }else if(seq[0] == 'O') {
	      	switch (seq[1]) {
	        	case 'H': return HOME;
	        	case 'F': return END;
      		}
      	}
	    return '\x1b';
	    
	} else return c;
}

//	4) Attende la pressione di un tasto e lo gestisce, gestendo anche i ctrl-*
void processaChar(){
	int c = letturaPerpetua();
	switch (c) {
	    case CTRL_KEY('q'):
	      	write(STDOUT_FILENO, "\x1b[2J", 4);
	     	write(STDOUT_FILENO, "\x1b[H", 3);
	      	exit(0);
	      	break;

	    case HOME:
	    	Editor.x = 0;
	    	break;
	   	case END:
	   		/*Porto il cursore alla fine della riga*/
	   		if(Editor.y < Editor.numRighe)	Editor.x = Editor.row[Editor.y].size;
	   		break;
	    case PAGINA_SU:
	    case PAGINA_GIU:
	    	{
	    		if(c == PAGINA_SU)	Editor.y = Editor.offsetRiga;
	    		else if(c == PAGINA_GIU){
	    			Editor.y = Editor.offsetRiga + Editor.righe -1;
		    		if(Editor.y > Editor.numRighe)	Editor.y = Editor.numRighe;
		    	}
		    	int nTimes = Editor.righe;
		    	while(nTimes--)	muoviIlCursore(c == PAGINA_SU ? FRECCIA_SU : FRECCIA_GIU);
	    	}
	    	break;
	    case FRECCIA_SU:
	    case FRECCIA_GIU:
	    case FRECCIA_SINISTRA:
	    case FRECCIA_DESTRA:
	      	muoviIlCursore(c);
	      	break;
	}
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
	editorScroll();
	struct StringBuffer sb = StringBuffer_INIT;

	sbAppend(&sb, "\x1b[?25l", 6);	// nascondo il cursore prima di aggiornare lo schermo e...
									// 'l' alla fine significa setMode
	
	sbAppend(&sb, "\x1b[H", 3);

	disegnaRighe(&sb);
	statusBarInit(&sb);
	disegnaMessaggio(&sb);

	/*Posizionamento del cursore*/
	char buf[32];
  	/*snprintf(buf, sizeof(buf), "\x1b[%d;%dH", Editor.y + 1, Editor.x + 1);*/
  	snprintf(buf, sizeof(buf), "\x1b[%d;%dH", (Editor.y - Editor.offsetRiga) + 1,(Editor.rx - Editor.offsetColonna) + 1);
  	sbAppend(&sb, buf, strlen(buf));

	sbAppend(&sb, "\x1b[?25h", 6);	// ... lo mostro subito dopo il completamento dell'aggiornamento
									// h alla fine significa reset mode
	write(STDOUT_FILENO, sb.b, sb.len);
	
	sbFree(&sb);
  	/*	OBSOLETE, GUARDA SU ^ ^
   	write(STDOUT_FILENO, "\x1b[2J", 4);
  	write(STDOUT_FILENO, "\x1b[H", 3);	// H == posizionamento del cursore
  	disegnaRighe();
  	write(STDOUT_FILENO, "\x1b[H", 3);*/
}

// 6) è l'ora di disegnare le righe del terminale
void disegnaRighe(struct StringBuffer * sb) {
  	int i;
    for (i = 0; i < Editor.righe; i++) {  // disegno ad ogni inizio riga del terminale le mie iniziali

	    /*if(i%2 != 0)  write(STDOUT_FILENO, "Ⓛ\r\n", 5);
	    else  write(STDOUT_FILENO, "Ⓣ\r\n", 5);*/
	    /*sprintf(out, "%d%s", i, base);    // prova a mettere i numeri
	    write(STDOUT_FILENO, out, 4);*/

	    /*write(STDOUT_FILENO, "Ⓛ", 3);
	    if(i <= Editor.righe) write(STDOUT_FILENO, "\r\n", 2);*/

	    int filerow = i + Editor.offsetRiga;  /*Indice di riga del file*/

	    /*Controllo se sto scrivendo una riga che fa parte del buffer di edito di testo...*/
	    if (filerow >= Editor.numRighe) {
	        if (Editor.numRighe == 0 && i == Editor.righe / 3) {  /*Se non passo alcun file*/
		    	char welcomeMessage[80];
		        int welcomelen = snprintf(welcomeMessage, sizeof(welcomeMessage),COLOR_GREEN"Il più bel text editor %s","[Mio]"COLOR_RESET);
		        if (welcomelen > Editor.colonne) welcomelen = Editor.colonne;
		            /*  Per centrare la stringa sullo schermo, divido la larghezza per 2
		              Questo mi dice quanto lontano da destra e da sinistra devo stampare 
		              */
		        int padding = (Editor.colonne - welcomelen) / 2;
		        if(padding) {
		            sbAppend(sb, "~", 1);
		            /*sbAppend(sb, "Ⓛ", 3);*/
		            padding--;
		        }
		        while(padding--)  sbAppend(sb, " ", 1);
		        sbAppend(sb, welcomeMessage, welcomelen);
		        /*write(STDOUT_FILENO, "\033[48;5;57m ", 10)  COLORA LO SCHERMO DI BLU;*/
		    } else  sbAppend(sb, "~", 1);
	    }else{
	        int len = Editor.row[filerow].effSize - Editor.offsetColonna; /*Sottraggo il numero di caratteri a sinistra dell'offset*/
	        if(len < 0) len = 0;  /*Gestisco il caso in cui len sia negativo. Le setto a 0 in modo che nulla venga visualizzato su quella linea*/
	        if (len > Editor.colonne) len = Editor.colonne;
	        sbAppend(sb, &Editor.row[filerow].effRow[Editor.offsetColonna], len);
	    }
	    sbAppend(sb, "\x1b[K", 3);   // rimuovo la sequenza di escape ('K'). K cancella la riga corrente
	    sbAppend(sb, "\r\n", 2);
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
	/*Inizializzo il cursore in alto a sinistra dello schermo*/
	Editor.x = 0;
	Editor.y = 0;
	Editor.rx = 0;
	Editor.offsetRiga = 0;
	Editor.offsetColonna = 0;
	Editor.numRighe = 0;
	Editor.row = NULL;
	Editor.nomeFile = NULL;
	Editor.statusmsg[0] = '\0';
	Editor.statusmsg_time = 0;
	/*Per colorare lo schermo*/
	/*write(STDOUT_FILENO, "\033[48;5;57m ", 10);	*/

	if(prendiDimensioni(&Editor.righe, &Editor.colonne) == -1)	handle_error("Errore: nella size! Impossibile inizializzare l'editor");
	Editor.righe -= 2; /*Tolgo le due rige in basso (quella per la barra e quella per la scrittura)*/
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


/*10) per creare una write dinamica e fare append al buffer nella struct*/
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



/*12) Funzione Per Muovere il Cursore
Muovo il cursore con i tasti w-a-s-d
-	w -> Freccia su
-	a -> Freccia sinistra
-	s -> Freccia in giù
-	d -> Freccia destra
Occorre anche gestire le eccezzioni per non far superare la dimensione dello schermo
*/
void muoviIlCursore(int tasto){
	EditorR *row = (Editor.y >= Editor.numRighe) ? NULL: &Editor.row[Editor.y];
	switch (tasto) {
	    case FRECCIA_SINISTRA:
	    	if(Editor.x != 0)	Editor.x--;	// Voglio che non vada oltre il bordo del terminale
	    	/*Se non ho nulla nella linea in cui mi trovo, se premo la freccia a sinistra devo salire di
	    	riga, come un qualsiasi editor  .....(*)*/
	    	else if(Editor.y > 0){
	    		Editor.y --;
	    		Editor.x = Editor.row[Editor.y].size;
	    	}
	      	break;
	    case FRECCIA_DESTRA:
	    	/*Ora posso andare a destra solo fino alla fine del file*/
	    	/*Se il cursore si trova su una riga effettiva, lo faccio puntare al cursore del terminale*/
	    	if(row && Editor.x < row->size)	Editor.x++;
	    	/*(*).... lo stesso gestisco per la freccia a destra, se sono alla fine della riga devo
	    		scendere giù nella riga successiva*/
	    	else if(row && Editor.x == row->size){
	    		Editor.y++;
	    		Editor.x = 0;
	    	}
	      	break;
	    case FRECCIA_SU:
	    	if(Editor.y != 0)	Editor.y--;
	      	break;
	    case FRECCIA_GIU:
	    	/* Gestione mediante offset di riga*/
	    	if(Editor.y < Editor.numRighe)		Editor.y++;
	      	break;
	}
	/*Gestisco il caso in cui la x dell'editor finisce oltre la fine della linea
	Considero NULL la linea vuota di lunghezza 0*/
	row = (Editor.y >= Editor.numRighe)? NULL : &Editor.row[Editor.y];
	int lunghRiga = row ? row->size : 0;
	if(Editor.x > lunghRiga)	Editor.x = lunghRiga;
}

/*test*/
/*void apriFileTest() {
  	char *line = "Hello, world!";
  	ssize_t linelen = 13;
  	Editor.row.size = linelen;
  	Editor.row.chars = malloc(linelen + 1);
  	memcpy(Editor.row.chars, line, linelen);
  	Editor.row.chars[linelen] = '\0';
  	Editor.numRighe = 1;
}*/

/*12)*/
void openFile(char* nomeFile){
	free(Editor.nomeFile);
	Editor.nomeFile = strdup(nomeFile);	/*Faccio una copia della stringa e alloco memoria per essa*/
	FILE *fp = fopen(nomeFile, "r");
  	if (!fp) 	handle_error("Errore: open fallita");
  	char *line = NULL;
  	size_t linecap = 0; /*capacità di linea, utili per sapere quanta memoria è stata assegnata
  						vale tanto quanto la riga o -1 a EOF*/
	ssize_t linelen;
  	linelen = getline(&line, &linecap, fp);	/*Prendo la prima riga del file. Uso getline
  											piché la funzione si occupa della gestione della memoria
  											autonomamente*/
	while((linelen = getline(&line, &linecap, fp)) != -1){
    	while(linelen > 0 && (line[linelen - 1] == '\n' || line[linelen - 1] == '\r'))
      	linelen--;
    	appendRow(line,linelen);
  	}
  	free(line);
  	fclose(fp);
}

/*13)*/
void appendRow(char *s, size_t len) {
  	Editor.row = realloc(Editor.row, sizeof(EditorR) * (Editor.numRighe + 1));

  	int at = Editor.numRighe;
  	Editor.row[at].size = len;
  	Editor.row[at].chars = malloc(len + 1);
  	memcpy(Editor.row[at].chars, s, len);
  	Editor.row[at].chars[len] = '\0';

  	/*Qui gestisco la grandezza degli spazi lasciati da un tab o da uno spazio*/
  	Editor.row[at].effSize = 0;
  	Editor.row[at].effRow = NULL;
  	aggiornaRiga(&Editor.row[at]);
  	Editor.numRighe++;
}

/*	14)
Funzione che ha lo scopo di verificare se il cursore si è spostato all'esterno della finestra.
	Questo mi serve per far si che avvenga correttamente lo scroll verticale in modo tale
	da poter settare il cursore appena all'interno della finestra visibile*/
void editorScroll() {
	Editor.rx = 0;

	if(Editor.y < Editor.numRighe)	Editor.rx = xToRx(&Editor.row[Editor.y], Editor.x);

	/*Se il cursore si trova sopra la finestra visibile, lo faccio scorrere fin dove si trova*/
	if(Editor.y < Editor.offsetRiga) Editor.offsetRiga = Editor.y;
	/*Se il cursore è oltre la parte inferiore della finestra visibile, lo ri-regolo*/
  	if(Editor.y >= Editor.offsetRiga + Editor.righe) Editor.offsetRiga = Editor.y - Editor.righe + 1;
  	
  	/*Faccio lo stesso per lo scrolling orizzontale*/
  	if(Editor.rx < Editor.offsetColonna)	Editor.offsetColonna = Editor.rx;
  	if(Editor.rx >= Editor.offsetColonna + Editor.colonne)	Editor.offsetColonna = Editor.rx - Editor.colonne +1;
}

/*15) Funzione di appoggio per aggiornamento degli spazi su una riga, riempie il contenuto
della stringa copiando ogni carattere e reindirizzandolo modificato*/
void aggiornaRiga(EditorR* row){
	int i, idx = 0, tabs = 0;
  	for (i = 0; i < row->size; i++)	if (row->chars[i] == '\t') tabs++;
    
  	free(row->effRow);
  	/*
  	Occorre scorrere i caratteri della riga per contare quanta memoria allocare per ogni tab, dato
  	che ognuno di essi occupa 8 caratteri. per ogni tab row->size parte da 1, quindi alloco 7*tab +1*/
  	row->effRow = malloc(row->size + tabs*(STOP_TAB -1)+1);
  	
  	for (i = 0; i < row->size; i++) {
  		/*Gestisco il tab*/
    	if (row->chars[i] == '\t') {	/*Se becco un tab, aggiungo uno spazio*/
      		row->effRow[idx++] = ' ';
      		while (idx % STOP_TAB != 0) row->effRow[idx++] = ' ';
    	}else 	row->effRow[idx++] = row->chars[i];
    
  	}
  	
  	/*Ora idx conterrà il numero di caratteri copiati e essegno la sua effettiva size*/
 	row->effRow[idx] = '\0';
  	row->effSize = idx;
}

/*16) Funzione che aggiorna il valore di x della struct config in rx, per calcolare l'offset effettivo
di ogni tab e tramutarlo in uno spazio vero!*/
int xToRx(EditorR* row, int x){
	int rx = 0, i;
	for(i = 0; i < x; i++){
		if(row->chars[i] == '\t')	rx += (STOP_TAB - 1) -(rx % STOP_TAB);
		/*rx % STOP_TAB mi da quante colonne sono alla destra del tab. Sottraggo STOP_TAB -1
		dunque per scoprire quante ne sono a sinistra. Infine rx++ mi porta direttamente al prossimo
		tab*/
		rx++;
	}
	return rx;
}

/*17) DIsegno la status bar*/
void statusBarInit(struct StringBuffer *sb){
	sbAppend(sb, "\x1b[7m", 4);	/*Inverto il colore del terminale da nero a bianco*/
	char status[80], rstatus[80];

	/*Mostro fino a 20 caratteri del nome del file, seguiti dal numero di righe del file*/
	int len = snprintf(status, sizeof(status), "%.20s - %d righe", 
			Editor.nomeFile ? Editor.nomeFile : "[Nessun File Aperto]", Editor.numRighe);
	int rlen = snprintf(rstatus, sizeof(rstatus), "%d%d", Editor.y +1, Editor.numRighe);
	if(len > Editor.colonne)	len = Editor.colonne;
	sbAppend(sb, status, len);

	while(len < Editor.colonne){
		/*Disegno spazi fino alla fine dello schermo*/
		if(Editor.colonne - len == rlen){
			sbAppend(sb, rstatus, rlen);
			break;
		}else{
			sbAppend(sb, " ", 1);
			len++;
		}
	}
	sbAppend(sb, "\x1b[m", 3);	/*torno alla normale formattazione*/
	sbAppend(sb, "\r\n", 2);	/*Disegno nell'ultima riga quella di aiuto*/
}

/*18) Voglio che la funzione sia variadica e che chieda un numero arbitrario di argomenti */
void setStatusMessage(const char* fmt, ...){	
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(Editor.statusmsg, sizeof(Editor.statusmsg), fmt, ap);
	va_end(ap);
	Editor.statusmsg_time = time(NULL);	/*time(NULL) mi da l'ora corrente*/
}

/*19) Questa funzione mostra un messaggio nell'ultima riga del terminale
N.B.: "svuotaschermo è la funzione che inizializza tutto*/
void disegnaMessaggio(struct StringBuffer *sb){
	sbAppend(sb, "\x1b[K", 3);	/*pulisco la riga*/
  	int msglen = strlen(Editor.statusmsg);	/*verifico che il messaggio entri nello schermo*/
  	if (msglen > Editor.colonne) msglen = Editor.colonne;
  	if (msglen && time(NULL) - Editor.statusmsg_time < 5)	/*faccio scomparire il messaggio dopo 5 secondi*/
    sbAppend(sb, Editor.statusmsg, msglen);
}

/*-------------------------------
	INIZIO LA SCRITTURA DI CHAR
--------------------------------*/