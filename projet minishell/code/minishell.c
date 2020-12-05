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

//#define TAILLE_MAX 100 /* taille max d'un mot : à présent définie dans analex.h*/
#define ARGS_MAX 10    /* nombre maximum d'arguments a une commande */

/* Execute la commande donnee par le tableau argv[] dans un nouveau processus ;
 * les deux premiers parametres indiquent l'entree et la sortie de la commande,
 * a rediriger eventuellement.
 * Renvoie le numero du processus executant la commande
 *
 * FONCTION À MODIFIER/COMPLÉTER.
 *votre minishell doit créer un nouveau processus à l’aide de fork(), programmer l’exécution de la commande à l’intérieur du
 *processus fils à l’aide de execvp(), et mettre le processus parent en attente de la fin de son processus fils grâce à wait(), ou waitpid().
 *La fonction executer() crée un nouveau processus fils, exécute la commande dans ce fils et renvoie dans le père le numéro du processus fils.
*/
pid_t execute(int entree, int sortie, char* argv[]) {
  pid_t fpid;
  if ((fpid = fork()) < 0) {
		fprintf(stderr, "Erreur au fork errno:%d %s\n", errno, strerror(errno));
    exit(1);
  }
  else if(fpid == 0) {
    // processus fils
    int pidFils = getpid();

    //déterminer s'il y a des redirections et faire les changements si nécessaires :
    if (entree != 0){
      close(0);
      if(dup2(entree, 0) == -1){
        fprintf(stderr, "Impossible d'effectuer la redirection de l'entrée, errno:%d %s\n",	errno, strerror(errno));
  			exit(1);
      }
      // à présent l'entrée de la commande vient du fichier (de descripteur entree)
    }

    if (sortie != 1){
      close(1);
      if(dup2(sortie, 1) == -1){
        fprintf(stderr, "Impossible d'effectuer la redirection de la sortie, errno:%d %s\n",	errno, strerror(errno));
        exit(1);
      }
      // à présent la sortie de la commande va dans le fichier (de descripteur sortie)
    }

    // argv[0] contient le nom de l'exécutable et argv tous les arguments nécessaires pour lancer la commande
    if (execvp(argv[0], argv) == -1){
      fprintf(stderr, "Impossible d'effectuer la commande via execvp(), errno:%d %s\n",	errno, strerror(errno));
      exit(1);
    }
    exit (pidFils);
  }
  else{
    // processus père : retourne le pid du fils
    return fpid;
  }
}

