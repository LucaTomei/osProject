#define _DEFAULT_SOURCE		// Evita i warning della vfork()
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "termFunc.h"
#include "editorFunc.h"
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>	/*Per la struct winsize e ottenere le dimensioni dello schermo*/


#define COLORASCHERMO 		write(STDOUT_FILENO, "\033[48;5;148m ", 12);
#define MODIFICA_CURSORE 	write(STDOUT_FILENO, "\033[5 q", 5);	/*Setto il cursore stile editor*/
#define RESETTACURSORE		write(STDOUT_FILENO, "\033[1 q", 5);

	/*#define colore "\x1b[attr1;attr2;attr3m*/
#define COLOR_GREEN   	"\x1b[1;32m"	/*Bold Verde*/
#define COLOR_RESET		"\x1b[0m"
#define COLOR_ALERT		"\x1b[1;31m"

#define CTRL_KEY(k) ((k) & 0x1f) // trucchetto per gestire tutti i ctrl-*

extern config Editor;

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

void pulisciTerminale(){
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

// 1) 	Esco dalla modalità cooked del terminale, disabilitando la stampa a video per entrare in
//		raw mode
void abilitaRawMode(){
	if(tcgetattr(STDIN_FILENO, &Editor.initialState) == -1) 	handle_error("Errore nell'ensbleRawMode!");
	atexit(disabilitaRawMode);		// quando termina il programma, ripristino i flag
	struct termios raw = Editor.initialState;	// copia locale

	raw.c_iflag &= ~(IXON);		// disabilito ctrl-s e ctrl-q
	raw.c_iflag &= ~(ISIG);		// disabilito ctrl-v
	raw.c_iflag &= ~(IEXTEN);	// disabilito ctr-o
	raw.c_iflag &= ~(ICRNL);	// disabilito \n
	raw.c_oflag &= ~(OPOST);	// disabilito funzionalità di elaborazione dell'output

	// dissbilito ECHO,
	// la modalità canonica per leggere byte a byte,
	// e ISIG --> Gestore dei segnali, dissbilita ctrl-c e ctrl-z
	raw.c_lflag &= ~(ECHO | ICANON | ISIG);	

	// disabilito tutti gli altri flag	
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

// 5) 	Occorre svuotare lo schermo del terminale dopo ogni pressione di un tasto e fare in modo 
// 		che il cursore sia posizionato sempre in alto a sinistra dello schermo
// 		(Studia bene il link nel file VT100EscapeSequenze.txt)
void svuotaSchermo(){
	/* Per fare questo bisogna scrivere sullo standard output 4 byte, di cui
		* il primo \x1b == 27 in decimale è il carattere di escape
		* il secondo [ è un carattere di escape
		* il terzo ?25== 2 -> Indica che voglio cancellare l'intero schermo
		* il quarto J -> Indica che voglio eliminare <esc>
		Le sequenze di escape su terminale iniziano sempre con un carattere escape (27), seguito da
		[. In questo modo comunico al terminale di spostare il cursore, cambiare il colore del font,
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
void disegnaRighe(struct StringBuffer * sb){
  	int i;
    for (i = 0; i < Editor.righe; i++){  // disegno ad ogni inizio riga del terminale le mie iniziali

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
		        int welcomelen = snprintf(welcomeMessage, sizeof(welcomeMessage),COLOR_GREEN"\tLighTextEditor (by LucasMac)"COLOR_RESET);
		        if (welcomelen > Editor.colonne) welcomelen = Editor.colonne;
		        /*  Per centrare la stringa sullo schermo, divido la larghezza per 2
		            Questo mi dice quanto lontano da destra e da sinistra devo stampare 
		        */
		        int padding = (Editor.colonne - welcomelen) / 2;
		        if(padding) {
		            /*sbAppend(sb, "~", 1);*/
		            if(i % 2 == 0)	sbAppend(sb, "Ⓛ", 3);
		        	else sbAppend(sb, "Ⓣ", 3);
		            padding--;
		        }
		        while(padding--)  sbAppend(sb, " ", 1);
		        sbAppend(sb, welcomeMessage, welcomelen);
		        /*write(STDOUT_FILENO, "\033[48;5;57m ", 10)  COLORA LO SCHERMO DI BLU;*/
		    }else{	/*sbAppend(sb, "~", 1);*/ 
		        if(i % 2 == 0)	sbAppend(sb, "Ⓛ", 3);
		        else sbAppend(sb, "Ⓣ", 3);
		    }
	    }else{
	        int len = Editor.row[filerow].effSize - Editor.offsetColonna; /*Sottraggo il numero di caratteri a sinistra dell'offset*/
	        if(len < 0) len = 0;  /*Gestisco il caso in cui len sia negativo. Le setto a 0 in modo che nulla venga visualizzato su quella linea*/
	        if (len > Editor.colonne) len = Editor.colonne;
	        /*sbAppend(sb, &Editor.row[filerow].effRow[Editor.offsetColonna], len);*/

	        char *c = &Editor.row[filerow].effRow[Editor.offsetColonna];
	        unsigned char* hl = &Editor.row[filerow].color[Editor.offsetColonna];
	        
	        int curr_color = -1;	/*Se voglio il colore di default ---> -1*/

	        int j;
	        for(j = 0; j < len; j++){	/*Itero tra i caratteri*/
	        	if(iscntrl(c[j])){	/*Traduco i caratteri control che non mi servono in caratteri ASCII*/
	        		char strano = (c[j] <= 26) ? '@' + c[j] : '?';	/*aggiungendo il suo valore a @, poiché le lettere maiuscolo in ascii vanno dopo la @*/
	        		/*? se non è nell'alfabeto*/
	        		sbAppend(sb, "\x1b[7m", 4);	/*passo a colori invertiti prima di stampare il carattere tradotto*/
          			sbAppend(sb, &strano, 1);
          			sbAppend(sb, "\x1b[m", 3);	/*disattivo i colori invertiti*/
	        		if(curr_color != -1){
	        			char buf[16];
            			int clen = snprintf(buf, sizeof(buf), "\x1b[%dm", curr_color);
            			sbAppend(sb, buf, clen);
	        		}
	        	}else if(hl[j] == NORMALE){	/*Se non è particolare*/
	        		if(curr_color != -1){
		        		sbAppend(sb, "\x1b[39m", 5);	/*Resetto il colore*/
		        		curr_color = -1;
	        		}
	        		sbAppend(sb, &c[j], 1);
	        	}else{	/*Altriment, basta scrivere la sequenza di escape nel buffer*/
	        		int color = daSintassiAColore(hl[j]);
	        		if(color != curr_color){
	        			curr_color = color;
	        			char buf[16];
	        			int clen = snprintf(buf, sizeof(buf), "\x1b[%dm", color);
	        			sbAppend(sb, buf, clen);
	        		}
	        		sbAppend(sb, &c[j], 1);
	        	}
	        }
	        sbAppend(sb, "\x1b[39m", 5);	/*Devo essere sicuro che poi tutto sia resettato*/
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

/*	14)
Funzione che ha lo scopo di verificare se il cursore si è spostato all'esterno della finestra.
	Questo mi serve per far si che avvenga correttamente lo scroll verticale in modo tale
	da poter settare il cursore appena all'interno della finestra visibile*/
void editorScroll(){
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


/*17) Disegno la status bar*/
void statusBarInit(struct StringBuffer *sb){
	sbAppend(sb, "\x1b[7m", 4);	/*Inverto il colore del terminale da nero a bianco*/
	char status[80], rstatus[80];

	/*Mostro fino a 20 caratteri del nome del file, seguiti dal numero di righe del file*/
	int len = snprintf(status, sizeof(status), "%.20s - %d lines %s",
    		Editor.nomeFile ? Editor.nomeFile : "[Nessun File Aperto]", Editor.numRighe,
    		Editor.sporco ? "(Modificato)" : "");

	int rlen = snprintf(rstatus, sizeof(rstatus), "%s | %d/%d",
    	Editor.syntax ? Editor.syntax->filetype : "Nessun Tipo Trovato", Editor.y + 1, Editor.numRighe);

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

/*19) Questa funzione mostra un messaggio nell'ultima riga del terminale
N.B.: "svuotaschermo è la funzione che inizializza tutto*/
void disegnaMessaggio(struct StringBuffer *sb){
	sbAppend(sb, "\x1b[K", 3);	/*pulisco la riga*/
  	int msglen = strlen(Editor.statusmsg);	/*verifico che il messaggio entri nello schermo*/
  	if (msglen > Editor.colonne) msglen = Editor.colonne;
  	if (msglen && time(NULL) - Editor.statusmsg_time < 5)	/*faccio scomparire il messaggio dopo 5 secondi*/
    sbAppend(sb, Editor.statusmsg, msglen);
}
