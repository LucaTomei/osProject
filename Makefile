CC=gcc
mioEditor: mioEditor.c
	$(CC) mioEditor.c -o mioEditor -Wall -Wextra -pedantic -std=c99
test1:
	./mioEditor fileDaleggereUnaRiga.txt
test2:	
	./mioEditor fileTabbato.txt
clean: 
	$(RM) mioEditor
