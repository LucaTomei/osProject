#include <unistd.h> // read, write, close
#include <fcntl.h>  // open
#include <stdlib.h> // exit
#include <stdio.h>  // perror
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <limits.h>	// PATH_MAX

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
	/*Prendo il nome della directory in cui sono*/
	char cwd[PATH_MAX];	
	char* current_dir = getcwd(cwd, sizeof(cwd));
	
	int ret;

	char* init = "[Desktop Entry]\nVersion=1.0\nComment=The Best Text Editor\nName=LighTextEditor\nExec=gnome-terminal -e \"bash -c ";


	/*	CREAZIONE DIRECTORY e download del file	*/
	struct passwd *pw = getpwuid(getuid());
	char *homedir = pw->pw_dir;		// home directory

	char* createDir = append(homedir, "/LighTextEditor");
	
	struct stat st = {0};
	if (stat(createDir, &st) == -1) {
    	ret = mkdir(createDir, 0700);
    	if(ret != 0)	handle_error("Impossibile creare directory in $HOME");
	}

	char* fileInDir = append(homedir, "/LighTextEditor/LighTextEditor");
	if(access(fileInDir, F_OK) != -1 ) {
		handle_error("Termino l'installazione: Il file risulta già presente!");
	}
	else {
		ret = chdir(createDir);
		if(ret != 0)	handle_error("Impossibile spostarmi in $HOME");
		ret = system("wget https://github.com/LucaTomei1995/osProject/raw/master/src/LighTextEditor -O LighTextEditor");
    	if(ret != 0)	handle_error("Impossibile scaricare il file");

    	// Setto il permesso di esecuzione al file
    	ret = system("chmod a+x LighTextEditor");
    	if(ret != 0)	handle_error("Impossibile cambiare permesso al file");

    	int ret = system("wget https://suaymac.com/uploads/gallery/album_2/gallery_1_2_75159.jpg -O icon.png");
    	if(ret != 0)	handle_error("Errore nel Download dell'Icona");
	}

	/*	CREAZIONE FILE DESKTOP */
	// Suppongo che il file sia posizionato in home
	// Prendo la directory home dell'utente
	
	char* lightExec = append(homedir, "/LighTextEditor/LighTextEditor\"\nIcon=");	// Nome eseguibile in $HOME

	char* tmp1 = append(init, lightExec);
	char* tmp2 = append(tmp1, homedir);
	char* done = append(tmp2, "/LighTextEditor/icon.png\nTerminal=true\nType=Application\nCategories=Application;");

	// Prima di creare il file mi sposto nella directory in cui sono
	ret = chdir(current_dir);
	if(ret != 0)	handle_error("Impossibile spostarmi nella directory di partenza");


	/*printf("%s\n", done);*/
	FILE* f = fopen(nomeFile, "w");		// <---- Il file non è in HOME!!!
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
	int ret;

	/*Prendo il nome della directory in cui sono*/
	char cwd[PATH_MAX];	
	char* current_dir = getcwd(cwd, sizeof(cwd));

  	char* file_name = "LighTextEditor.desktop";
	create_desktop_file(file_name);

	ret = system("tput reset");
	if(ret != 0)	handle_error("Errore");

	printf("Inserisci la password di root per procedere con l'installazione\n");
	ret = system("sudo cp LighTextEditor.desktop /usr/share/applications/");
	if(ret != 0)	printf("Impossibile creare il file\n");

	/*Elimino il file desktop <----------TODO*/
	ret = chdir(current_dir);
	if(ret != 0)	handle_error("Impossibile spostarmi nella directory di partenza");

	ret = remove(file_name);
	if(ret != 0)	handle_error("Impossibile eliminare il file");

	ret = system("tput reset");
	if(ret != 0)	handle_error("Errore");
	return 0;
}
