#include <stdio.h>
#include <stdlib.h>

#include <editline/readline.h>

int main(int argc,char** argv) {
	puts("Lisp version 0.0.0.0.3");
	puts("Press Ctrl+c to Exit\n");

	while(1) {

		char* input = readline("lispy> ");

		add_history(input);
		
		printf("No you're a %s\n", input);

		free(input);
	}
	return 0;
}

