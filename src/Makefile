CC=gcc
mioEditor: main.c
	$(CC) main.c termFunc.c editorFunc.c -o mioEditor -Wall -Wextra -pedantic -std=c99
test1:
	./mioEditor TestInProgress/fileDaleggereUnaRiga.txt
test2:	
	./mioEditor TestInProgress/fileTabbato.txt
test3:
	./mioEditor TestInProgress/fileC.c
nuovo:
	./mioEditor
clean: 
	$(RM) mioEditor
.PHONY: mioEditor clean		#Forzo la compilazione ogni volta