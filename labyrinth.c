#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>



#define BUFSIZE 256
#define MUR 6
#define MUR_AJOUT 7
#define POSITION_INITIALE 9
#define POSITION_TRESOR 1
#define POSITION_PRECEDENTE 5
#define CHEMIN_EMPRUNTABLE 3
#define CHEMIN_INEXPLORER 2
#define MUR_RETOUR 8
#define TAILLE_MAX 10000



/*
	Ponçot Cédric
	Courvoisier Nicolas
	Groupe TP1A

	Stratégie mise en place pour trouver le trésor et revenir à la position initiale :
	
	-Créer un tableau de la taille du labyrinth +2 pour les bordures extérieures
	-Initialiser le tableau avec des 0 pour les parties du labyrinth inexplorer
		et des 6 pour les bordures extérieurs que l'on considere comme des murs 
		au même titre que ceux dans le labyrinth
	-Mettre à 3 les positions où l'on peut aller 
	-Mettre à 2 les chemins inexplorer
	-Mettre à 1 la position du trésor et se déplacer en sa direction en gardant un ordre
		de priorité
	-Mettre à 5 la position précédente
	-Condamné les impasses en ajoutant de nouveau mur que l'on affecte au nombre 7
	-légère part de random pour éviter de boucler sur les mêmes positions et changer
		ensuite de chemin
	
	-Une fois le trésor trouvé
	-On cherche le chemin le plus cours pour retourner à la position initiale le plus
		rapidement possible
	-On met des 8 derrière notre passage pour condamner nos pas précédent
	-On arrive à la position initiale
	
	
	Problèmes rencontrés :
	
	-On a essayé pour beaucoup de seed manuellement, à chaque fois cela marche
	-On a essayé avec le script, on a eu des problèmes des fois celui-ci s'arrete
		sans qu'on comprenne vraiment pourquoi. il nous retourne l'erreur :
		./Script_labyrinth.sh: arithmetic expression: expecting primary; "11154+".
	On a pas eu ce problème à chaque fois, en lançant le script ça le faisait que pour 
	certain seed, on relançait ensuite le script et les seed en questions qui bloquaient
	précédement passaient sur ce nouveau lancement, c'était assez étrange.
	Et on a essayé de passer manuellement des seeds qui bloquaient avec le script,
	et donc manuellement celle-ci passait. On ne comprend pas trop ce problème.
	
	-Autres porblèmes, parfois on lance l'algo, il trouve bien toujours le trésor. Mais ne
	revient pas à la position initiale, on relance exactement le même seed et trouve toujours
	le trésor et maintenant revient à la position initiale. On n'a pas trouvé de solution
	à ce problème (cela arrive quand même que très rarement).
*/



/* Prototypes */

void afficheCaseAdj(char* buf);
int recupCasePrio(int *data,int x,int y,int width,int priox,int prioy);
void Deplace(int a);
int matrice2tab(int width, int x,int y);
void remplissageTab(int *data,char *buf,int x,int y,int width);
void AfficheTab(int *data,int width,int height,int x,int y);
void inittab(int *data, int width, int height);
bool impasse(int res,int east,int west,int north,int south);
void remisea3(int *data,int width,int x,int y);
int min(int east,int west,int south,int north,int priox,int prioy);
int get_prio_x(int x,int xt);
int get_prio_y(int y,int yt);
void reset8(int *data,int width,int height);
int taille(int *data,int x,int y,int xi,int yi, int width,int height);
int minimum(int a, int b , int c ,int d);
void retour(int *data,int x,int y,int xi,int yi, int width,int height);



/* Fonction main */
/* Complexité :
   -initab :
   -for qui fait n tour de boucles et possède :
       -get_prio_x : o(1)
       -get_prio_y : o(1)
       -remplissageTab : o(8)
       -recupCasePrio : o(1)
       -remisea3 : o(9)
       -Deplace : o(1)
   -retour : o(n*h*l)
   -AfficheTab : o(h*l)
   => complexité total :
   		= n*(o(1) + o(1) + o(8) + o(1) + o(9) o(1)) + o(n*h*l) + o(h*l)
   		Donc = o(n*h*l) */
