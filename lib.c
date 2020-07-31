#if !defined(LIB_C)
#define LIB_C


#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "lib.h"

//funzione che mi restituisce il minimo
int min(int x, int y){
	if(x<y)return x;
	else return y;
}

//funzione che calcola il massimo comun divisore tra due numeri. se è 1, restituisce il più piccolo fra i due numeri
int MCD(int x, int y){
	int temp;
	int a=x;
	int b=y;
	while(y!=0){
		temp=x%y;
		x=y;
		y=temp;
	}

	if(x==1 || x==2) return min(a,b);
	else return x;

}


#endif /* LIB_C */

