#if !defined(LIB_H)
#define LIB_H


#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//per il controllo sistematico degli errori
#define SYSCALL(c,e, stampa) if(c==e){perror(stampa); exit(EXIT_FAILURE); } 

/*****tipo del messaggio*****/
typedef struct message{
    long long int id; //id
    int stima;//stima
} message;

//funzione che mi restituisce il minimo fra due interi
int min(int x, int y);

//funzione che calcola il massimo comun divisore tra due numeri. se è 1, restituisce il più piccolo fra i due numeri
int MCD(int x, int y);


#endif /* LIB_H */