int main() {
	setbuf(stdout, NULL);
	char buf[BUFSIZE];
	// get the width
	fgets(buf, BUFSIZE, stdin);
	int width = atoi(buf);
	// get the height
	fgets(buf, BUFSIZE, stdin);
	int height = atoi(buf);
	/* Ajoute 2 à la hauteur et la largeur du labyrinth 
	   pour faire apparaitre les bordures de celui-ci dans le tableau */
	width=width+2;
	height=height+2;
	/* Alloue l'espace mémoire pour le tableau du labyrinth */
	int *data=calloc(width*height,sizeof(int));
	// get the initial x
	fgets(buf, BUFSIZE, stdin);
	int x = atoi(buf)+1;
	// get the initial y
	fgets(buf, BUFSIZE, stdin);
	int y = atoi(buf)+1;
	// get the treasure x
	fgets(buf, BUFSIZE, stdin);
	int xt = atoi(buf)+1;
	// get the treasure y
	fgets(buf, BUFSIZE, stdin);
	int yt = atoi(buf)+1;
	
	// initialize the state of the game
	/* Affecte la valeur de 1 à la position du trésor */
	data[matrice2tab(width,xt,yt)]=POSITION_TRESOR;
	/* Initialise le tableau */
	inittab(data,width,height);
	/* Initialise les positions initiales */
	int xi = x;
	int yi = y;
	/* Initialise le compteur de pas */
	int cmp=0;
	for (;;) {
		/* Retourne la priorité de déplacement pour aller jusqu'au trésor */
		int priox=get_prio_x(x,xt);
		int prioy=get_prio_y(y,yt);
		/* ++cmp compte le nombre de tour de boucle pour savoir 
		   le nombre de pas jusqu'au trésor */
		++cmp;
		// get the adjacent cells
		fgets(buf, BUFSIZE, stdin);
		/* Remplit le tableau, pour déterminer les murs et passages */
		remplissageTab(data,buf,x,y,width);
		/* Regarde ou se déplacer en priorité */
		int a=recupCasePrio(data,x,y,width,priox,prioy);
		/* Met à 3 les chemins enmpruntable */
		remisea3(data,width,x,y);
		/* Nouvelle affectation pour la position précédente */
		if(data[matrice2tab(width,x,y)]!=MUR && data[matrice2tab(width,x,y)]!=MUR_AJOUT){
			/* Met à 5 la position précédente */
			data[matrice2tab(width,x,y)]=POSITION_PRECEDENTE;
		}
		// update the state of the game
		// send the new direction
		Deplace(a);// or "SOUTH" or "EAST" or "WEST"
		/* Incrémente ou decrémente pour mettre à jour la position actuelle */
		switch (a){
			case 1:--y;
				break;
			case 3:--x;
				break;
			case 4: ++x;
				break;
			case 6: ++y;
				break;
		}
		/* Détecte quand on a trouvé le trésor et commence le trajet retour */
		if (x == xt && y == yt) {
			/* Affiche le nombre de pas jusqu'au trésor */
			fprintf(stderr,"Le nombre de coup est : %d.",cmp);
			/* Met un 9 dans le tableau sur la position initiale */
			data[matrice2tab(width,xi,yi)]=POSITION_INITIALE;
			/* Appel la fonction de reout */
			retour(data,x,y,xi,yi,width,height);
			/* Stop la boucle une fois le retour effectué */
			break;
		}
		
		fgets(buf, BUFSIZE, stdin);
		if ((strcmp(buf, "END\n") == 0)) {
			break;
		}
	}
	/* Affiche le tableau final pour constater les déplacements effectués et le trajet retour */
	AfficheTab(data, width, height, x, y);
	/* Libere la mémoire */
	free(data);
	return 0;
}



