#ifndef FINDTEXT_H
#define FINDTEXT_H
char *cerca_in_db(char *text);
void colora_il_testo( GtkWidget *text_view, GtkTextIter *arg1);
void testa_sintassi(GtkWidget *text_view, GtkTextIter *arg1);
void inserisci_partentesi_aperta(GtkWidget **punt_widget, GtkWidget *text_view, GtkTextIter *arg1);
void inserisci_parentesi_chiusa(GtkWidget **punt_widget);
void testo_tra_parentesi(GtkTextBuffer *textbuffer, GtkTextIter *arg1, char *arg2, int arg3, gpointer user_data);
#endif