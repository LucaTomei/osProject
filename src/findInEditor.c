/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
							GESTIONE RICERCA TESTO IN EDITOR
*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/*30) Ricerca del testo*/
void cercaTesto(){
	/*Voglio che quanto termino una ricerca con ESC il cursore torni indietro al punto
	in cui si trovava all'inizio, prima della ricerca.*/
	int vecchio_x = Editor.x, vecchio_y = Editor.y;
	int vecchio_offsetCol = Editor.offsetColonna, vecchio_offsetRiga = Editor.offsetRiga;
    
    char *query = promptComando("Cerca: %s (ESC per uscire)", cercaTestoCallback);
    if (query)  free(query);
    else{	/*---->Restore<----*/
    	Editor.x = vecchio_x;
    	Editor.y = vecchio_y;
    	Editor.offsetRiga = vecchio_offsetRiga;
    	Editor.offsetColonna = vecchio_offsetCol;
    }
}
/*31) Mi serve per migliorare la ricerca del testo, per non farla fallire se il file presenta TAB.
  In questo modo converto l'indice di trovato con un indice di char per poi restituirlo e 
  assegnarlo alla variabile x dell'Editor per trovare il punto preciso in cui si trova.*/
/*int cercaAndTabAux(EditorR *row, int rx){
    int curr = 0;
    int i;
    for (i = 0; i < row->size; i++) {
      if (row->chars[i] == '\t')  curr += (STOP_TAB - 1) - (curr % STOP_TAB);
      curr++;
      if (curr > rx) return i;
    }
    return i;
}*/

/*32) Funzione di callback per la ricerca di testo*/
void cercaTestoCallback(char *toFind, int key){
	/*Voglio consentire la ricerca direzionale nel testo tramite le frecce!*/
	static int last_match = -1;	/*Contiene indice di riga su cui si trova l'ultimo result trovato (-1 se no)*/
	static int direction = 1;	/*memorizza la direzione della ricerca (1 in avanti, -1 per indietro)*/

	static int old_riga_color;	/*Li uso per salvare e resettare il coloro dopo l'uscita dalla Ricerca*/
	static char* old_color = NULL;

	if(old_color){
		memcpy(Editor.row[old_riga_color].color, old_color, Editor.row[old_riga_color].effSize);
		free(old_color);
		old_color = NULL;
	}

  	if (key == '\r' || key == '\x1b') {
  		last_match = -1;
  		direction = 1;
  		return; /*Se premo invio o  esc ---> esco*/
    }else if(key == FRECCIA_DESTRA || key == FRECCIA_GIU)	direction = 1;
    else if(key == FRECCIA_SINISTRA || key == FRECCIA_SU)	direction = -1;
    else{
    	last_match = -1;
    	direction = 1;
    }

    if(last_match == -1)	direction = 1;	/*Se ho già trovato roba, inizio dalla riga successiva*/
    int curr = last_match;

    int i;
    for(i = 0; i < Editor.numRighe; i++) {  /*Altrimenti ciclo su tutte le righe del file*/
    	curr += direction;

    	if(curr == -1)	curr = Editor.numRighe -1;	/*Se ho finito le parole che matchano...*/
    	else if(curr == Editor.numRighe)	curr = 0;	/*... ricomincio dall'inizio*/
      	
      	EditorR *row = &Editor.row[curr];
      	/*Uso strstr per verificare se la toFind è una sottostringa della riga corrente*/
      	char *match = strstr(row->effRow, toFind);
      	if (match) {
      		last_match = curr;	/*setto il last a curr in modo che se premo freccia su ricomincio*/
          	Editor.y = curr; /*ho trovato l'indice*/
          	Editor.x = xToRx(row, match - row->effRow);
          	Editor.offsetRiga = Editor.numRighe;  /*Scorro fino alla fine del file*/

          	old_riga_color = curr;
          	old_color = malloc(row->effSize);
          	memcpy(old_color, row->color, row->effSize);

          	memset(&row->color[match - row->effRow], RICERCA, strlen(toFind));	/*Setto il blu per il carattere trovato*/
          	break;
      	}
    }
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
						FINE GESTIONE RICERCA TESTO IN EDITOR
*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
