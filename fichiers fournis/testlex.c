/**************************************/
/*             testlex.c              */
/**************************************/

#include <stdio.h>
#include "analex.h"

#define TAILLE_MAX 100

char *libToken[] = {
	"T_WORD",
	"T_BAR",
	"T_SEMI",
	"T_AMPER",
	"T_LT",
	"T_GT",
	"T_GTGT",
	"T_NL",
	"T_EOF"
};

int main(int argc, char* argv[]) {
	TOKEN t;
	char w[TAILLE_MAX];
	while ( (t = getToken(w)) != T_EOF ) {
		printf(" Token : %s", libToken[t]) ;
		if (t==T_WORD)
			printf(", valeur: %s\n", w);
		else 
			printf ("\n");
	}
	return 0 ;
}
