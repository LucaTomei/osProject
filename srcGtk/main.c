#include <gtk/gtk.h>
#include "utilities.h"

extern char *DB[][2];

int main(int argc, char *argv[]){
    GtkWidget *window, *scrolled_win, *vbox, *barra_ricerca, *toolbar, *find, *barra_menu, *filemenu, *file, *quit, *statusbar ;
    LighTextEditor *editor = g_slice_new(LighTextEditor);

    GtkToolItem *new;
    GtkToolItem *open;
    GtkToolItem *save;
    GtkToolItem *sep;
    GtkToolItem *exit;
    GtkToolItem *undo;
    GtkToolItem *redo;
    GtkToolItem *font;

    GtkTextBuffer *buffer;

    /*	INIZIALIZZAZIONE FINESTRA 	*/
    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "LighTextEditor");
    gtk_container_set_border_width(GTK_CONTAINER(window), 3);
    gtk_widget_set_size_request(window, 600, 500);

    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

    editor->textview = gtk_text_view_new();
    editor->search = gtk_entry_new();
    find = gtk_button_new_from_stock(GTK_STOCK_FIND);
    gtk_entry_set_text(GTK_ENTRY(editor->search), "Cerca");

    g_signal_connect(G_OBJECT(find), "clicked", G_CALLBACK(ricerca_aux), (gpointer) editor);

    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor->textview));
    g_signal_connect(G_OBJECT(buffer), "insert_text", G_CALLBACK(testo_tra_parentesi), editor->textview);

    /*	CREAZIONE 	TOOLBAR*/
    toolbar = gtk_toolbar_new();
    gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_ICONS);
    gtk_container_set_border_width(GTK_CONTAINER(toolbar), 2);

    new = gtk_tool_button_new_from_stock(GTK_STOCK_NEW);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), new, -1);
    g_signal_connect(G_OBJECT(new), "clicked", G_CALLBACK(click_nuovo), (gpointer) editor);

    /*	APERTURA DI UN FILE 	*/
    open = gtk_tool_button_new_from_stock(GTK_STOCK_OPEN);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), open, -1);
    g_signal_connect(G_OBJECT(open), "clicked", G_CALLBACK(click_apri), (gpointer) editor);

    save = gtk_tool_button_new_from_stock(GTK_STOCK_SAVE);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), save, -1);
    g_signal_connect(G_OBJECT(save), "clicked", G_CALLBACK(click_salva), (gpointer) editor);

    sep = gtk_separator_tool_item_new();
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), sep, -1);

    undo = gtk_tool_button_new_from_stock(GTK_STOCK_UNDO);
    gtk_widget_set_name(GTK_WIDGET(undo), "undo");
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), undo, -1);


    redo = gtk_tool_button_new_from_stock(GTK_STOCK_REDO);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), redo, -1);

    g_signal_connect(G_OBJECT(redo), "clicked", G_CALLBACK(annulla_ripeti), undo);
    g_signal_connect(G_OBJECT(undo), "clicked", G_CALLBACK(annulla_ripeti), redo);


    exit = gtk_tool_button_new_from_stock(GTK_STOCK_QUIT);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), exit, -1);
    g_signal_connect(G_OBJECT(exit), "clicked", G_CALLBACK(gtk_main_quit), (gpointer) editor);

    
    /*
		SELEZIONE DELLO STILE DEI FONT
    */
    font = gtk_tool_button_new_from_stock(GTK_STOCK_SELECT_FONT);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), font, -1);
    g_signal_connect(G_OBJECT(font), "clicked", G_CALLBACK(seleziona_font), (gpointer)(editor));


    statusbar = gtk_statusbar_new();

    g_signal_connect(buffer, "changed", G_CALLBACK(aggiorna_barra_di_stato), statusbar);

    g_signal_connect_object(buffer, "mark_set",  G_CALLBACK(aggiornaBarCallback), statusbar, 0);



    barra_menu = gtk_menu_bar_new();
    filemenu = gtk_menu_new();

    file = gtk_menu_item_new_with_label("File");
    quit = gtk_menu_item_new_with_label("Esci");
    g_signal_connect(G_OBJECT(quit), "activate", G_CALLBACK(gtk_main_quit), (gpointer) editor);

    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file), filemenu);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), quit);
    gtk_menu_shell_append(GTK_MENU_SHELL(barra_menu), file);



    scrolled_win = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_win), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scrolled_win), editor->textview);

    barra_ricerca = gtk_hbox_new(FALSE, 5);
    gtk_box_pack_start(GTK_BOX(barra_ricerca), editor->search, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(barra_ricerca), find, FALSE, FALSE, 0);


    /*	VISIBILITÀ FINESTRA */
    vbox = gtk_vbox_new(FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), barra_menu, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), barra_ricerca, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled_win, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), statusbar, FALSE, FALSE, 0);

    gtk_container_add(GTK_CONTAINER(window), vbox);
    gtk_widget_show_all(window); // rendo la finestra fisibile

    gtk_main();
    return 0;
}