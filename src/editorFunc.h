#include <ctype.h>	/*per iscntrl()*/
#include <fcntl.h>	/*per i flag usati nel salvataggio del file su disco*/
#include <stdarg.h>	/*Per va_start() e va_end()*/

#define COLORA_NUMERI (1<<0)
#define COLORA_STRINGHE (1<<1)

enum highLight{
	NORMALE = 0,
	COMMENTO,
	COMMENTO_MULTILINEA,
	MATCHA1,	/* Per evidenziare le parole chiavi di C*/
	MATCHA2,	/*......................................*/
	STRINGA,
	NUMERO,
	RICERCA
};

void inizializzaEditor();
void processaChar();
void sbAppend(struct StringBuffer *sb, const char *s, int len);
void sbFree(struct StringBuffer *sb);
void muoviIlCursore(int tasto);
int lockfile(const char *const filepath, int *const fdptr);
void openFile(char* nomeFile);
void inserisciRiga(int at, char *s, size_t len);
void aggiornaRiga(EditorR* row);
int xToRx(EditorR* row, int x);
void setStatusMessage(const char* fmt, ...);
void scriviInRiga(EditorR *row, int at, int c);
void inserisciChar(int c);
char *rowToString(int *buflen);
void salvaSuDisco();
void cancellaCharInRiga(EditorR* row, int at);
void cancellaChar();
void liberaRiga(EditorR* row);
void cancellaRiga(int at);
void appendiStringaInRiga(EditorR* row, char* s, size_t len);
void inserisciNewLine();
char *promptComando(char *prompt, void (*callback)(char *, int));
void cercaTesto();
/*int cercaAndTabAux(EditorR *row, int rx);*/
void cercaTestoCallback(char *toFind, int key);
void aggiornaSintassi(EditorR *row);
int daSintassiAColore(int color);
int is_separator(int c);
void selezionaSintassiDaColorare();
void openNewFileFromPrompt();
