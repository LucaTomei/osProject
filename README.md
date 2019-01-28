# Progetto Del Corso di Sistemi Operativi
Il progetto ha lo scopo di creare un editor di testo da terminale, che implementi più comandi possibili per l’utilizzo dello stesso. 
Il cuore dell’Editor di Testo è rappresentato dalla struct config, presente nel file _utilities.h_, così definita:

	typedef struct config{
	    /*Coordinate orizzantali (colonne) e verticali (righe*/
	    int x, y;
	    int rx; /*indice del campo di rendering, se non vi sono TAB rx == x, se ci sono rx > x*/
	    int offsetRiga;     /*tiene traccia della riga/colonna in cui sono x lo scorrimento*/
	    int offsetColonna;  /*orizzontale e verticale dell'editor. Sarà l'indice dei caratteri*/
	    int righe, colonne;             
	    int numRighe;
	    EditorR* row;   /*Mi serve un puntatore ai dati di carattere da scrivere*/
	    int sporco; /*Si occuperà di mostrare se il file è stato modificato, verrà mostrato quando inizio a scrivere sul file e nascosto appena lo stalvo*/
	    char* nomeFile;
	    char statusmsg[80]; /*Stringa che mi serve per abilitare la ricerca nella barra di stato*/
	    time_t statusmsg_time;  /*Timestamp per messaggio, in modo in poco tempo posso cancellarlo*/
	    struct editorSyntax *syntax;    /*Contiene tutto ciò che mi serve per riconosce il tipo di file*/
	    struct termios initialState;    // Salvo lo stato iniziale del terminale e tutti i suoi flag
	}config;
Per l’implementazione dell’editor, ho suddiviso il progetto il 3 macro aree:
#### 1. Modifica del Terminale, con funzioni che lo implementano
Pe
#### 2. Funzioni di utility per Editor
#### 3. Funzioni per Colorazione della Sintassi e riconoscimento del tipo di File