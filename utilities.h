/*-------------------------------------------------------------------------------------------
				Dichiarazione di Funzioni
--------------------------------------------------------------------------------------------*/
#include <stddef.h>
#include <errno.h>
#include <stdio.h>
void muoviIlCursore(int tasto);

/*Struct per l'editor*/
typedef struct EditorR{
	int size;
	int effSize;	/*Gestisco le effettive tabulazioni, mostrando gli spazi come dico io e non...*/
	char* chars;
	char* effRow;/*... come fa di default il terminale, altrimenti un TAB occuperebbe 7 caratteri circa*/
} EditorR;


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

void aggiornaRiga(EditorR* row);


int xToRx(EditorR* row, int x);

void statusBarInit(struct StringBuffer *sb);