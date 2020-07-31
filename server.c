#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/wait.h>
#include<unistd.h>
#include<errno.h>
#include <pthread.h>
#include<time.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include "lib.h"
#include<signal.h>

#define MAX 15

int p;//fd pipe
int idS;//id del server
volatile sig_atomic_t go=1;//condizione di terminazione del ciclo, global signal safe

//quando arriva SIGTERM pone a 0 go e consente al ciclo di terminare
void SIG_Handler(){
go=0;
}


void* work( void* fd_c ) {

		printf("SERVER %d CONNECT FROM CLIENT\n",idS);
		
		long fd=(long)fd_c;
		long long int idC;//id del client
		int secret;//secret
		char buf[16];//buffer che uso per la read
		int uno,due,tre,c,i,j;//variabili di appoggio
		int numMex=0;//numero messaggi arrivati
		struct timeval now;


		while((c=(read((long)fd,buf,sizeof(buf))))>0){
			SYSCALL(gettimeofday(&now,NULL),-1,"Error->time\n");//per prima cosa prendo il tempo
			
			if(numMex==0){// è il primo messaggio che ricevo
				uno=(now.tv_sec*1000)+(now.tv_usec/1000);//lo voglio in millisecondi
				long long int b=strtoull(buf,NULL,16); //printf("----%llx\n",b);

				//breve procedimento trovato su stackoverflow per passare da network byte order a host byte order
				//chiamo la funzione ntohl sulle due parti dell'id(32 bit ciascuna)
				long int prima = ntohl((long int)(b >> 32));
				long int poi = ntohl((long int)(b & 0xFFFFFFFFLL));
				idC=( (( (long long int)poi ) << 32) | prima);

				numMex++;
				printf("SERVER %d INCOMING FROM %llx @ %ld:%ld\n",idS,idC,now.tv_sec,now.tv_usec);
				fflush(stdout);
			}else if(numMex==1){// è il secondo messaggio che ricevo
				due=(now.tv_sec*1000)+(now.tv_usec/1000);
				secret=due-uno;//mi sono arrivati solo due messaggi, per ora la stima è la differenza dei tempi di arrivo
				numMex++;
				printf("SERVER %d INCOMING FROM %llx @ %ld:%ld\n",idS,idC,now.tv_sec,now.tv_usec);
				fflush(stdout);
			}else{//è almeno il terzo messaggio che ricevo
				uno=(now.tv_sec*1000)+(now.tv_usec/1000);
				//differenza tempo tra l'ultimo messaggio ricevuto(due) e questo(uno)
				tre=uno-due;
				due=uno;//in due voglio sempre l'ultimo mex ricevuto
				secret=min(tre,secret);
				numMex++;
				printf("SERVER %d INCOMING FROM %llx @ %ld:%ld\n",idS,idC,now.tv_sec,now.tv_usec);
				fflush(stdout);
			}
			
		}

		if(numMex==0 || numMex==1) secret=-1;//un messaggio non è sufficiente per stimare il secret
		if(secret>3000)secret=-1;//secret non può essere maggiore di 3000

			//preparo il messaggio da mandare al supervisor
			message mex;
			mex.id=idC;
			mex.stima=secret;

			//write è atomica se la dimensione del mex è minore della dimensione della pipe
			write(p, &mex, sizeof(mex));//manda al supervisor

			printf("SERVER %d CLOSING %llx ESTIMATE %d\n",idS,idC,secret);
			fflush(stdout);
	
			pthread_exit(0);
		}


int main (int argc, char* argv[]) {

	//controllo i parametri passati
	if(argc != 3){
		//numero errato di parametri
		fprintf( stderr, "server: wrong parameter number\n");
		exit(EXIT_FAILURE);
	}

	p = atoi(argv[1]); //fd della pipe
	idS = atoi(argv[2]); //id del server

	//gest segnali
	struct sigaction ST;
	struct sigaction SI;

	//resetto
	bzero(&SI,sizeof(SI));
	bzero(&ST,sizeof(ST));

	//registro gestore
	SI.sa_handler = SIG_IGN;//ignoro SIGINT
	ST.sa_handler= SIG_Handler;

	//installazione gestore
	SYSCALL(sigaction(SIGINT,&SI,NULL),-1,"Error->sigaction sigint\n");
	SYSCALL(sigaction(SIGTERM,&ST,NULL),-1,"Error->sigaction sigterm\n");


	printf("SERVER %d ACTIVE\n", idS);
	fflush(stdout);

	//file descriptor
	int fd_skt;
	long fd_c; 

	//preparo la connessione
	char SOCKNAME[15] = "OOB-server-";
	sprintf((SOCKNAME+11), "%d", idS);
	unlink(SOCKNAME); 
	struct sockaddr_un sa; 
	strncpy(sa.sun_path, SOCKNAME,MAX);
	sa.sun_family=AF_UNIX; //socket di unix(connessioni in locale su stesso pc)

	SYSCALL((fd_skt=socket(AF_UNIX,SOCK_STREAM,0)),-1,"Error->socket\n"); //creazione file descriptor del socket
	SYSCALL((bind(fd_skt,(struct sockaddr *)&sa, sizeof(sa))),-1,"Error->bind\n"); //assegna un indirizzo ad un socket
	SYSCALL(listen(fd_skt,SOMAXCONN),-1,"Error->listen\n"); //segnala che sul socket sono disposto ad accettare connessioni


	while(go==1){//condizione di terminazione gestita sia da segnale che dall'interno del ciclo
		
		if((fd_c=accept(fd_skt,NULL,0))==-1){
			if(errno==EINTR)go=0;//interruzione causata da segnale->esco dal ciclo
			else{
			    perror("Error->accept\n");
			    exit(EXIT_FAILURE);
			}
		}
		//creo un nuovo thread per ogni connessione
		pthread_t t;
		if((pthread_create( &t, NULL,&work, (void*)fd_c ))!=0){perror("Error->thread\n");}//se fallisce continuo
	 	else pthread_detach(t); //libera le risorse finita l'esecuzione

	}

	
	close(fd_skt); 
	close(fd_c);
	close(p);  
	exit(EXIT_SUCCESS);  
	return 0;
}


