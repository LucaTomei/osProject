#ifndef _termfunc_h
#define _termfunc_h
#include <stdio.h>
#include <stdlib.h>
#include "utilities.h"
#include <unistd.h>
#include <string.h>
#include <ctype.h>

int prendiDimensioni(int *righe, int *colonne);
void sbAppend(struct StringBuffer *sb, const char *s, int len);
void sbFree(struct StringBuffer *sb);
int posizioneCursore(int* righe, int* colonne);
void testaCioCheScrivi(char c);
void pulisciTerminale();
void abilitaRawMode();
void disabilitaRawMode();
void svuotaSchermo();
void disegnaRighe(struct StringBuffer * sb);

void editorScroll();
void statusBarInit(struct StringBuffer *sb);
void disegnaMessaggio(struct StringBuffer *sb);
int xToRx(EditorR* row, int x);
#endif