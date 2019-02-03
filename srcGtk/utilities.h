#ifndef UTILITIES_H
#define UTILITIES_H
#include "findText.h"
GtkTextTagTable *table1;
GtkTextTag *highlight_tag;

typedef struct {
    GtkWidget *textview, *search;
}LighTextEditor;


/*Lista di funzioni e dei loro corrispettivi nel database */
extern char *DB[][2];


GtkWidget* costruttore_finestra(char *tip);
int get_pos(char *text);
void ricerca_aux(GtkWidget *search_button, LighTextEditor *editor);
void aggiorna_barra_di_stato(GtkTextBuffer *buffer, GtkStatusbar *statusbar);
void aggiornaBarCallback(GtkTextBuffer *buffer,const GtkTextIter *new_location, GtkTextMark *mark,gpointer data);
void click_nuovo(GtkToolButton*, LighTextEditor*);
void click_apri(GtkToolButton*, LighTextEditor*);
void click_salva(GtkToolButton*, LighTextEditor*);
void click_taglia(GtkToolButton*, LighTextEditor*);
void click_copia(GtkToolButton*, LighTextEditor*);
void click_incolla(GtkToolButton*, LighTextEditor*);
void click_cerca(GtkButton*, LighTextEditor*);
void annulla_ripeti(GtkWidget *widget, gpointer item);
void seleziona_font(GtkToolButton *cut, LighTextEditor *editor);
void select_font(GtkToolButton *cut, LighTextEditor *editor);
#endif