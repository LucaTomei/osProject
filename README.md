# Progetto Del Corso di Sistemi Operativi
Il progetto ha lo scopo di creare un editor di testo da terminale, che implementi più comandi possibili per l’utilizzo dello stesso. 
Il cuore dell’Editor di Testo è rappresentato dalla struct config, presente nel file _utilities.h_, così definita:

	typedef struct config{
	    int x, y;   /*Indice di Riga e di Colonna del Terminale*/
	    int rx; /* Indice del campo di rendering, se non vi sono TAB rx == x, se ci sono rx > x*/
	    int offsetRiga;     /*Tiene traccia della riga in cui sono */
	    int offsetColonna;  /*Tiene traccia della colonna in cui sono. Rappresenterà l'indice dei caratteri*/
	    int righe, colonne;
	    int numRighe;
	    EditorR* row;   /*Mi serve un puntatore ai dati di carattere da scrivere*/
	    int sporco; /*Si occuperà di mostrare se il file è stato modificato*/
	    char* nomeFile;
	    char statusmsg[80];  /*Stringa che utilizzo per la ricerca bella barra di stato*/
	    time_t statusmsg_time;  /*Timestamp per messaggio, in modo tale in poco tempo posso cancellarlo*/
	    struct editorSyntax *syntax;    /*Contiene tutto ciò che mi serve per riconosce il tipo di file*/
	    struct termios initialState;    // Salvo lo stato iniziale del terminale e tutti i suoi flag
	}config;
Per l’implementazione dell’editor, ho suddiviso il progetto il 3 macro aree:
#### __1. Modifica del Terminale, con funzioni che lo implementano
I files _ termFunc.h_ e _ termFunc.c_ contengono le funzioni che ho utilizzato per settare determinati flag sul terminale.
Per prima cosa, ho scritto una funzione chiamata _”abilitaRawMode”_, che si occupa di uscire dalla classica modalità “cooked mode” del terminale ed entrare in modalità “Raw Mode”. Occorre quindi:
- Disabilitare tutti i tasti _ctrl_ che utilizza di default 
- Disabilitare gli accapo e la funzionalità di elaborazione dell’output (comprese le printf)
- Disabilitare la gestione dei segnali 
- Settare il numero minimo di byte prima che la _read_ ritorni e il tempo massimo di attesa
- Eliminare qualsiasi input non letto
Ovviamente, una volta terminata la scrittura nell’Editor, mi servirà la funzione _disabilitaRawMode_ per riassegnare tutti gli attributi che originariamente possedeva il terminale.
Il _main_, una volta abilitata la modalità _RawMode_, dovrà continuamente svuotare lo schermo e processare ogni singolo char che daremo in input sul terminale. Per fare ciò, utilizzo le _”Sequenze di Escape”_, le quali iniziano sempre con un carattere escape `\x1b`, seguito sempre da `[`. In questo modo comunico al terminale di spostare il cursore, cambiare il colore del font, cancellare parti dello schermo,...
> _Molto Utile è stata la guida sul sito [https://vt100.net/docs/vt100-ug/chapter3.html#ED][1], che mostra il significato di ogni singola sequenza di escape._
Per svuotare lo schermo, occorre scrivere sullo standard output 4 byte:
- Il Primo `\x1b` (27 in decimale) è il carattere di escape
-  Il Secondo`[` è un altro carattere di escape
- Il Terzo  `?25`  indica che voglio cancellare l'intero schermo
- Il Quarto `J` indica che voglio eliminare _\<esc\>_
Una volta impostati tu
#### __2. Funzioni di utility per Editor
#### __3. Funzioni per Colorazione della Sintassi e riconoscimento del tipo di File

[1]:	https://vt100.net/docs/vt100-ug/chapter3.html#ED