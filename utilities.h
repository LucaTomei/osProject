/*-------------------------------------------------------------------------------------------
	Dichiarazione di Funzioni
--------------------------------------------------------------------------------------------*/
#include <stddef.h>
#include <errno.h>
#include <stdio.h>
void muoviIlCursore(int tasto);


#define handle_error(msg)    do { perror(msg); exit(EXIT_FAILURE); } while (0)	// gestore errori

struct StringBuffer;
void sbAppend(struct StringBuffer *sb, const char *s, int len);
void sbFree(struct StringBuffer *sb);
void disegnaRighe(struct StringBuffer *sb);

static void pulisciTerminale();

void abilitaRawMode();
void disabilitaRawMode();
void testaCioCheScrivi(char c);

int letturaPerpetua();
void processaChar();

void svuotaSchermo();


/*void disegnaRighe();*/
/*void disegnaRighe(struct StringBuffer *sb);*/

int prendiDimensioni(int *righe, int *colonne);

void editorScroll();
void inizializzaEditor();

int posizioneCursore(int* righe, int* colonne);

enum editorKey;

void apriFileTest();

void openFile(char* nomeFile);	

void appendRow(char *s, size_t len);