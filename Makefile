CC=gcc
mioEditor: mioEditor.c
	$(CC) mioEditor.c -o mioEditor -Wall -Wextra -pedantic -std=c99
test:
	./mioEditor fileDaleggereUnaRiga.txt
clean: 
	$(RM) mioEditor
