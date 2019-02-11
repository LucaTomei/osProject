#define _DEFAULT_SOURCE		// Evita i warning della vfork() e dell'ftruncate
#include "termFunc.h"	// contiene le dichiarazioni di funzioni
#include "editorFunc.h"	
#include <stdio.h>
#include <sys/wait.h>

#define COLOR_RESET		"\x1b[0m"
#define COLOR_ALERT		"\x1b[1;31m"

#define STOP_TAB 8
#define ESCI 3	/*numero di volte che devo premere ctrl-q per uscire*/

extern config Editor;

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
	Editor.sporco = 0;
	Editor.nomeFile = NULL;
	Editor.statusmsg[0] = '\0';
	Editor.statusmsg_time = 0;
	Editor.syntax = NULL;	/*Quando lo setto a NULL significa che il file non matcha il tipo e non coloro nulla*/

	/*Per colorare lo schermo*/
	/*write(STDOUT_FILENO, "\033[48;5;57m ", 10);	*/

	if(prendiDimensioni(&Editor.righe, &Editor.colonne) == -1)	handle_error("Errore: nella size! Impossibile inizializzare l'editor");
	Editor.righe -= 2; /*Tolgo le due rige in basso (quella per la barra e quella per la scrittura)*/
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
	if(c == '\x1b'){
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
	/*Tengo traccia di quante volte occorre premere ctrl-q per uscire*/
	static int quantePress = ESCI;
	int c = letturaPerpetua();

	switch (c) {
		case CTRL_KEY('n'):
			openNewFileFromPrompt();
			break;	
		case '\r':
			inserisciNewLine();
			break;
	    case CTRL_KEY('q'):
	    	if(Editor.sporco && quantePress > 0){
	    		setStatusMessage(COLOR_ALERT "  Attenzione:I tuoi dati andranno persi!\t\x1b[0m Ctrl-q %d volte per uscire", 
	    			quantePress);	
	    		quantePress--;
	    		return;
	    	}
	      	write(STDOUT_FILENO, "\x1b[2J", 4);
	     	write(STDOUT_FILENO, "\x1b[H", 3);
	      	exit(0);
	      	break;
	    case CTRL_KEY('s'):
	    	salvaSuDisco();
	    	break;
	    case HOME:
	    	Editor.x = 0;
	    	break;
	   	case END:
	   		/*Porto il cursore alla fine della riga*/
	   		if(Editor.y < Editor.numRighe)	Editor.x = Editor.row[Editor.y].size;
	   		break;
	   	case CTRL_KEY('f'):
	   		cercaTesto();
	   		break;
	   	case BACKSPACE:
	   	case CTRL_KEY('h'):	
	   	case CANC:		
	   		if(c == CANC)	muoviIlCursore(FRECCIA_DESTRA);
	   		cancellaChar();
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
	    case CTRL_KEY('l'):	/*tasto aggiornamento schermo terminale, non lo gestisco*/
	      	break;
	    case '\x1b':
	    	break;
	    default:	/*altrimenti scrivo*/
	    	inserisciChar(c);
	    	break;
	}
	quantePress = ESCI;
}



