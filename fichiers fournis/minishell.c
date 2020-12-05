/**************************************/
/*            minishell.c             */
/**************************************/

#include<unistd.h>
#include<stdio.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/types.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include "analex.h"

#define TAILLE_MAX 100 /* taille max d'un mot */
#define ARGS_MAX 10    /* nombre maximum d'arguments a une commande */

/* Execute la commande donnee par le tableau argv[] dans un nouveau processus ;
 * les deux premiers parametres indiquent l'entree et la sortie de la commande,
 * a rediriger eventuellement.
 * Renvoie le numero du processus executant la commande
 *
 * FONCTION À MODIFIER/COMPLÉTER.
 *
*/
pid_t execute(int entree, int sortie, char* argv[]) {

  /* ... */

  return 0; 
}

/* Lit et execute une commande en recuperant ses tokens ;
 * les deux premiers parametres indiquent l'entree et la sortie de la commande ;
 * pid contient le numero du processus executant la commande et
 * background est non nul si la commande est lancee en arriere-plan
 *
 * FONCTION À MODIFIER/COMPLÉTER.
*/
TOKEN commande(int entree, int sortie, pid_t* pid, int* background) {

  /* ... */

  TOKEN t; 
  char word[TAILLE_MAX];  
  t = getToken(word);
  return t; 
}

/* Retourne une valeur non-nulle si minishell est en train de s'exécuter en mode interactif, 
 * c'est à dire, si l'affichage de minishell n'est pas redirigé vers un fichier.
 * (Important pour les tests automatisés.)
 *
 * NE PAS MODIFIER CETTE FONCTION.
 */
int is_interactive_shell(){
  return isatty(1);
}

/* Affiche le prompt "minishell>" uniquement si l'affichage n'est pas redirigé vers un fichier. 
 * (Important pour les tests automatisés.)
 *
 * NE PAS MODIFIER CETTE FONCTION.
 */
void print_prompt(){
  if(is_interactive_shell()){ 
    printf("mini-shell>");
    fflush(stdout);
  }  
}

/* Fonction main 
 * FONCTION À MODIFIER/COMPLÉTER.
 */
int main(int argc, char* argv[]) {
	TOKEN t;
	pid_t pid, fid;
	int background = 0;
	int status = 0;


	print_prompt(); // affiche le prompt "minishell>" 
	
	while ( (t = commande(0, 1, &pid, &background)) != T_EOF) {

	  /* Boucle principale: à modifier/compléter */	  
	  
	  if (t == T_NL) {
	    print_prompt(); 
	  }
	}

	if(is_interactive_shell())
	  printf("\n") ;

	return status; // Attention à la valeur de retour du wait, voir man wait et la macro WEXITSTATUS
}
