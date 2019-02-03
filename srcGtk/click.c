#include <gtk/gtk.h>
#include "utilities.h"
/* 	Verifico se l'utente desidera creare un nuovo documento. Se è così, cancello
	tutto il testo dal buffer.*/
void click_nuovo(GtkToolButton *cut, LighTextEditor *editor){
    GtkWidget *dialog;
    GtkTextBuffer *buffer;
    int result;

    dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL,GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
                                     "Tutti i cambiamenti verranno persi. Desideri Continuare?");

    result = gtk_dialog_run(GTK_DIALOG(dialog));
    if(result == GTK_RESPONSE_YES){
        buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor->textview));
        /*Svuoto il buffer*/
        gtk_text_buffer_set_text(buffer, "", -1);
    }
    gtk_widget_destroy(dialog);
}

/* Sostituisco il contenuto del buffer corrente con il contenuto del file */
void click_apri(GtkToolButton *cut, LighTextEditor *editor){
    GtkWidget *dialog;
    int result;
    GtkTextBuffer *buffer;
    char *content, *file;

    dialog = gtk_file_chooser_dialog_new("Scegli un File", NULL,
                                          GTK_FILE_CHOOSER_ACTION_OPEN,
                                          GTK_STOCK_OPEN, GTK_RESPONSE_APPLY,
                                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                          NULL);

    result = gtk_dialog_run(GTK_DIALOG(dialog));
    if(result == GTK_RESPONSE_APPLY){
        buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor->textview));
        file = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

        g_file_get_contents(file, &content, NULL, NULL);
        gtk_text_buffer_set_text(buffer, content, -1);

        g_free(content);
        g_free(file);
    }

    gtk_widget_destroy(dialog);
}

/* Salvo il conenuto del buffer nel file*/
void click_salva(GtkToolButton *cut, LighTextEditor *editor){
    GtkWidget *dialog;
    int result;
    GtkTextBuffer *buffer;
    char *content, *file;
    GtkTextIter start, end;

    dialog = gtk_file_chooser_dialog_new("Salva il File", NULL,
                                          GTK_FILE_CHOOSER_ACTION_SAVE,
                                          GTK_STOCK_SAVE, GTK_RESPONSE_APPLY,
                                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                          NULL);

    result = gtk_dialog_run(GTK_DIALOG(dialog));
    if(result == GTK_RESPONSE_APPLY){
        buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor->textview));
        file = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

        gtk_text_buffer_get_bounds(buffer, &start, &end);
        content = gtk_text_buffer_get_text(buffer, &start, &end, TRUE);

        g_file_set_contents(file, content, -1, NULL);
        g_free(content);
        g_free(file);
    }
    
    gtk_widget_destroy(dialog);
}

/* Copio la roba copiata negli appunti e la rimuovo dal buffer */
void click_taglia(GtkToolButton *cut, LighTextEditor *editor){
    GtkClipboard *clipboard;
    GtkTextBuffer *buffer;

    clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor->textview));
    gtk_text_buffer_cut_clipboard(buffer, clipboard, TRUE);
}

/* Copio la roba selezionata negli appunti */
void click_copia(GtkToolButton *copy, LighTextEditor *editor){
    GtkClipboard *clipboard;
    GtkTextBuffer *buffer;

    clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor->textview));
    gtk_text_buffer_copy_clipboard(buffer, clipboard);
}

/* Cancello tutto il testo selezionato e incollo la roba degli appunti */
void click_incolla(GtkToolButton *paste, LighTextEditor *editor){
    GtkClipboard *clipboard;
    GtkTextBuffer *buffer;

    clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor->textview));
    gtk_text_buffer_paste_clipboard(buffer, clipboard, NULL, TRUE);
}

/* Cerca una stringa di testo dalla posizione corrente del cursore */
void click_cerca(GtkButton *cut, LighTextEditor *editor){
    const char *find;
    char *output;

    GtkWidget *dialog;
    GtkTextBuffer *buffer;
    GtkTextIter start, begin, end;
    gboolean success;

    int i = 0;
    find = gtk_entry_get_text(GTK_ENTRY(editor->search));
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor->textview));
    gtk_text_buffer_get_start_iter(buffer, &start);
    success = gtk_text_iter_forward_search(&start,(char*) find, 0, &begin, &end, NULL);
    while(success){
        gtk_text_iter_forward_char(&start);
        success = gtk_text_iter_forward_search(&start,(char*) find, 0,&begin, &end, NULL);
        start = begin;
        i++;
    }
    output = g_strdup_printf("La stringa '%s' è stata trovata %i volte!", find, i);
    dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, output, NULL);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    g_free(output);
}