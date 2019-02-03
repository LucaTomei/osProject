#include <string.h>
#include <stddef.h>
#include <stdlib.h>

#include "highlight.h"
#include "editorFunc.h"
#include "utilities.h"


extern config Editor;

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
GESTIONE HIGHLIT
*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*33) Per la gestione dei colori*/
void aggiornaSintassi(EditorR *row){
    row->color = realloc(row->color, row->effSize);	/*Rialloco la memoria necessaria, poiché il colore occuperà di più*/
    memset(row->color, NORMALE, row->effSize);	/*alloco il 'color' della struct highlight*/

    if(Editor.syntax == NULL)	return;	/*Se non trova nessun estensione file, non serve colorare*/

	  char **parole = Editor.syntax->parole;

	  char *scs = Editor.syntax->commento_singolo;	/*single line comment start*/
  	int scs_len = scs ? strlen(scs) : 0;	/*la utilizzo per tenere traccia del commento, se stringa vuota 0*/

  	char* mcs = Editor.syntax->inizio_multilinea;
  	int mcs_len = mcs ? strlen(mcs) : 0;
  	char* mce = Editor.syntax->fine_multilinea;
  	int mce_len = mce ? strlen(mce) : 0;
  	/*tengo traccia dei separatori che trovo, lo setto a true perché considero l'inizio riga come
  	  separatore*/
  	int prec_sep = 1;
  	int in_string = 0;	/*La utilizzo per tenere traccia se sono dentro una stringa, se si la evidenzio*/
  	
  	/*Tengo traccia se sono all'interno di un commento su più righe <--> Solo per multilinea*/
  	/*in_comment varrà true se la riga precedente è evidenziata*/
  	int in_comment = (row->index > 0 && Editor.row[row->index - 1].is_comment);

  	int i = 0;
  	/*Itero su tutti i caratteri e setto il colore ai numeri se li trovo*/
  	while(i < row->effSize){
  		  char c = row->effRow[i];
  		  /*Setto il colore precedente = al tipo di colore del carattere precedente*/
  		  unsigned char prec_color = (i > 0) ? row->color[i - 1]: NORMALE;

  		  if(scs_len && !in_string && !in_comment){	/*!in_comment xk i commenti multipli non devono essere riconosciuti in quelli singoli*/
  			    /*Verifico se questo carattere è l'inizio di un commento a riga singola*/
  			    if(!strncmp(&row->effRow[i], scs, scs_len)){
    				  /*Setto l'intero resto della linea e interrompo il ciclo evidenziando la sinstassi*/
    				  memset(&row->color[i], COMMENTO, row->effSize - i);
    				  break;
  			    }
  		  }
  		  /*Evidenziamento commento multilinea*/
  		  if (mcs_len && mce_len && !in_string){	/*verifico mcs_len, in_string e mce_len non sono nulli */
      		  if(in_comment) {
	        	    row->color[i] = COMMENTO_MULTILINEA;
	        	    if(!strncmp(&row->effRow[i], mce, mce_len)) {
	          		    memset(&row->color[i], COMMENTO_MULTILINEA, mce_len);
    	          		i += mce_len;
    		          	in_comment = 0;
    		          	prec_sep = 1;
    		          	continue;
	        	    }else{
    	          		i++;
    	          		continue;
	        	    }
	          /*Se non sono in un commento multilinea, verifico se sono all'inizio con strncmp*/
      		  }else if (!strncmp(&row->effRow[i], mcs, mcs_len)){
    		        memset(&row->color[i], COMMENTO_MULTILINEA, mcs_len);
    		        i += mcs_len;
    		        in_comment = 1;	/*imposto in_comment su true e mangio la riga*/
    		        continue;
      		  }
    	  }

  		  if(Editor.syntax->flags & COLORA_STRINGHE){
  			    if(in_string){
  				  row->color[i] = STRINGA;
  				  /*	Se sono in una stringa e il carattere corrente è // 
  					e c'è almeno un altro carattere in quella linea ...*/
  				      if(c == '\\' && i + 1 < row->effSize){
          					row->color[i + 1] = STRINGA;	/*...evidenzio il carattere che viene dopo lo /*/
          					i += 2;	/*e consumo entrambe i caratteri*/
          					continue;
  				      }
  				      if(c == in_string)	in_string = 0;	/*Se c è il carattere di chiusuara, resetto la variabile*/
            				i++;
            				prec_sep = 1;
            				continue;
  			    }else{
        				if(c == '"' || c == '\''){	/*Evidenzio sia le stringhe a virgoletta singola che doppia*/
          					in_string = c;	/*la passo a in_string per sapere quale chiude la stringa*/
          					row->color[i] = STRINGA;	/*Se in_string è impostato, allora posso evidenziare*/
          					i++;	/*consumo la stringa*/
          					continue;
        				}
  			    }
  		  }
  		  if(Editor.syntax->flags & COLORA_NUMERI){
	  		    /*Vedo se trovo anche numeri decimali*/
      			if ((isdigit(c) && (prec_sep || prec_color == NUMERO)) || (c == '.' && prec_color == NUMERO)){
      	  			row->color[i] = NUMERO;
      	  			i++;	/*gli faccio mangiare il carattere successivo*/

      	  			prec_sep = 0;	/*Sto in mezzo alla stringa colorata*/
      	  			continue;	/*continuo il loop*/
      	  	}
  		  }
  		  if (prec_sep) {	/*Controllo prec_sep per assicurarmi che un separatore venga prima della parola chiave*/
      		  int j;
      		  for (j = 0; parole[j]; j++){	/*Eseguo il ciclo per ciascuna parola chiave possibile*/
        		    int pLen = strlen(parole[j]);	/*Per ognuna, memorizzo la sua lunghezza e se secondaria in p2*/
        		    int p2 = parole[j][pLen - 1] == '|';	/*Per ogni parola chiave metto un separatore sia per l'inizio che per la fine*/
		            if(p2) pLen--;	/*La decremento per tenere conto di |*/
		            /*Verifico se la parola chiave è nella posizione corrente del testo e...*/
		            if(!strncmp(&row->effRow[i], parole[j], pLen) &&	/*... se c'è un separatore '\0' => se a fine riga*/
            	         is_separator(row->effRow[i + pLen])) {
          			    memset(&row->color[i], p2 ? MATCHA2 : MATCHA1, pLen);	/*Evidenzio la parola chiave solo una volta*/
		          	    i += pLen;	/*consumo la parola chiave e incremento i*/
		          	    break;	/*interrompo il ciclo interno*/
        		    }
      		  }
      		  if (parole[j] != NULL) {
        		    prec_sep = 0;
        		    continue;
      		  }
    	  }
    		prec_sep = is_separator(c);
    		i++;
  	}
  	/*Imposto is_comment per vedere se il commento multilinea è stato chiuso*/
  	int changed = (row->is_comment != in_comment);	
  	row->is_comment = in_comment;
  	/*Aggiorno la sintassi di tutte le righe che seguono la riga corrente*/
  	if(changed && row->index + 1 < Editor.numRighe)		aggiornaSintassi(&Editor.row[row->index + 1]);
}

