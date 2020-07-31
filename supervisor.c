#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include <unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include <fcntl.h>
#include<signal.h>

//nuove librerie
#include "lib.h"
#include "abr.h"

node *stimeFinali=NULL;//albero binario di ricerca in cui inserire il secret stimato,il rispettivo id del client e il numero di server per stimarlo
volatile sig_atomic_t go=1;// condizione terminazione ciclo principale. signal safe
volatile sig_atomic_t count=0;//contatore globale signal safe 


//gestore del segnale SIGINT
void SIG_Handler(){

	if(count==0){// se non ho ricevuto un altro SIGINT meno di un secondo prima
		count=1;//così mi rendo conto che il successivo è il secondo SIGINT
		alarm(1);//invia al processo corrente SIGALRM dopo 1 secondo(serve perchè voglio riportare count a zero)
		invisit(stimeFinali,0);
		
	}else{
	    //ho ricevuto un altro SIGINT meno di un secondo fa 
	    go=0;// devo uscire dal ciclo e chiudere tutto


	}
}


//gestore del segnale SIGALRM
void SIG_Alarm_Handler(){
	count=0;//riporto count a zero 
}


int main (int argc, char* argv[]){

	//controllo che il numero di argomenti passati sia corretto
	if(argc != 2){
		fprintf( stderr, "supervisor: wrong parameter number");
		exit(EXIT_FAILURE);
	}

    	
	struct sigaction SI;
	struct sigaction SA;

	//resetto
	bzero(&SI,sizeof(SI));
	bzero(&SA,sizeof(SA));

	//registro gestore
	SI.sa_handler = SIG_Handler;
	SA.sa_handler= SIG_Alarm_Handler;


	//forzo la riesecuzione di syscall se interrotte da segnali
	SI.sa_flags=SA_RESTART;
	SA.sa_flags=SA_RESTART;

	//installazione gestore
	SYSCALL(sigaction(SIGINT,&SI,NULL),-1,"Error->sigaction sigint\n");
	SYSCALL(sigaction(SIGALRM,&SA,NULL),-1,"Error->sigaction sigalrm\n");


	int k=atoi(argv[1]); //numero di server
	int servers[k];//qui salvo il process id di tutti i server
	int servPipe[k][2];//array di pipe. ogni server può comunicare con il supervisor usando la rispettiva pipe
	int pd,i;
	

	printf("SUPERVISOR STARTING %d\n",k);
	fflush(stdout);

	//mi servono perchè gli argomenti alla execl posso passarli solo come stringa
	char *iStringa=(char*)malloc(sizeof(int));
		if(iStringa==NULL){//controllo che sia andata a buon fine
			 perror("Error->malloc");        
			 exit(EXIT_FAILURE); 
		}

	char *pipeStringa=(char*)malloc(sizeof(int));
		if(pipeStringa==NULL){
			 perror("Error->malloc");        
			 exit(EXIT_FAILURE); 
		}




	for(i=0; i<k; i++){ 
	
		//creo una pipe anonima con cui comunicare con il processo figlio/server
		//con servPipe[i][0] posso leggere dalla pipe, con servPipe[i][1] posso scrivere nella pipe(poi verrà chiusa)
		SYSCALL(pipe(servPipe[i]), -1, "Error->pipe\n");
	    	sprintf(iStringa, "%d", i);
	    	sprintf(pipeStringa, "%d",servPipe[i][1]);

		//utilizzo fork() per creare i k server(figli)
		if((pd=fork())==-1){//errore
			perror("Error->fork\n"); 
			exit(EXIT_FAILURE);

		//utilizzo execl, passando come secondo argomento fd pipe e come terzo l'indice(id server) 
		}else if(pd==0){//figlio
			
			close(servPipe[i][0]);//figlio->chiudo pipe in lettura
			SYSCALL(execl("./myserver", "myserver", pipeStringa,iStringa, NULL), -1, "Error->execl\n");   //chmod 777 server
			exit(0);

		}else{//padre 
			servers[i]=pd;//in servers[i] ho il process ID del figlio
	   		close(servPipe[i][1]);//padre->chiudo pipe in scrittura
		      }
	}


	

	//libero la memoria
	free(iStringa);
	free(pipeStringa);

	

	//utilizzo un selettore per gestire le read dalle pipe
	fd_set set;//insieme file descriptor attivi
	fd_set rdset; //insieme fd attesi in lettura
	FD_ZERO(&set);

		//registro i file descriptor nella maschera
		for(i=0; i< k; i++){
			FD_SET(servPipe[i][0],&set);
			fcntl(servPipe[i][0], F_SETFL,O_NONBLOCK); //non voglio bloccarmi alla read sulle pipe->setto il fd
		}

	rdset=set;
	int r,s;
	

	//fino a che non ricevo un doppio SIGINT go resta settata ad 1 e non esco dal ciclo
	while(go==1){
		rdset=set; //bisogna inizializzare ogni volta rdset perchè le select lo modifica
		struct timeval t;
		t.tv_sec=0;
		t.tv_usec=150;
		if((s=(select((servPipe[k-1][0])+1, &rdset, NULL, NULL, &t)))==-1){ //controllare errno->interruzione da segnali
		     	if(errno != EINTR){
				perror("Error->select\n");
				exit(EXIT_FAILURE);
			}
		}else{   
			//scorro i file descriptor
			for(i=0; i<k; i++){
				if(FD_ISSET(servPipe[i][0],&rdset)){//se fd vale 1 in rdset(lettura). quindi pronto per essere letto
					message mex;//NB il messaggio viene già inviato direttamente dall'altra parte come message
					
					if(((r=read(servPipe[i][0],&mex,sizeof(message)))==-1)){
						if(errno==EAGAIN)continue; //necessario perchè non blocking->read potrebbe non essere pronta
						else{
						  perror("Error->read\n");
						  exit(EXIT_FAILURE);
						}
					    }else{ 
						if(mex.stima > 0){//se è -1 vuol dire che il server ha ricevuto un solo mex dal client
							//inserisco id e stima nell'abr, la funzione insert si occupa di fare tutti i controlli
							stimeFinali=insert(stimeFinali,mex.id,mex.stima);
							fprintf(stdout,"SUPERVISOR ESTIMATE %d FOR %llx FROM %d\n", mex.stima,mex.id,i);
							fflush(stdout);
						}
					
				               }

			         }


		        }


	     }
	}

	//se sono uscita da ciclo significa che mi sono arrivati due segnali SIGINT a meno di un secondo di distanza l'uno dall'altro

	invisit(stimeFinali,1);//stampo le informazioni sulle stime raccolte

	printf("SUPERVISOR EXITING\n");
	//fflush(stdout);


	for(int j=0;j<k;j++){
		kill(servers[j],SIGTERM);//per "chiudere" servers[j]
		close(servPipe[j][0]);//chiudo le pipe
	}


	destroy(stimeFinali);//libero la struttura

	return 0;

}


