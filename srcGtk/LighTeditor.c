#include <gtk/gtk.h>
typedef struct{
  	GtkWidget *textview, 
  	*search;
}TextEditor;

GtkTextTagTable *table1;
GtkTextTag *highlight_tag;


static void open_clicked (GtkToolButton *cut, TextEditor *editor)
{
  GtkWidget *dialog;
  gint result;
  GtkTextBuffer *buffer;
  gchar *content, *file;
  
  dialog = gtk_file_chooser_dialog_new ("Choose a file ..", NULL,
                                        GTK_FILE_CHOOSER_ACTION_OPEN,
                                        GTK_STOCK_OPEN, GTK_RESPONSE_APPLY,
                                        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                        NULL);
  
  result = gtk_dialog_run (GTK_DIALOG (dialog));
  if (result == GTK_RESPONSE_APPLY)
  {
    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor->textview));
    file = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

    g_file_get_contents (file, &content, NULL, NULL);
    gtk_text_buffer_set_text (buffer, content, -1);
    
    g_free (content);
    g_free (file);
  }

  gtk_widget_destroy (dialog);
}

int main(int argc, char const *argv[]){
	GtkWidget *window, *scrolled_win, *vbox, *searchbar, *toolbar, *find, *menubar, *filemenu, *file, *quit, *statusbar ;
  	TextEditor *editor = g_slice_new (TextEditor);
  	
  	GtkToolItem *new;
  	GtkToolItem *open;
  	GtkToolItem *save;
  	GtkToolItem *sep;
  	GtkToolItem *exit;
  	GtkToolItem *undo;
  	GtkToolItem *redo;
  	GtkToolItem *font1, *font2, *font3;

  	GtkTextBuffer *buffer;

  	/*	INIZIALIZZAZIONE FINESTRA 	*/
  	gtk_init(&argc, &argv);	// inizializzazione prima di invocazione funzioni
  	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window), "LighTextEditor");
	gtk_container_set_border_width (GTK_CONTAINER (window), 3);
	gtk_widget_set_size_request (window, 600, 500);
	 
	g_signal_connect (G_OBJECT (window), "destroy", G_CALLBACK (gtk_main_quit), NULL);

	/*	CREAZIONE 	TOOLBAR*/
	toolbar = gtk_toolbar_new();
  	gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_ICONS);
  	gtk_container_set_border_width(GTK_CONTAINER(toolbar), 2);


	/*	APERTURA DI UN FILE 	*/
	open = gtk_tool_button_new_from_stock(GTK_STOCK_OPEN);
  	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), open, -1);
  	g_signal_connect (G_OBJECT (open), "clicked", G_CALLBACK (open_clicked), (gpointer) editor);

	/*	VISIBILITÀ FINESTRA */
	vbox = gtk_vbox_new (FALSE, 5);
	gtk_box_pack_start (GTK_BOX (vbox), toolbar, FALSE, FALSE, 0);
  	gtk_container_add (GTK_CONTAINER (window), vbox);
  	gtk_widget_show_all (window);
  
  	gtk_main();	// rendo la finestra fisibile
	return 0;
}