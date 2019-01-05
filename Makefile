CC=gcc
mioEditor: mioEditor.c
	$(CC) mioEditor.c -o mioEditor -Wall -Wextra -pedantic -std=c99
clean: 
	$(RM) mioEditor