#include <stdio.h>
int main(int argc, char* argv){
	int i;
	for(i = 1; i <= 10; i++){
		switch(i){
			case(10):
				printf("Valgo 10\n");
				break;
			case(1):
				printf("Valgo 10 -9\n");
				break;
			default:
				printf("bo\t");
				break;
		}
	}
	printf("\n"); 
}
