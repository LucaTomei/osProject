#include <gtk/gtk.h>
#include "utilities.h"

/*Costruttore della finestra principale*/
GtkWidget* costruttore_finestra(char *tip){
    GtkWidget *win;
    GtkWidget *label;
    GtkWidget *eb;
    GdkColormap *cmap;
    GdkColor color;
    PangoFontDescription *pfd;

    win = gtk_window_new(GTK_WINDOW_POPUP);
    gtk_container_set_border_width(GTK_CONTAINER(win), 0);

    eb = gtk_event_box_new();
    gtk_container_set_border_width(GTK_CONTAINER(eb), 1);
    gtk_container_add(GTK_CONTAINER(win), eb);

    label = gtk_label_new(tip);
    gtk_container_add(GTK_CONTAINER(eb), label);

    // Setto il font una volta trovata la corrispondenza dell'hilight
    pfd = pango_font_description_from_string("Sans Bold Italic Condensed 12px"); 
    gtk_widget_modify_font(label, pfd);

    cmap = gtk_widget_get_colormap(win);
    color.red = 0;
    color.green = 0;
    color.blue = 0;
    if(gdk_colormap_alloc_color(cmap, &color, FALSE, TRUE))	gtk_widget_modify_bg(win, GTK_STATE_NORMAL, &color);
    else	g_warning("Allocazione del colore fallita!\n");

    cmap = gtk_widget_get_colormap(eb);
    color.red = 65535;
    color.green = 65535;
    color.blue = 45535;
    if(gdk_colormap_alloc_color(cmap, &color, FALSE, TRUE))	gtk_widget_modify_bg(eb, GTK_STATE_NORMAL, &color);
    else	g_warning("Allocazione del colore fallita!\n");
    return win;
}

/*
    FINESTRA DI SELEZIONE FONT
*/
void seleziona_font(GtkToolButton *cut, LighTextEditor *editor){
    GtkResponseType result;

    GtkWidget *dialog = gtk_font_selection_dialog_new("Seleziona Font");
    result = gtk_dialog_run(GTK_DIALOG(dialog));

    if(result == GTK_RESPONSE_OK || result == GTK_RESPONSE_APPLY){

        PangoFontDescription *descr_font;
        char *nome_font = gtk_font_selection_dialog_get_font_name(GTK_FONT_SELECTION_DIALOG(dialog));

        descr_font = pango_font_description_from_string(nome_font);

        gtk_widget_modify_font(GTK_WIDGET(editor->textview), descr_font);

        g_free(nome_font);
    }
    gtk_widget_destroy(dialog);
}





void annulla_ripeti(GtkWidget *widget, gpointer item){
     int count = 2;
    const char *name = gtk_widget_get_name(widget);

    if(strcmp(name, "undo")) count++;
    else count--;

    if(count < 0) {
        gtk_widget_set_sensitive(widget, FALSE);
        gtk_widget_set_sensitive(item, TRUE);
    }

    if(count > 5) {
        gtk_widget_set_sensitive(widget, FALSE);
        gtk_widget_set_sensitive(item, TRUE);
    }
}

/*
    BARRA DI STATO CHE MOSTRA LA POSIZIONE DEL TESTO
*/
void aggiorna_barra_di_stato(GtkTextBuffer *buffer, GtkStatusbar *statusbar){
    char *msg;
    int row, col;
    GtkTextIter iter;

    gtk_statusbar_pop(statusbar, 0);

    gtk_text_buffer_get_iter_at_mark(buffer, &iter, gtk_text_buffer_get_insert(buffer));

    row = gtk_text_iter_get_line(&iter);
    col = gtk_text_iter_get_line_offset(&iter);

    msg = g_strdup_printf("Colonna %d Riga %d", col+1, row+1);

    gtk_statusbar_push(statusbar, 0, msg);
    g_free(msg);
}

void aggiornaBarCallback(GtkTextBuffer *buffer, const GtkTextIter *new_location, GtkTextMark *mark, gpointer data){
    aggiorna_barra_di_stato(buffer, GTK_STATUSBAR(data));
}