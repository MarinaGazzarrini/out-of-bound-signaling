#if !defined(ABR_H)
#define ABR_H

typedef struct node{
long long int key; //id del client
int stima;     //stima fino a quel momento
int numServer; //il numero di server che hanno contribuito alla stima
struct node *sx;
struct node *dx;
}node;

//per inserire un nuovo nodo (o controllare se già esiste e nel caso aggiornarlo)
node *insert(node *n, long long int id, int s);

//per visitare l'albero e stampare il contenuto(in ordine non decrescente di chiave)
//d la uso per capire se devo stampare su stderr o stdout
void invisit(node *t,int d);

//eliminare l'albero
void destroy(node *t);

#endif /* ABR_H */

