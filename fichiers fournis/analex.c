/**************************************/
/*              analex.c              */
/**************************************/

#include "analex.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

typedef enum {START, IN_WORD} STATE;
char *specials = "|&;><"; /* les caracteres speciaux qui peuvent terminer un mot */


TOKEN getToken(char* w) {
	STATE state = START;
	int i = 0;
	int c;
	
	while (1) {
		c = getchar();
		switch (state) {
			case START:
				if (c == '|')
					return T_BAR;
				else if (c == '&')
					return T_AMPER;
				else if (c == ';')
					return T_SEMI;
				else if (c == '<')
					return T_LT;
				else if (c == '>') {
					c = getchar() ;
					if (c=='>')
						return T_GTGT ;
					else {
						ungetc(c, stdin) ;
						return T_GT;
					}
				}
				else if (c == '\n')
					return T_NL;
				else if (c == EOF)
					return T_EOF;
				else if (!isspace(c)) { /* on lit un mot */
					i = 0;
					w[i++] = c;
					state = IN_WORD;
				}
					break;
			case IN_WORD:
				if (isspace(c) || strchr(specials, c)) { /* fin d'un mot */
					w[i] = 0;
					ungetc(c, stdin);
					return T_WORD;
				} else
					w[i++] = c;
				break;
			default:
				break;
		}
	}
}
