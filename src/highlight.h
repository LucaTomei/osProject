#ifndef _highlight_h
#define _highlight_h
#include "utilities.h"
#include "editorFunc.h"

char *ESTENSIONI_C[] = { ".c", ".h", ".cpp", NULL };
char *PAROLE_C[] = {
    "switch", "if", "while", "for", "break", "continue", "return", "else",
    "struct", "union", "typedef", "static", "enum", "class", "case",
    "int|", "long|", "double|", "float|", "char|", "unsigned|", "signed|",
    "void|", NULL
};

struct editorSyntax HLDB[] = {
    {
      "c",
      ESTENSIONI_C,
      PAROLE_C,
      "//", "/*", "*/",
      COLORA_NUMERI | COLORA_STRINGHE
    },
};

#define HLDB_ENTRIES (sizeof(HLDB) / sizeof(HLDB[0])) /*Contiene la lunghezza dell'array HLDB*/

void aggiornaSintassi(EditorR *row);
int daSintassiAColore(int color);
int is_separator(int c);
void selezionaSintassiDaColorare();
#endif