/* Lit et execute une commande en recuperant ses tokens ;
 * les deux premiers parametres indiquent l'entree et la sortie de la commande ;
 * pid contient le numero du processus executant la commande et
 * background est non nul si la commande est lancee en arriere-plan
 *
 * FONCTION À MODIFIER/COMPLÉTER.
 *On modifie la fonction commande() en lui ajoutant deux arguments, de telle sorte qu’elle reçoive de main() les descripteurs de l’entrée et
 *de la sortie standard (initialement 0 et 1) : commande(int entree, int sortie).
 *Cela va permettre de gérer plus facilement les tubes dans la section suivante. )
 *Les descripteurs entree et sortie sont transmis à la fonction executer(entree, sortie, argv), et dans le
 *fils, avant de faire exec(), on effectue si nécessaire les redirections.
*/
TOKEN commande(int entree, int sortie, pid_t* pid, int* background) {

  TOKEN t;
  char word[TAILLE_MAX];
  char *tabArgs[ARGS_MAX + 1] = {NULL};
  int numArg = 0, status = 0;
  *background = 0; //par défaut, si on ne trouve pas de &, on laisse background à 0
  int fdin = entree, fdout = sortie; // par défaut s'il n'y a pas de redirection
  int fdpipe[2] ; // sert s'il y a des pipe

  while ( (t = getToken(word)) != T_EOF ){

    if(t == T_OVERFLOW){
      fprintf(stderr, "L'argument dépasse la longueur autorisée (%d) : %s\n", TAILLE_MAX, word);
      for(int i=0; i<numArg;i++){
        free(tabArgs[i]);
      }
      *background = 1;
      return T_NL;
    }

    if(t == T_AMPER){
      *background = 1;
      //exécuter en arrière-plan puis continuer à lire car il peut y avoir des commandes derrière qui elles seront exécutées au 1er plan
      *pid = execute(fdin,fdout,tabArgs);
      for(int i=0; i<numArg;i++){
        free(tabArgs[i]);
      }
      return t;
    }

    if(t == T_WORD){
      if(numArg >= ARGS_MAX){
        numArg ++;
        //on continue à lire les arguments même s'ils sont trop nombreux, sinon on aura un problème à la lecture de la prochaine commande
      }

      else {
      // si T_WORD : stocker une copie dans un tableau tabArgs
        tabArgs[numArg] = malloc (TAILLE_MAX * sizeof(char));
        strcpy(tabArgs[numArg], word);
        numArg++;
      }
    }

    if((t == T_NL) || (t == T_SEMI)){
      if(numArg > ARGS_MAX){
        *background = 1;
        fprintf(stderr, "La commande dépasse la longueur autorisée (%d arguments maximum)\n", ARGS_MAX);
        for(int i=0; i<ARGS_MAX;i++){
          free(tabArgs[i]);
        }
      }
      else if(numArg != 0){ // si on essaie de lire à la suite d'un & et qu'il n'y a plus rien,tabArgs est vide donc on ne peut pas exécuter : on se contente de renvoyer t
        // donner le pid du fils au main()
        *pid = execute(fdin,fdout,tabArgs);
        //avant de return, libérer la mémoire de tabArgs
        for(int i=0; i<numArg;i++){
          free(tabArgs[i]);
        }
      }
      else if (numArg == 0) *background = 1; // retour chariot après une mise en arrière-plan : il ne faut pas faire le wait dans le main
      return t;
    }

    if(t == T_LT){
      //pour obtenir le nom de fichier
      if (getToken(word) == T_OVERFLOW){
        fprintf(stderr, "Le nom du fichier dépasse la longueur autorisée (%d) : %s\n", TAILLE_MAX, word);
        for(int i=0; i<numArg;i++){
          free(tabArgs[i]);
        }
        *background = 1;
        return T_NL;
      }
      // ouvrir le fichier d'entrée
      if ((fdin = open(word, O_RDONLY)) == -1) {
        fprintf(stderr, "Impossible ouvrir le fichier d'entrée %s errno:%d %s\n", word, errno, strerror(errno));
        for(int i=0; i<numArg;i++){
          free(tabArgs[i]);
        }
        *background = 1;
        return T_NL;
      }
    }

    if(t == T_GT){
      //pour obtenir le nom de fichier
      if (getToken(word) == T_OVERFLOW){
        fprintf(stderr, "Le nom du fichier dépasse la longueur autorisée (%d) : %s\n", TAILLE_MAX, word);
        for(int i=0; i<numArg;i++){
          free(tabArgs[i]);
        }
        *background = 1;
        return T_NL;
      }
      // ouvrir le fichier de sortie en mode Trunc
      if ((fdout = open(word, O_WRONLY|O_TRUNC|O_CREAT, 0640)) == -1){
        fprintf(stderr, "Impossible ouvrir le fichier de sortie %s errno:%d %s\n", word, errno, strerror(errno));
        for(int i=0; i<numArg;i++){
          free(tabArgs[i]);
        }
        *background = 1;
        return T_NL;
      }
    }

    if(t == T_GTGT){
      // pour obtenir le nom de fichier
      if (getToken(word) == T_OVERFLOW){
        fprintf(stderr, "Le nom du fichier dépasse la longueur autorisée (%d) : %s\n", TAILLE_MAX, word);
        for(int i=0; i<numArg;i++){
          free(tabArgs[i]);
        }
        *background = 1;
        return T_NL;
      }
      // ouvrir le fichier de sortie en mode Append
      if ((fdout = open(word, O_WRONLY|O_APPEND|O_CREAT, 0640)) == -1){
        fprintf(stderr, "Impossible ouvrir le fichier de sortie %s errno:%d %s\n", word, errno, strerror(errno));
        for(int i=0; i<numArg;i++){
          free(tabArgs[i]);
        }
        *background = 1;
        return T_NL;
      }
    }


    if(t == T_BAR){
      //on crée un tube
      if (pipe(fdpipe) == -1) {
    		fprintf(stderr, "Erreur creation du pipe errno:%d %s\n", errno, strerror(errno));
        *background = 1;
    		exit(1);
    	}
      pid_t fpid;
      // pour pouvoir attendre que le fils ait écrit dans l'extrémité écriture du tube avant de vouloir lire dans l'extrémité lecture
      if ((fpid = fork()) < 0) {
    		fprintf(stderr, "Erreur au fork errno:%d %s\n", errno, strerror(errno));
        exit(1);
      }

      //fils
      else if(fpid == 0){
        close(fdpipe[0]); //ferme l'extrémité lecture du tube
        //puis on appelle execute() pour exécuter la partie gauche avec redirection de la sortie dans le tube en écriture.
        *pid = execute(fdin,fdpipe[1],tabArgs);
        kill(getpid(),SIGKILL);
        exit(0);
      }
      //père
      else{
        //attendre que le processus fils ait écrit dans le tube : waitpid sur le pid du fils.
        if ((waitpid(fpid, &status,0)) == -1){
          fprintf(stderr, "Erreur sur le waitpid, errno:%d %s\n", errno, strerror(errno));
          for(int i=0; i<numArg;i++){
            free(tabArgs[i]);
          }
          *background = 1;
          return T_NL;
        }
        close(fdpipe[1]); // ferme l'extrémité écriture du tube
        // Pour traiter la partie droite, on appelle récursivement la fonction commande() avec comme entrée le tube en lecture.
        TOKEN lastToken = commande(fdpipe[0], 1, pid, background);
        return lastToken;
      }
    }
  }
  return T_EOF;
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
    //affiche le prompt en bleu
    printf("\033[1;36mmini-shell> \033[0m");
    fflush(stdout);
  }
}

/* Fonction main
 * FONCTION À MODIFIER/COMPLÉTER.
 */
int main(int argc, char* argv[]) {
	TOKEN t;
  // remplie par commande(), indiquera le pid du fils
	pid_t pid, fid;
  // remplie par commande(), indiquera si la commande est passée ou non en arrière-plan
	int background = 0;
	int status = 0;

	print_prompt(); // affiche le prompt "minishell>"

	while ( (t = commande(0, 1, &pid, &background)) != T_EOF) {
    // Si la commande ne se termine pas par un & (ie *background vaut 0): vous exécutez le processus fils, et dans le parent,
    //vous attendez que le fils se termine avec l'appel systeme wait().
    // gérer l'arrière-plan
    if (background == 0) {
      //wait() : en cas de réussite, l'identifiant du processus fils terminé est renvoyé ; en  cas d'erreur, la valeur de retour est -1.
      while ((fid = wait(&status) != pid)){
        if (fid == -1){
          fprintf(stderr, "Erreur sur le wait, errno:%d %s\n", errno, strerror(errno));
          break;
        }
        printf("[ %d ] %d\n", fid, status);
      }
    }

	  if (t == T_NL){
	    print_prompt();
	  }
	}

	if(is_interactive_shell())
	  printf("\n") ;


	return WEXITSTATUS (status); // Attention à la valeur de retour du wait, voir man wait et la macro WEXITSTATUS
}