/* Fonctions */

/* Fonction qui met à 3 un chemin exploré que l'on peut emprunter */
/* Double boucle for l'une dans l'autre qui font au maximum 3 boucle chacune
   donc 3*3 = 9 la complexité est o(9) */
void remisea3(int *data,int width,int x,int y){
	for(int i=x-1;i<x+2;i++){
		for(int j=y-1;j<y+2;j++){
			if(data[matrice2tab(width,i,j)]==POSITION_PRECEDENTE){
				/* Met un 3 quand le chemin est empruntable sur la position précédente */
				data[matrice2tab(width,i,j)]=CHEMIN_EMPRUNTABLE;
			}
		}
	}	
}

/* Fonctions qui initialise notre tableau, 
   qui permet de mettre des murs sur le périmetre du rectangle */
/* Deux boucles for indépendantes :
   -l'une parcourt une fois la hauteur, donc o(h)
   -l'autre parcourt une fois la largeur, donc o(l)
   -complexité totale : o(h) + o(l) */
void inittab(int *data, int width, int height) {
	for (size_t i = 0; i < height; ++i) {
		data[matrice2tab(width, 0, i)] = MUR;
		data[matrice2tab(width, width - 1, i)] = MUR;
	}
	for (size_t i = 0; i < width ; ++i) {
		data[i] = MUR;
		data[width*height - i - 1] = MUR;
	}
}

/* Cherche la priorité entre deux points x */
/* Comparaison entre deux valeurs, donc complexité en o(1) */
int get_prio_x(int x,int xt){
	if(x>xt)return 1;
	return 0;

}

/* Cherche la priorité entre deux points y */
/* Comparaison entre deux valeurs, donc complexité en o(1) */
int get_prio_y(int y,int yt){
	if(y>yt)return 3;
	return 2;

}

/* Remplit le tableau en fonction de l'exploration du labyrinth */
/* Tous les if ne sont que de simple comparaisons avec affectation
   On a une boucle for qui fait 8 tours de boucles donc o(8) */
void remplissageTab(int *data,char *buf,int x,int y,int width){
	/* Met un 3 quand c'est un chemin emprunter */
	if(data[matrice2tab(width,x,y)]!=MUR){
			data[matrice2tab(width,x,y)]=CHEMIN_EMPRUNTABLE;
	}
	/* Remplit les 8 cases adjacentes à la position actuelle */
	for(int i=0;i<8;++i){
		int ytemp=y;
		int xtemp=x;
		if(i<3){
			ytemp--;
		}
		else{
			if(i>4){
				ytemp++;
			}
		}
		if(i==0||i==3||i==5){
			xtemp--;
		}
		else{
			if(i==2||i==4||i==7){
				xtemp++;
			}
		}
		if(buf[i]=='_'){
			int tmp=data[matrice2tab(width,xtemp,ytemp)];
			if(tmp!=CHEMIN_EMPRUNTABLE&&tmp!=MUR&&tmp!=POSITION_PRECEDENTE&&tmp!=MUR_AJOUT){
				/* Met un 2 quand on a un chemin inexploré */
				data[matrice2tab(width,xtemp,ytemp)]=CHEMIN_INEXPLORER;
			}
		}
		else{
			if(buf[i]=='W'){
				if(data[matrice2tab(width,xtemp,ytemp)]!=MUR_AJOUT){
					/* Met un 6 quand on a mur */
					data[matrice2tab(width,xtemp,ytemp)]=MUR;
				}
			}
			else{
				/* Met un 1 à la position du trésor */
				data[matrice2tab(width,xtemp,ytemp)]=POSITION_TRESOR;
			}
		}
	}
}

/* Fonction qui affiche le tableau */
/* Double boucle for l'une dans l'autre qui parcourt tout le tableau
   donc la complexité est la hauteur multiplié par la largeur
   ce quie nous donne donc o(h*l) */
