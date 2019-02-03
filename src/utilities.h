/*-------------------------------------------------------------------------------------------
						Dichiarazione di Strutture Dati
--------------------------------------------------------------------------------------------*/
#ifndef _utilities_h
#define _utilities_h

#include <errno.h>
#include <termios.h>	/* Per qbilitare e disabilitare Raw Mode*/
#include <time.h>	/*per disabilitare dopo 5 secondi la barra sotto*/

#define handle_error(msg)    do { perror(msg); exit(EXIT_FAILURE); } while (0)	// gestore errori

#define StringBuffer_INIT {NULL, 0}	// inizializza la struct 
struct StringBuffer{
  	char *b;
  	int len;
};

#define CTRL_KEY(k) ((k) & 0x1f) // trucchetto per gestire tutti i ctrl-*

/*Struct per l'editor*/
typedef struct EditorR{
	int index;	// Per gestire i commenti /* */ su più linee gestendo l'indice all'interno del file
	int size;
	int effSize;	/*Gestisco le effettive tabulazioni, mostrando gli spazi come dico io e non...*/
	char* chars;
	char* effRow;/*... come fa di default il terminale, altrimenti un TAB occuperebbe 7 caratteri circa*/
	unsigned char *color;	/*conterrà valori da 0 a 255 e vedrà se ogn carattere matcherà con un stringa mia, per l'highlight*/
	int is_comment;	/*Variabile boolean per gestione commento*/
} EditorR;


typedef struct config{
	/*Coordinate orizzantali (colonne) e verticali (righe*/
	int x, y;
	int rx;	/*indice del campo di rendering, se non vi sono TAB rx == x, se ci sono rx > x*/
	int offsetRiga;		/*tiene traccia della riga/colonna in cui sono x lo scorrimento*/
	int offsetColonna; 	/*orizzontale e verticale dell'editor. Sarà l'indice dei caratteri*/
  	int righe, colonne;				
  	int numRighe;
  	EditorR* row;	/*Mi serve un puntatore ai dati di carattere da scrivere*/
  	int sporco;	/*Si occuperà di mostrare se il file è stato modificato, verrà mostrato quando inizio a scrivere sul file e nascosto appena lo stalvo*/
  	char* nomeFile;
  	char statusmsg[80];	/*Stringa che mi serve per abilitare la ricerca nella barra di stato*/
  	time_t statusmsg_time;	/*Timestamp per messaggio, in modo in poco tempo posso cancellarlo*/
  	struct editorSyntax *syntax;	/*Contiene tutto ciò che mi serve per riconosce il tipo di file*/
  	struct termios initialState;	// Salvo lo stato iniziale del terminale e tutti i suoi flag
}config;

enum editorKey {
	BACKSPACE = 127,	/*ASCII == 127*/
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

struct editorSyntax {	/*Per gestire per ogni tipo di file (.c, ...) la colorazione*/
  	char *filetype;
  	char **filematch;	/*Array di stringhe, ognuna delle quali contiene un pattern per verificare se il tipo file matcha*/
  	char **parole;	/*Array di stringhe, ognuna delle quali conterrà una parola che matcha con una parola chiave del linguaggio*/
  	char *commento_singolo;
  	char* inizio_multilinea;
  	char* fine_multilinea;
  	int flags;	/*Per gestire cosa devo colorare (numeri, stringhe, ...)*/
};
#endif