/*10) per creare una write dinamica e fare append al buffer nella struct*/
void sbAppend(struct StringBuffer *sb, const char *s, int len){
  	char *new = realloc(sb->b, sb->len + len);
  	if (new == NULL) return;
  	memcpy(&new[sb->len], s, len);
  	sb->b = new;
  	sb->len += len;
}
/*11) Distruttore*/
void sbFree(struct StringBuffer *sb){
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


int lockfile(const char *const filepath, int *const fdptr){
    struct flock lock;
    int used = 0; /* Bits da 0 a 2: stdin, stdout, stderr */
    int fd;

    FILE *fp = fopen(filepath, "a+");
    /* Se i futuro mi servirà il descrittore, lo inizializzo con -1 (disabile) */
    if (fdptr)  *fdptr = -1;

    /* Path non valido*/
    if (filepath == NULL || *filepath == '\0')  return errno = EINVAL;

    /* Open the file. */
    
    fd = fileno(fp);    // apro il file in lettura/scrittura
    
    if (fd == -1) {
        if (errno == EALREADY)  errno = EIO;    // ritorno EIO = Input/output error
        return errno;
    }

    /* Chiudo i descrittori standard che temporaneamente abbiamo usato. */
    if (used & 1){
        close(STDIN_FILENO);
    }
    if (used & 2){
        close(STDOUT_FILENO);
    }
    if (used & 4){
        close(STDERR_FILENO);
    }

    /* Se sono finiti i descrittori  */
    if (fd == -1)   return errno = EMFILE;    

    /* Lock esclusiva su un file, riguarda l'intero file!*/
    lock.l_type = F_WRLCK;  // blocco di lettura e scrittura	/*F_WRLCK --- F_RDLCK*/
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    if (fcntl(fd, F_SETLK, &lock) == -1) {  // lock fallita, chiudo il file e ritorno l'errore
        close(fd);
        return errno = EALREADY;
    }else return 0;
}

/*12)*/
void openFile(char* nomeFile){
	free(Editor.nomeFile);
	Editor.nomeFile = strdup(nomeFile);	/*Faccio una copia della stringa e alloco memoria per essa*/

	selezionaSintassiDaColorare();

	/*FILE *fp = fopen(nomeFile, "r");*/
	FILE *fp = fopen(nomeFile, "r");
  	if (!fp) 	handle_error("Errore: open fallita");
  	char *line = NULL;
  	size_t linecap = 0; /*capacità di linea, utili per sapere quanta memoria è stata assegnata
  						vale tanto quanto la riga o -1 a EOF*/
	ssize_t linelen;

	/*Prendo la prima riga del file. Uso getline poiché la funzione si occupa della gestione 
	della memoria autonomamente*/
	while((linelen = getline(&line, &linecap, fp)) != -1){
    	while(linelen > 0 && (line[linelen - 1] == '\n' || line[linelen - 1] == '\r'))
      	linelen--;
    	inserisciRiga(Editor.numRighe ,line,linelen);
  	}
  	free(line);
  	fclose(fp);
  	Editor.sporco = 0;	/*Lo setto a 0 altrimenti ogni volta che apro l'editor mi dice il file è stato modificato senza ancora non toccarlo*/
}

/*13) Inserisce una nuova riga*/
void inserisciRiga(int at, char *s, size_t len){
  	if (at < 0 || at > Editor.numRighe) return;

  	Editor.row = realloc(Editor.row, sizeof(EditorR) * (Editor.numRighe + 1));
  	memmove(&Editor.row[at + 1], &Editor.row[at], sizeof(EditorR) * (Editor.numRighe - at));
  	/*Incremento l'indice di riga del file*/
  	for(int i = at + 1; i < Editor.numRighe; i++)	Editor.row[i].index++;

  	Editor.row[at].index = at;	/*Gestione indice di riga per commenti*/

  	Editor.row[at].size = len;
  	Editor.row[at].chars = malloc(len + 1);
  	memcpy(Editor.row[at].chars, s, len);
  	Editor.row[at].chars[len] = '\0';

  	/*Qui gestisco la grandezza degli spazi lasciati da un tab o da uno spazio*/
  	Editor.row[at].effSize = 0;
  	Editor.row[at].effRow = NULL;
  	Editor.row[at].color = NULL;
  	Editor.row[at].is_comment = 0;
  	aggiornaRiga(&Editor.row[at]);

  	Editor.numRighe++;
  	Editor.sporco++;	/*Tengo traccia che il file è stato modificato*/
}


/*15) Funzione di appoggio per aggiornamento degli spazi su una riga, riempie il contenuto
della stringa copiando ogni carattere e reindirizzandolo modificato*/
void aggiornaRiga(EditorR* row){
	int i, idx = 0, tabs = 0;
  	for (i = 0; i < row->size; i++)	if (row->chars[i] == '\t') tabs++;
    
  	free(row->effRow);
  	/*Occorre scorrere i caratteri della riga per contare quanta memoria allocare per ogni tab, dato
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

  	aggiornaSintassi(row);
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
	
/*18) Voglio che la funzione sia variadica e che chieda un numero arbitrario di argomenti */
void setStatusMessage(const char* fmt, ...){	
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(Editor.statusmsg, sizeof(Editor.statusmsg), fmt, ap);
	va_end(ap);
	Editor.statusmsg_time = time(NULL);	/*time(NULL) mi da l'ora corrente*/
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
							Inizio la Scrittura di Char
*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*20) Funzione che mi fa scrivere su una riga*/
void scriviInRiga(EditorR *row, int at, int c){
	/*at = indice in cui voglio inserire il carattere*/
  	if(at < 0 || at > row->size) at = row->size;
  	row->chars = realloc(row->chars, row->size + 2);	/*aggiungo 2 per fare spazio ai byte NULL*/
  	/*uso memmove al posto di memcopy perché più sicuro quando gli array di origine e destinazione
  	si sovrappongono*/
  	memmove(&row->chars[at + 1], &row->chars[at], row->size - at + 1);	/*faccio spazio al carattere da inserire*/
  	row->size++;
  	row->chars[at] = c;
  	aggiornaRiga(row);
  	Editor.sporco++;	/*Tengo traccia che il file è stato modificato*/
}
/*21) Funzione per inserimento di char su riga*/
void inserisciChar(int c){
	/*Verifico se il cursore si trova dopo la fine del file, quindi aggiungo una nuova riga...*/
	if(Editor.y == Editor.numRighe)	inserisciRiga(Editor.numRighe,"", 0);	/*...prima di inserire un carattere li*/
	scriviInRiga(&Editor.row[Editor.y], Editor.x, c);
	/*Sposto il cursore in avanti in modo che il prossimo carattere inserito andrà
	dopo il carattere appena inserito su ^*/
	Editor.x++;
}

/*22) Funzione che incapsula una riga e la converte in stringa*/
char *rowToString(int *buflen){
  	int totlen = 0;
  	int j;
  	/*Sommo le lunghezze di ogni riga di testo*/
  	for(j = 0; j < Editor.numRighe; j++)	totlen += Editor.row[j].size + 1;	/*aggiungo uno per newline*/
  	*buflen = totlen;	/*salvo lunghezza totale per mostrare quanto è lunga la stringa*/
  	char *buf = malloc(totlen);	/*assegno memoria necessaria*/
  	char *p = buf;
  	/*Esegui il ciclo sulle righe, copiando il contenuto di ogni riga fino alla fine del buffer...*/
  	for(j = 0; j < Editor.numRighe; j++) {
    	memcpy(p, Editor.row[j].chars, Editor.row[j].size);
    	p += Editor.row[j].size;
    	*p = '\n';	/*... aggiungendo un carattere alla fine di ogni riga*/
    	p++;
  	}
  	return buf;	/*restituisco buf e faccio liberare memoria al chiamante*/
}
/*23) Finalmente il salvataggio effettivo di un file su disco*/
void salvaSuDisco(){
	/*Gestisco il caso di "nuovoFile", in tal caso sarà null e non saprò dove salvarlo*/
	if(Editor.nomeFile == NULL){
		Editor.nomeFile = promptComando("Salva Come: %s\t(ESC per uscire)", NULL);
		if(Editor.nomeFile == NULL){
			setStatusMessage("Salvataggio Interrotto");
			return;
		}
		selezionaSintassiDaColorare();
	}

	int len;
	char *buf = rowToString(&len);

	int fd = open(Editor.nomeFile, O_RDWR | O_CREAT, 0644);
	if(fd != -1){
		/*imposto la dimensione del file alla lunghezza specificata*/
		/*Uso ftruncate al posto di O_TRUNC poiché da test effettuati ho notato che se la write fallisce
		tronca il totale del contenuto del file, cosa che non voglio. In tal caso con ftruncate, imposto
		una dimensione statica al file, in modo che se è più corto aggiunge caratteri 0 di padding, se più
		lungo lo taglia fino alla len, non troncando completamente il file!*/
		if(ftruncate(fd, len) != -1){
			/*devo essere sicuro che la write restituisca i byte letti con la stessa len che gli ho
			detto di scrivere*/
			if(write(fd, buf, len) == len){
				close(fd);
				free(buf);
				Editor.sporco = 0;
				setStatusMessage("Ho scritto %d byte su disco", len);
				return;
			}
			close(fd);
		}
		free(buf);
		setStatusMessage("Non riesco a salvare! I/O error: %s", strerror(errno));
	}
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
									FUNZIONI DI CANC
*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*24) Implemento la funzione di DEL, mangia il testo dalla destra del prossimo char
	(Funzione ausiliaria)*/
void cancellaCharInRiga(EditorR* row, int at){
	if (at < 0 || at >= row->size) return;
	/*Sovrascrivo il il carattere cancellato con il carattere alla posizione successiva*/
  	memmove(&row->chars[at], &row->chars[at + 1], row->size - at);
  	row->size--;
  	aggiornaRiga(row);
  	Editor.sporco++;
}

/*25) In questo modo cancello il primo char che è sulla sinistra del cursore*/
void cancellaChar(){
	/*Se il cursore supera la fine del file, non cancello nulla*/
	if(Editor.y == Editor.numRighe)	return;
	/*Se il cursore è all'inizio della prima riga, non deve fare nulla*/
	if(Editor.x == 0 && Editor.y == 0)	return;

	/*Altrimenti se c'è un carattere a sinistra del cursore, lo cancello e sposto il cursore a sinistra
	N.B.: row punta alla riga che sto cancellando*/
	EditorR *row = &Editor.row[Editor.y];
	if(Editor.x > 0){
		cancellaCharInRiga(row, Editor.x -1);
		Editor.x --;
	}else{	/*Se la x dell'Editor è uguale a 0 ( => all'inizio di qualsiasi riga)*/
		Editor.x = Editor.row[Editor.y - 1].size;
		/*Dato che row punta alla riga che sto cancellando, appendo la successiva riga a quella prima ...*/
		appendiStringaInRiga(&Editor.row[Editor.y -1 ], row->chars, row->size);
		cancellaRiga(Editor.y);	/*... e cancello la riga su cui è presente*/
		Editor.y--;
	}
}

/*26) Il cancellaChar non fa nulla se mi trovo su una linea sotto la prima e cancello dall'inizio, cosa 
che gestisce ogni edito. Quindi devo fare in modo che unisca le due righe*/
void liberaRiga(EditorR* row){
	free(row->effRow);
	free(row->chars);
	free(row->color);
}
void cancellaRiga(int at){
	if(at < 0 || at >= Editor.numRighe)	return;
	liberaRiga(&Editor.row[at]);
	/*Sovrascrivo la struttura della riga cancellata con il resto delle righe che vengono dopo di essa*/
	memmove(&Editor.row[at], &Editor.row[at + 1], sizeof(EditorR) * (Editor.numRighe - at - 1));
  	
  	for(int i = 0; i < Editor.numRighe - 1; i++)	Editor.row[i].index--;	/*Decremento l'indice di riga*/

  	Editor.numRighe--;
  	Editor.sporco++;	/*avviene una modifica, quindi aggiorno*/
}

/*27) Funzione che aggiunge una riga alla fine di una riga*/
void appendiStringaInRiga(EditorR* row, char* s, size_t len){
	/*Le nuove dimensioni della riga saranno la size della riga + la lunghezza +1(comprende il byte null)*/
	row->chars = realloc(row->chars, row->size + len + 1);
	/*Copio la stringa data alla fine del contenuto di di row->chars*/
  	memcpy(&row->chars[row->size], s, len);
  	row->size += len;
  	row->chars[row->size] = '\0';
  	aggiornaRiga(row);
 	Editor.sporco++;
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
								FINE FUNZIONI DI CANC
*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
								GESTIONE DEL TASTO INVIO
*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*28) Inserimento new line per tasto di invio*/
void inserisciNewLine(){
	/*Se sono all'inizio del file, basta aggiungere una riga*/
	if (Editor.x == 0) inserisciRiga(Editor.y, "", 0);
  	else{	/*Altrimenti splitto la linea in 2*/
	    EditorR *row = &Editor.row[Editor.y];
	    /*1) Inserisco una riga con i caratteri che stanno a destra del cursore*/
	    inserisciRiga(Editor.y + 1, &row->chars[Editor.x], row->size - Editor.x);
	    row = &Editor.row[Editor.y];
	    row->size = Editor.x;
	    row->chars[row->size] = '\0';
	    /*2) dato che la realloc della inserisciRiga potrebbe spostare la memoria e invalidare il
	    	puntatore, tronco i contenuti della riga corrente, e la aggiorno con aggiornaRiga*/
	    aggiornaRiga(row);
  	}
  	Editor.y++;
  	Editor.x = 0;	/*Sposto il cursore all'inizio della riga successiva*/
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
							FINE GESTIONE DEL TASTO INVIO
*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
								GESTIONE PROMPT PER NUOVO FILE
Ora occorre gestire il caso in cui non passo alcun file ad argv[1].
In quel caso devo far inserire il nome del file di testo nella barra alla fine del
file, altrimenti se scrivessi qualcosa, perderei tutto.
*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*29) Scrittura nel prompt ---> Ultima riga editor*/
/*Inserisco come argomento un puntatore a funzione in modo tale che se gli passo null non fa niente
e se gli passo caratteri con la propria size mi ricerca il testo*/
char *promptComando(char *prompt, void (*callback)(char *, int)){	
  	size_t bufsize = 128;
  	char *buf = malloc(bufsize);	/*Memorizzo l'input dell'utente*/
  	size_t buflen = 0;
  	buf[0] = '\0';
  	while (1) {
  		setStatusMessage(prompt, buf);
    	svuotaSchermo();
    	int c = letturaPerpetua();
    	if(c == CANC || c == CTRL_KEY('h') || c == BACKSPACE){	/*Cancellazione nel prompt*/
    		if(buflen != 0)	buf[--buflen] = '\0';
    	}else if (c == '\x1b') {	/*Se premo esc*/
    		setStatusMessage("");
    		if(callback)	callback(buf, c);
      		free(buf);	
      		return NULL;
    	}else if (c == '\r') {	/*Se premo invio e l'input non è vuoto, cancello il messaggio*/
      		if (buflen != 0) {
        		setStatusMessage("");
        		if (callback) callback(buf, c);
        		return buf;
      		}
    	}else if(!iscntrl(c) && c < 128) {	/*Se inserisco un carattere stampabile, lo aggiungo a buf*/
      		if (buflen == bufsize - 1) {	/*Se buflen è pieno, raddoppio la memoria a lei associata*/
        		bufsize *= 2;	
        		buf = realloc(buf, bufsize);
      		}
      		buf[buflen++] = c;
      		buf[buflen] = '\0';	/*Mi devo assicurare che buf termini con '\0', altrimenti non
      			funziona un ca*** */
    	}
    	if(callback) 	callback(buf, c);
  	}
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
							FINE GESTIONE PROMPT PER NUOVO FILE	
*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void openNewFileFromPrompt(){
	char *nomeFile = promptComando("Quale file vorresti aprire? %s", NULL);
	
	free(Editor.nomeFile);
	/*Verifico se il file esiste*/
	if(access(nomeFile, F_OK) != -1){	/*Il file esiste*/
		inizializzaEditor();

		Editor.nomeFile = strdup(nomeFile);
		selezionaSintassiDaColorare();
		FILE *fp = fopen(nomeFile, "r");
		if(!fp) 	handle_error("Errore: open fallita");
		char *line = NULL;
		size_t linecap = 0; 
		ssize_t linelen;
		while((linelen = getline(&line, &linecap, fp)) != -1){
    		while(linelen > 0 && (line[linelen - 1] == '\n' || line[linelen - 1] == '\r'))
      		linelen--;
    		inserisciRiga(Editor.numRighe ,line,linelen);
  		}
  		free(line);
	  	fclose(fp);	
	  	Editor.sporco = 0;

		/*----> Versione 2 <---- - Performance*/
		/*char *cmd = "./mioEditor";
		char *args[3];
		args[0] = "./mioEditor";
		args[1] = nomeFile;
		args[2] = NULL;
		pid_t pulitore = vfork();
		if(pulitore == 0){
			int res = execvp(cmd, args);
			if(res == -1)	handle_error("Errore nella Exec");
		}else if(pulitore > 0){
			int status;
			wait(&status);
			disabilitaRawMode();
			abilitaRawMode();
		}else 	handle_error("Errore nella vfork");*/
	}else{	/*Il file non esiste*/
	  	inizializzaEditor();
	  	selezionaSintassiDaColorare();
	  	FILE *fp = fopen(nomeFile, "w");
	  	if(!fp) 	handle_error("Errore: open fallita");
	  	fclose(fp);	
	  	Editor.sporco = 0;
	}
	
	free(nomeFile);
}