/*34) Dice tutto il nome*/
int daSintassiAColore(int color){
	  switch(color){
    		case COMMENTO:
    		case COMMENTO_MULTILINEA:	return 36;	/*Se trovo un commento, lo coloro di ciano*/
    		case MATCHA1:	return 33;	/*Giallo*/
    		case MATCHA2:	return 32;	/*Verde*/
    		case STRINGA:	return 35;	/*Se trovo una stringa, la coloro di magenta*/
    		case NUMERO: 	return 31;	/*Ritorno il rosso*/
    		case RICERCA:	return 34;	/*Ritorno il blu*/
    		default: return 37;	/*Altrimenti bianco*/
	  }
}

/*35) Prende un carattere e ritorna true se è considerato un carattere di separazione*/
int is_separator(int c){
	  return isspace(c) || c == '\0' 
		        || strchr(",.()+-/*=~%<>[];", c) != NULL;	/*strcht ---> vedo se la prima occorrenza 
                  di un carattere nella stringa*/
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
								FINE GESTIONE HIGHLIT
*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
								GESTIONE TIPO DI FILE
*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*36) Cerca di vedere se il file in ingresso corrisponde ad uno dei miei file dichiarati nella struct*/
void selezionaSintassiDaColorare(){
  	Editor.syntax = NULL;	/*Se niente matcha o se non c'è nessun nomeFile*/
  	if (Editor.nomeFile == NULL) return;
  	/*Con strrchr prendo proprio l'ultimo char* della stringa del nomeFile, ovvero l'estensione*/
  	char *ext = strrchr(Editor.nomeFile, '.');	/*Ritorno il puntatore all'ultima occorrenza del carattere in stringa*/
  	
  	for(unsigned int j = 0; j < HLDB_ENTRIES; j++) {	/*Ricerca se l'estensione del file matcha con una delle mie*/
    	struct editorSyntax *s = &HLDB[j];
    	unsigned int i = 0;
    	while (s->filematch[i]) {
      		int is_ext = (s->filematch[i][0] == '.');
      		
      		if((is_ext && ext && !strcmp(ext, s->filematch[i])) ||
          		(!is_ext && strstr(Editor.nomeFile, s->filematch[i]))) {
        		Editor.syntax = s;
        		
        		int rigaFile;
        		for(rigaFile = 0; rigaFile < Editor.numRighe; rigaFile++){
        			aggiornaSintassi(&Editor.row[rigaFile]);
        		}

        		return;
      		}
      		i++;
    	}
  	}
}
