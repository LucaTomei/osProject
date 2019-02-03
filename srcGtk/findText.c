/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
*		FUNZIONI DI AUSILIO E PER LA RICERCA DEL TESTO
*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#include <gtk/gtk.h>
#include "utilities.h"

char *DB[][2] ={
    {"printf", "(const char *format, ...)"},
    {"fprintf", "(FILE *stream, const char *format, ...)"},
    {"scanf", "(const char *format, ...)"},
    {"snprintf", "(char *str, size_t size, const char *format, ...)"},
    {"fputs", "(const char *s, FILE *stream)"},
    {"putc", "(int c, FILE *stream)"},
    {"putchar", "(int c)"},
    {"puts", "(const char *s)"},
    {"for","(inizializzazione; condizione; contatore)"},
    {"if", "(condizione)"},
    {"else if","(condizione)"},
    {"else","(condizione)"},
    {"while","(condizione)"},
    {"int", "nome variable"},
    {"float","nome variable"},
    {"char","nome variable"},
    {"double","nome variable"},
    {"break","esci dal loop"},
    {"switch","variabile da verificare"},
    {"void",NULL},
    {"return",NULL},
    {"unsigned",NULL},
    {"long",NULL},
    {"continue",NULL}
};
#define DB_SIZE (sizeof(DB)/sizeof(DB[0]))

int get_pos(char *text){
    int i;
    gboolean found;
    found = FALSE;
    for(i = 0; i < DB_SIZE; i++){
        if(strcmp(text, DB[i][0]) == 0){
            found = TRUE;
            break;
        }
    }
    if(!found)	return 0;
    return 1;
}

void ricerca_aux(GtkWidget *search_button, LighTextEditor *editor){
    if(GTK_IS_TEXT_VIEW(editor->textview) && !GTK_IS_TEXT_TAG_TABLE(table1)) {
        const char *text;
        GtkTextBuffer *buffer;
        GtkTextIter iter;
        GtkTextIter mstart, mend;
        gboolean found;

        text = gtk_entry_get_text(GTK_ENTRY(editor->search));

        /* Popolo il buffer associato alla ricerca del testo*/
        buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor->textview));
       
        /* Ricerca il testo dall'inizio dal buffer */
        gtk_text_buffer_get_start_iter(buffer, &iter);
        found = gtk_text_iter_forward_search(&iter, text, 0, &mstart, &mend, NULL);

        if(GTK_IS_TEXT_TAG_TABLE(table1))	 gtk_text_tag_table_remove(table1,highlight_tag);

        highlight_tag = gtk_text_buffer_create_tag(buffer, NULL,"foreground", "blue", NULL);

        if(GTK_IS_TEXT_TAG_TABLE(table1))	gtk_text_tag_table_add(table1, highlight_tag);

        while(found == 1) {
            found = gtk_text_iter_forward_search(&iter, text, 0, &mstart, &mend, NULL);
            if(found) {	/*Se lo trovo coloro il testo*/
                /* If found, highlight the text. */
                gtk_text_buffer_apply_tag(buffer, highlight_tag, &mstart, &mend);
                iter = mend;
            }
        }
    }
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
*		FINE FUNZIONI DI AUSILIO E PER LA RICERCA DEL TESTO
*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
*       FUNZIONI DI AUSILIO E PER LA COLORAZIONE DEL TESTO
*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

char *cerca_in_db(char *text){
    int i;
    gboolean found;

    found = FALSE;
    for(i = 0; i < DB_SIZE; i++){
        if(strcmp(text, DB[i][0]) == 0){
            found = TRUE;
            break;
        }
    }
    if(!found)  return NULL;
    return g_strdup(DB[i][1]);
}

void colora_il_testo( GtkWidget *text_view, GtkTextIter *arg1){
    GtkTextBuffer *buffer;
    GtkTextIter iter;
    GtkTextIter mstart, mend;
   
    GtkTextTag *highlight_tag2;
   
    /* Ottengo il buffer associato al widget che visualizza il testo */
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
   
    /* Cerco dall'inizio dal buffer. */
    GtkTextIter start;
    char *text;

    int found = 0;

    /* Ottengo la parola dal cursore. */
    start = *arg1;
    if(!gtk_text_iter_backward_word_start(&start))   return;

    text = gtk_text_iter_get_text(&start, arg1);
    g_strstrip(text);

    found = get_pos(text);
    if(found == 0)  return;

    found = gtk_text_iter_forward_search(&start, text, 0, &mstart, &mend, NULL);

    if(GTK_IS_TEXT_TAG_TABLE(table1))   gtk_text_tag_table_remove(table1,highlight_tag2);

    // setto il colore rosso alle parole che matchano con il database
    highlight_tag2 = gtk_text_buffer_create_tag(buffer, NULL,"foreground", "red", NULL);    

    if(GTK_IS_TEXT_TAG_TABLE(table1))   gtk_text_tag_table_add(table1, highlight_tag2);

    if(found) gtk_text_buffer_apply_tag( buffer,highlight_tag2,&mstart,&mend );
}