void AfficheTab(int *data,int width,int height,int x,int y){
	for(int j=0;j<height;++j){
		fprintf(stderr,"\n");
		for(int i=0;i<width;++i){
			if(y==j&&x==i){
				fprintf(stderr,"X");
			}
			else{
				fprintf(stderr,"%d",data[matrice2tab(width,i,j)]);
			}
		}
	}
	fprintf(stderr, "\n");
}

/* Fonction qui transforme une coordonnée en une position */
/* Multiplciation entre deux valeurs plus addition à celle-ci, complexité en o(1) */
int matrice2tab(int width, int x,int y){
	return y*width+x;
}

/* Fonction qui affiche les cases adjacentes */
/* buf[i] a pour taille 8 donc au maximum 8 tours de boucles donc o(8) */
void afficheCaseAdj(char *buf){
	size_t i=0;
	while(buf[i]!='\0'){
		fprintf(stderr,"%c",buf[i]);
		++i;
	}
}

/* Fonction qui retourne un minimum */
/* Deux boucles for indépendantes l'une de l'autre 
   -la premiere fait 3 tours de boucles
   -la seconde 4
   -le if et le switch ne sont que des opérations simples donc o(1)
   -complexité totale est o(3 + 4) donc o(7) */
int min(int east,int west,int south, int north,int prio_x,int prio_y){
	int tab[4];
	int mini[4];
	int count=0;
	int vmin=east;
	int rdm=0;
	int rdm2=rand()%3;
	bool p=false;
	tab[0]=east;
	tab[1]=west;
	tab[2]=south;
	tab[3]=north;
	for(int i=1;i<4;++i){
		if(tab[i]<vmin){
			vmin=tab[i];
		}
	}
	for(int i=0;i<4;++i){
		if(tab[i]==vmin){
			mini[count]=i;
			++count;
			if(prio_y==i||prio_x==i){
				p=true;
				rdm=i;
			}
		}
	}
	if(p==false||rdm2!=0){
		rdm=rand()%count;
		switch (mini[rdm]) {
			case 0: return 4;
			case 1: return 3;
			case 2: return 6;
			default : return 1;
		}
	}
	switch (rdm) {
		case 0: return 4;
		case 1: return 3;
		case 2: return 6;
		default : return 1;
	}
}

/* Fonction qui regarde si le chemin actuel est une impasse */
/* Simples comparaisons donc o(1) */
bool impasse(int res,int east,int west,int north,int south){
	if (res==1){
		return east>5&&west>5&&south>5;
	}
	if (res==3){
		return east>5&&north>5&&south>5;
	}
	if (res==4){
		return north>5&&west>5&&south>5;
	}
	return east>5&&west>5&&north>5;

}

/* Fonction qui cherche si un passage est condamnable ou non */
/* impasse est en o(1) 
   le reste ne sont que de simples opérations donc o(1) */
int recupCasePrio(int *data,int x,int y,int width,int priox, int prioy){
	int east=data[matrice2tab(width,x+1,y)];
	int west=data[matrice2tab(width,x-1,y)];
	int north=data[matrice2tab(width,x,y-1)];
	int south=data[matrice2tab(width,x,y+1)];
	int res=min(east,west,south,north,priox,prioy);
	if (impasse(res,east,west,north,south)){
		/* Met un 7 quand on ajoute un nouveau mur pour condamner un passage inutile */
		data[matrice2tab(width,x,y)]=MUR_AJOUT;
	}
	return res;
}

/* Fonction qui envoie au serveur le déplacement à suivre*/
/* Complexité en o(1), aucune opération ici */
void Deplace(int a){
	switch (a){
		/* Deplacement au nord */
		case 1: puts("NORTH");
			break;
		/* Deplacement à l'ouest */
		case 3: puts("WEST");
			break;
		/* Deplacement à l'est */
		case 4: puts("EAST");
			break;
		/* Deplacement au sud */
		case 6: puts("SOUTH");
			break;
		
	}
}

