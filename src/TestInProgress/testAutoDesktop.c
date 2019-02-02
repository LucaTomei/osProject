#include <unistd.h> // read, write, close
#include <fcntl.h>  // open
#include <stdlib.h> // exit
#include <stdio.h>  // perror
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>

#define handle_error(msg)	do{perror(msg); exit(1);}while(0);

char* append(char* string1, char* string2){
	size_t dim = (strlen(string1)) + (strlen(string2));
	char *ret = (char*)malloc(dim * sizeof(char) + 1);
    strcpy(ret, string1);
    strcat(ret, string2);
	return ret;
}

/*
	*	Crea un cartella in $HOME di nome LighTextEditor
	*	Al suo interno mette il file compilato del programma che scarico dal mio GitHub
	*	Crea un file Desktop
	* 	Scarica l'icona del file
*/
void create_desktop_file(char* nomeFile){
	char* init = "[Desktop Entry]\nVersion=1.0\nComment=The Best Text Editor\nName=LighTextEditor\nExec=gnome-terminal -e \"bash -c ";


	/*	CREAZIONE DIRECTORY e download del file	*/
	struct passwd *pw = getpwuid(getuid());
	char *homedir = pw->pw_dir;		// home directory

	char* createDir = append(homedir, "/LighTextEditor");
	
	struct stat st = {0};
	if (stat(createDir, &st) == -1) {
    	int ret = mkdir(createDir, 0700);
    	if(ret != 0)	handle_error("Impossibile creare directory in $HOME");
	}

	char* fileInDir = append(homedir, "/LighTextEditor/LighTextEditor");
	if(access(fileInDir, F_OK) != -1 ) {
		handle_error("Termino l'installazione: Il file risulta già presente!");
	}
	else {
		int ret1 = chdir(createDir);
		if(ret1 != 0)	handle_error("Impossibile spostarmi in $HOME");
		int ret2 = system("wget https://github.com/LucaTomei1995/osProject/raw/master/src/LighTextEditor -O LighTextEditor");
    	if(ret2 != 0)	handle_error("Impossibile scaricare il file");

    	// Setto il permesso di esecuzione al file
    	int ret3 = system("chmod a+x LighTextEditor");
    	if(ret3 != 0)	handle_error("Impossibile cambiare permesso al file");

    	int ret 4 = system("wget https://raw.githubusercontent.com/LucaTomei1995/osProject/master/src/TestInProgress/AutoIstallazione/icon.png -O icon.png");
    	if(ret4 != 0)	handle_error("Errore nel Download dell'Icona");
	}

	/*	CREAZIONE FILE DESKTOP */
	// Suppongo che il file sia posizionato in home
	// Prendo la directory home dell'utente
	
	char* lightExec = append(homedir, "/LighTextEditor/LighTextEditor\"\nIcon=");	// Nome eseguibile in $HOME

	char* tmp1 = append(init, lightExec);
	char* tmp2 = append(tmp1, homedir);
	char* done = append(tmp2, "/LighTextEditor/icon.png\nTerminal=true\nType=Application\nCategories=Application;");

	printf("%s\n", done);
	FILE* f = fopen(nomeFile, "w");
	if(f == NULL)	handle_error("\n\nImpossibile creare il file in scrittura\n\n");
	fprintf(f, "%s", done);
	fclose(f);

	free(tmp1);
	free(tmp2);
	free(done);
	free(createDir);
	free(fileInDir);
	free(lightExec);
}


int main(int argc, char const *argv[]){
	create_desktop_file("LighTextEditor.desktop");
	return 0;
}