/*Funzione che ricerca nel testo se ho trovato una parola che matcha e mi printa la riga in cui si trova*/
void testa_sintassi(GtkWidget *text_view, GtkTextIter *arg1){
    GdkWindow *win;
    GtkTextIter start;
    GdkRectangle buf_loc;
    int x, y;
    int win_x, win_y;
    char *text;
    int found=0;

    /* Ottendo il char dal cursore */
    start = *arg1;
    if(!gtk_text_iter_backward_word_start(&start))  return;
    text = gtk_text_iter_get_text(&start, arg1);
    g_strstrip(text);

    /* Ottengo il suggerimento corrispondente */
    found= get_pos(text);
    if(found == 0)  return;

    /* Calcolo la posizione della stringa trovata nella finestra */
    gtk_text_view_get_iter_location(GTK_TEXT_VIEW(text_view), arg1, &buf_loc);

    g_print("%s ---> Trovata colorazione in posizione: %d, %d\n",text,gtk_text_iter_get_line_offset(&start)+1,1+gtk_text_iter_get_line(&start));

    colora_il_testo(text_view,arg1);
}


void inserisci_partentesi_aperta(GtkWidget **punt_widget, GtkWidget *text_view, GtkTextIter *arg1){
    GdkWindow *win;
    GtkTextIter start;
    GdkRectangle buf_loc;
    int x, y;
    int win_x, win_y;
    char *text;
    char *tip_text;

    /* Prendo la parola in prossimità del cursore. */
    start = *arg1;
    if(!gtk_text_iter_backward_word_start(&start))  return;
    text = gtk_text_iter_get_text(&start, arg1);
    g_strstrip(text);

    /* Inserisco la corrispondenza dal database */
    tip_text = cerca_in_db(text);
    if(tip_text == NULL)    return;

    /* Calcolo la posizione della parola corrispondente */
    gtk_text_view_get_iter_location(GTK_TEXT_VIEW(text_view), arg1, &buf_loc);

    gtk_text_view_buffer_to_window_coords(GTK_TEXT_VIEW(text_view), GTK_TEXT_WINDOW_WIDGET,
                                           buf_loc.x, buf_loc.y,
                                           &win_x, &win_y);
    /*Coloro il testo corrispondente*/
    colora_il_testo(text_view,arg1);    

    win = gtk_text_view_get_window(GTK_TEXT_VIEW(text_view), GTK_TEXT_WINDOW_WIDGET);
    gdk_window_get_origin(win, &x, &y);

    /* Distruggi qualsiasi finestra di suggerimento precedente. */
    if(*punt_widget != NULL)   gtk_widget_destroy(GTK_WIDGET(*punt_widget));

    /* 
        Creo una nuova finestra di riferimento e la inserisco nella posizione
        precedentemente calcolata
    */
    *punt_widget = costruttore_finestra(tip_text);
    g_free(tip_text);
    gtk_window_move(GTK_WINDOW(*punt_widget), win_x + x, win_y + y + buf_loc.height);
    gtk_widget_show_all(*punt_widget);
}

void inserisci_parentesi_chiusa(GtkWidget **punt_widget){
    if(*punt_widget != NULL){
        gtk_widget_destroy(GTK_WIDGET(*punt_widget));
        *punt_widget = NULL;
    }
}

void testo_tra_parentesi(GtkTextBuffer *textbuffer, GtkTextIter *arg1, char *arg2, int arg3, gpointer user_data){
     GtkWidget *punt_widget = NULL;
     int spazio;

    if(strcmp(arg2, "(") == 0)  inserisci_partentesi_aperta(&punt_widget, GTK_WIDGET(user_data), arg1);
    if(strcmp(arg2, ")") == 0)  inserisci_parentesi_chiusa(&punt_widget);
    if((strcmp(arg2, " ") && strcmp(arg2,"\n"))==0){   
        if(spazio == 0){   
            testa_sintassi(GTK_WIDGET(user_data), arg1);
            spazio = 1;
        }
    }
    else    spazio = 0;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
*       FINE FUNZIONI DI AUSILIO E PER LA COLORAZIONE DEL TESTO
*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
