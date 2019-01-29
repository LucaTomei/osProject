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
Per l’implementazione dell’editor, ho suddiviso il progetto il 4 macro zone:
#### __1. Modifica del Terminale, con funzioni che lo implementano__
I files _ termFunc.h_ e _ termFunc.c_ contengono le funzioni che ho utilizzato per settare determinati flag sul terminale.
Per prima cosa, ho scritto una funzione chiamata _”abilitaRawMode”_, che si occupa di uscire dalla classica modalità “cooked mode” del terminale ed entrare in modalità “Raw Mode”. Occorre quindi:
- Disabilitare tutti i tasti _ctrl_ che utilizza di default 
- Disabilitare gli accapo e la funzionalità di elaborazione dell’output (comprese le printf)
- Disabilitare la gestione dei segnali 
- Settare il numero minimo di byte prima che la _read_ ritorni e il tempo massimo di attesa
- Eliminare qualsiasi input non letto
---- 
Ovviamente, una volta terminata la scrittura nell’Editor, mi servirà la funzione _disabilitaRawMode_ per riassegnare tutti gli attributi che originariamente possedeva il terminale.
Il _main_, una volta abilitata la modalità _RawMode_, dovrà continuamente svuotare lo schermo e processare ogni singolo char che daremo in input sul terminale. Per fare ciò, utilizzo le _”Sequenze di Escape”_, le quali iniziano sempre con un carattere escape `\x1b`, seguito sempre da `[`. In questo modo comunico al terminale di spostare il cursore, cambiare il colore del font, cancellare parti dello schermo,...
> _Molto Utile è stata la guida sul sito [https://vt100.net/docs/vt100-ug/chapter3.html#ED][1], che mostra il significato di ogni singola sequenza di escape._
Per svuotare lo schermo, occorre scrivere sullo standard output 4 byte:
- Il Primo `\x1b` (27 in decimale) è il carattere di escape
-  Il Secondo`[` è un altro carattere di escape
- Il Terzo  `?25`  indica che voglio cancellare l'intero schermo
- Il Quarto `J` indica che voglio eliminare _\<esc\>_
Dopo aver impostato tutto l’occorrente, mi servirò di una struct `StringBuffer` e delle relative funzioni associate ad essa:
	struct StringBuffer {
	    char *b;
	    int len;
	};
La struct, mi servirà per creare una _write_ dinamica, in cui scriverò, tramite la funzione _memcpy_, l’input scritto nel terminale nel `char* b`, riallocando i byte necessari per la stringa, aggiornando anche la sua rispettiva lunghezza. Per ottenere il risultato che voglio aspettarmi utilizzo due funzioni:
- Il costruttore:
		void sbAppend(struct StringBuffer *sb, const char *s, int len)
- Il distruttore:
		void sbFree(struct StringBuffer *sb)
La funzione _ sbAppend_ verrà utilizzata anche per svuotare lo schermo, per nascondere il cursore `"sbAppend(&sb, "\x1b[?25l", 6);"` e successivamente per riposizionarlo in alto a sinistra nel terminale.
A questo punto, dovrò sapere l’effettiva dimensione del terminale (larghezza e altezza), per far si che:
- Il file in ingresso sia perfettamente centrato nel terminale
- Mostrare un messaggio di benvenuto nel caso in cui non passi alcun file
- Scrivere due righe sottostanti per mostrare sia la lista dei comandi implementati che per abilitare la ricerca nel file o l’apertura di un altro nella stessa finestra
	void disegnaRighe(struct StringBuffer \* sb)
Per sapere l’effettiva dimensione del file, utilizzo la `struct winsize` che contiene tutto ciò che mi occorre, cioè il numero di righe e di colonne e i pixel in orizzontale e verticale. Ora è il momento di posizionare il cursore sullo schermo, tramite la comoda funzione
	int posizioneCursore(int* righe, int* colonne)
Tale funzione mi servirà come appoggio per posizionare il cursore in posizione (0, 0).
Successivamente, dovrò verificare se il cursore si è spostato all’esterno della finestra, altrimenti potrebbe non funzionare lo scroll verticale, e posizionarlo all’interno della finestra “visibile”.
È il momento di inizializzare e disegnare la _status bar_:
	void statusBarInit(struct StringBuffer *sb)
Innanzitutto, occorre invertire il colore dell’ultima riga del terminale, per creare contrasto e renderla visibile `sbAppend(sb, "\x1b[7m", 4)`. La status bar dovrà mostrare il nome del file, il tipo (se conosciuto dall’editor), se è stato modificato, il numero di righe del file, l’indice di riga in cui mi trovo e una seconda barra che utilizzerò per la ricerca del testo, per l’apertura di un nuovo file e per mostrare un messaggio contenente i comandi da me implementati che verrà visualizzato solo per 5 secondi.
#### __2. Funzioni di utility per Editor__
L’Editor utilizza principalmente una struct, contenente tutto ciò che reputo necessario per la gestione dello stesso.
	typedef struct EditorR{
	    int index;  // Per gestire i commenti /* */ su più linee gestendo l'indice all'interno del file
	    int size;    /*Conterrà la size che occuperanno le stringhe*/
	    int effSize;    /*Gestisco le effettive tabulazioni, mostrando gli spazi come dico io e non...*/
	    char* chars;
	    char* effRow;   /*... come fa di default il terminale, altrimenti un TAB occuperebbe 7 caratteri circa*/
	    unsigned char *color;   /*conterrà valori da 0 a 255 e vedrà se ogn carattere matcherà con un stringa definita da me, per l'highlight*/
	    int is_comment; /*Variabile boolean per gestione commento*/
	} EditorR;
Prima di tutto occorre inizializzare la struct config, in modo tale da resettare ogni dato presente in essa e posizionare il cursore all’inizio del file. All’inizio ho gestito il posizionamento del cursore  attraverso la combinazione di tasti _W-A-S-D_, ma successivamente ho sostituito tale implementazione con la seguente struttura:
	enum editorKey {
	    BACKSPACE = 127,    /*ASCII == 127*/
	    FRECCIA_SINISTRA = 1000,    /* Dalla prossima chiave in poi i numeri incrementeranno di uno*/
	    FRECCIA_DESTRA,
	    FRECCIA_SU,
	    FRECCIA_GIU,
	    CANC,   /*<esc> [3 ~*/
	    HOME,   /*Fn + ←*/
	    END,    /*Fn + →*/
	    PAGINA_SU,
	    PAGINA_GIU
	};
In questo modo mi basterà concatenare la sequenza di escape `\x1b` con char che vanno da ‘1’ a ‘8’ per gestire i tasti _HOME, END, CANC PAGE-UP, PAGE-DOWN _ e da ‘A’ a ‘F’ per gestire i tasti _FRECCIA-SU, FRECCIA-GIU, FRECCIA-SINISTRA, FRECCIA-DESTRA_ tramite un semplicissimo switch nella funzione:
	int letturaPerpetua()
Tale funzione lavorerà insieme a…
	void processaChar()
…, la quale si occuperà di gestire ogni carattere passato, controllando se questo rappresenta un carattere speciale, se ho premuto _CTRL_ o se semplicemente dovrà scrivere. Per gestire i tasti _CTRL_, utilizzo la seguente macro: `#define CTRL_KEY(k) ((k) & 0x1f)`.
---- 
Una volta gestiti tutti questi casi, posso finalmente occuparmi dell’\_\_apertura di una file\_\_ tramite la funzione
	void openFile(char* nomeFile)
Appena un file viene aperto dovrò liberare la memoria allocata per il   `char* nomeFile` presente nella struct principale dell’Editor e successivamente riallocarla con il nome del file appena aperto, tramite la funzione _strdup_ che gestirà automaticamente la memoria che occorre. Il file sarà aperto in lettura, come fanno la maggior parte degli editor, e il salvataggio su disco sarà gestito successivamente da un’ altra funzione.  Per mostrare il contenuto del file sullo schermo dovrò scandire ogni sua linea, tramite la funzione ` ssize_t getline(char ** restrict linep, size_t * restrict linecapp, FILE * restrict stream)`, gratuitamente offerta da _\<stdio.h\>_ ,ed “iniettare” tante righe sul terminale, quante sono quelle scandite dal file. 
Per __modificare il contenuto del file__, utilizzo principalmente le seguenti funzioni:
	void inserisciRiga(int at, char *s, size_t len);
	void aggiornaRiga(EditorR* row);
	int xToRx(EditorR* row, int x);
	void scriviInRiga(EditorR *row, int at, int c);
	void inserisciChar(int c);
	char *rowToString(int *buflen);
*  La prima funzione la utilizzo per __gestire l’allocazione della memoria__ delle stringhe presenti su una riga, per gestire gli indici di ogni riga,  per processare ogni `char *` presente nell’Editor, incrementando il numero di righe e la sua lunghezza se presente un carattere di tabulazione
* La seconda funzione è una funzione ausiliaria, utilizzata per l’__aggiornamento degli spazi su una riga__, riempiendo il contenuto della stringa copiando ogni carattere per reindirizzarlo una volta modificato. Per fare ciò, occorre scorrere tutti i caratteri della riga per contare quanta memoria allocare per gli spazi e per i _TAB_. Dato che ogni carattere di tabulazione occupa 8 char, per ogni riga occorre allocare  `row->size + tabs*(STOP_TAB -1)+1`, in modo tale che ogni carattere letto venga copiato interamente nella _struct EditorR_.
* La terza funzione è anch’essa di appoggio e servirà per aggiornare il valore _x_ della _struct config_ in un valore _rx_ , per__ calcolare__ l’effettivo __offset di ogni tab__ e tramutarlo in un vero e proprio spazio. Per fare ciò devo sapere quante colonne sono alla destra del _TAB_ e quante ne sono a sinistra (_8 - 1_), quindi farò un controllo in un ciclo _for_ incrementando il valore di _rx_ per cercare il successivo _TAB_.
* Dalla quarta funzione in poi, mi occuperò dell’effettiva __scrittura di caratteri su__ una __riga__, dato che precedentemente ho gestito le tabulazioni e l’inserimento delle righe sul terminale. Questa fungerà da funzione ausiliaria per la prossima funzione e si occuperà dell’effettiva scrittura di caratteri nella struct dell’Editor.
* La quinta funzione __inserisce__ le __stringhe__ precedentemente lette __sulle righe del terminale__. Per fare ciò verifico dapprima la posizione del cursore sullo schermo  e se quest’ultimo si trova nella fine del file, dovrò aggiungere una nuova riga per dare la possibilità di scrivere oltre la fine del file. Sposterò successivamente la posizione del cursore in avanti in modo tale che il prossimo carattere inserito capiti subito dopo il carattere precedentemente aggiunto.
* La sesta funzione invece __incapsulerà una riga__ presente nel terminale __convertendola in una__ vera e propria__ stringa__. Un primo ciclo _for_ sommerà le lunghezze di ogni riga di testo salvando il suo valore in una variabile in modo tale che io possa allocare l’effettiva memoria necessaria per la stringa. Mi servirà anche un secondo ciclo _for_ per copiare il contenuto di ogni riga all’interno del buffer precedentemente allocato, aggiungendo un ulteriore carattere alla fine di ogni riga.
---- 
A questo punto sono pronto a __salvare__ il contenuto del file __sul disco__ tramite la funzione
	void salvaSuDisco()
Anche il salvataggio sarà dinamico, infatti prima di tutto verifico se il file è esistente, se si saprò il suo nome e dove salvarlo, altrimenti dovrò far immettere il nome per il suo successivo salvataggio e dovrò anche gestire l’interruzione di salvataggio in caso di ripensamento dall’utente. Aprirò successivamente il file in modalità lettura e scrittura se esiste, altrimenti verrà creato, tramite il flag `O_RDWR | O_CREAT`. Imposterò quindi la dimensione effettiva del file uguale alla lunghezza specificata dalla funzione _rowToString_ e con la funzione `int ftruncate(int fildes, off_t length)` di _\<unistd.h\>_ imposterò una dimensione statica al file, in modo che se è più corto, inserisce _0_ di padding, se più lungo verrà tagliato fino alla lunghezza specificata, non troncandolo completamente. A questo punto sono pronto ad usare la `write` per salvare il file sul disco mostrando anche all’utente quanti byte sono stati scritti sul disco.
#### __3. Funzioni di utility ausiliarie: Ricerca del Testo e Apertura file da Prompt__

#### __4. Funzioni per Colorazione della Sintassi e riconoscimento del tipo di File__

[1]:	https://vt100.net/docs/vt100-ug/chapter3.html#ED