/* Fonction qui retourne la taille optimal du trajet retour */
/* Celle-ci parcourt tous les chemins déjà explorer pour déterminer
   le chemin le plus court, il y a n cases chemins possibles
   donc complexité en o(n) */
int taille(int *data,int x,int y,int xi,int yi, int width,int height){
	if(data[matrice2tab(width,x,y)]==MUR||data[matrice2tab(width,x,y)]==0||data[matrice2tab(width,x,y)]==MUR_RETOUR){
		/* Détecte si on essaie de passer sur un mur, 
		   qqch d'inexploré ou si on revient sur nos pas */
		return TAILLE_MAX;
	}
	if(data[matrice2tab(width,x,y)]==POSITION_INITIALE){
		/* Si on arrive sur cette case à 9, on est revenu à la position initiale 
		   Donc on peut arréter la recherche de taille */
		return 0;
	}
	/* Met un 8 sur le chemin retour derrière soit */
	data[matrice2tab(width,x,y)]=MUR_RETOUR;
	return 1+minimum(taille(data,x-1,y,xi,yi,width,height) ,taille(data,x+1,y,xi,yi,width,height),taille(data,x,y-1,xi,yi,width,height) ,taille(data,x,y+1,xi,yi,width,height));
}

/* Fonction qui retourne le minimum d'un trajet retour */
/* Simple comparaison entre des valeurs donc o(1) */
int minimum(int a, int b , int c ,int d){
	if (a<=b && a<=c && a<=d){
		return a;
	}
	if (b<=a && b<=c && b<=d){
		return b;
	}
	if (c<=a && c<=b && c<=d){
		return c;
	}
	return d;
}

/* Fonction qui condamne notre trajet retour en remplacent les 8 par des 7 */
/* Double boucle for l'une dans l'autre qui parcourt tout le tableau
   donc la complexité est la hauteur multiplié à la largeru
   donc o(h*l) */
void reset8(int *data,int width,int height){
	for(int i=0;i<width;++i){
		for(int j=0;j<height;++j){
			if(data[matrice2tab(width,i,j)]==MUR_RETOUR){
				/* Met un 7 pour condamner le chemin */
				data[matrice2tab(width,i,j)]=MUR_AJOUT;
			}
		}
	}
}

/* Fonction de retour pour revenir à la position initiale
   Suivant la direction on décrément ou incrémente la position actuelle */
/* Parcourt n tours de boucles jusqu'à revenir à la position initiale 
   avec 4 fonctions de reset8 (complexité de o(h*l))
   les if ne sont que des comparaisons avec affectations donc o(1)
   la complexité est donc o(n*h*l) */
void retour(int *data,int x,int y,int xi,int yi, int width,int height){
	char buf[BUFSIZE];
	/* Boucle jusqu'à revenir à la position initiale */
	while(x!=xi||y!=yi){
		data[matrice2tab(width,x,y)]=MUR;
		int size_west=taille(data,x-1,y,xi,yi,width,height);
		reset8(data,width,height);
		int size_north=taille(data,x,y-1,xi,yi,width,height);
		reset8(data,width,height);
		int size_east=taille(data,x+1,y,xi,yi,width,height);
		reset8(data,width,height);
		int size_south=taille(data,x,y+1,xi,yi,width,height);
		reset8(data,width,height);
		/* Réalise le déplacement retour avec incrémentation de la position */
		if(size_west<=size_north&&size_west<=size_south&&size_west<=size_east){
			puts("WEST");
			--x;	
		}
		else{
			if(size_north<=size_west&&size_north<=size_south&&size_north<=size_east){
				puts("NORTH");
				--y;
			}
			else{
				if(size_east<=size_north&&size_east<=size_south&&size_east<=size_west){
					fgets(buf, BUFSIZE, stdin);
					puts("EAST");
					++x;
				}
				else{
					puts("SOUTH");
					++y;
				}
			}
		}
	}
}

