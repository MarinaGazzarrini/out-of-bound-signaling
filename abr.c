#if !defined(ABR_C_)
#define ABR_C_


/*albero binario di ricerca dove andrò ad inserire gli l'id del client, la stima del supervisor associata e il numero di server che hanno contribuito*/

#include "abr.h"
#include "lib.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//controlla se esiste già un nodo con key==id. Se non esiste lo inserisce, altrimenti aggiorna quello che già esiste. Restituisce l'albero
node *insert(node *t, long long int id, int s){

	if(t==NULL){//nuovo nodo
		node *new=malloc(sizeof(node));
		if(new==NULL){//controllo se malloc è andata a buon fine
			 perror("Error->malloc\n"); 
			 fflush(stderr);       
			 exit(EXIT_FAILURE); 
		}
		new->key = id;
		new->stima = s;
		new->numServer=1;
		new->sx = NULL;
		new->dx=NULL;
		return new;
	}else{
		if(t->key == id){//aggiorno la stima e il numero di server
			t->numServer= (t->numServer) +1;
			t->stima= min((t->stima),s); //in alternativa si poteva fare MCD

		}else if(t->key < id) t->dx= insert(t->dx, id,s);
			else t->sx= insert(t->sx,id,s);
		return t;
	}
}

//stampo il contenuto dell'albero in ordine non decrescente delle chiavi
void invisit(node *t,int d){
	if(t != NULL){
		invisit(t->sx,d);
		if(d==0){//stampo su stderr
			fprintf(stderr, "SUPERVISOR ESTIMATE %d FOR %llx BASED ON %d\n",t->stima,t->key,t->numServer);
			fflush(stderr);
		}else{ //stampo su stdout
			fprintf(stdout, "SUPERVISOR ESTIMATE %d FOR %llx BASED ON %d\n",t->stima,t->key,t->numServer);
			fflush(stdout);
		}
		invisit(t->dx,d);
	}

}



void destroy(node *t){
	if(t !=NULL){
		destroy(t->sx);
		destroy(t->dx);
		free(t);
	}
	t=NULL;
}

#endif /* ABR_C_ */


