#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/wait.h>
#include<unistd.h>
#include<errno.h>
#include <time.h>
#include <arpa/inet.h>

#define MAX 16


int main(int argc, char* argv[]){

	int p; //numero di server a cui manda i messaggi
	int k; //numero dei server in totale
	int w; //numero messaggi da inviare in totale

	//controllo dei parametri in ingresso
	if(argc !=4){ 
		//ho passato un numero errato di parametri
		fprintf( stderr, "client: wrong parameter number\n");
		//fflush(stderr);
		exit(EXIT_FAILURE);

	}

	p=atoi(argv[1]);
        k=atoi(argv[2]);
	w=atoi(argv[3]);

		if(p<1 || p>k){//valore di p errato
			fprintf( stderr, "wrong value of p\n");
			//fflush(stderr);
			exit(EXIT_FAILURE);

		}

			if(w<(3*p) || w==(3*p)){//valore di w errato
				fprintf( stderr, "wrong value of w\n");
				//fflush(stderr);
				exit(EXIT_FAILURE);

			}
	

	

/*Genero id e secret. Come argomento di srand combino il valore di time() e di getpid() che mi restituisce il pid del processo corrente così da evitare il più possibile la generazione di valori uguali per gli id dei client*/

	long long int id;
	int secret;

	//voglio essere certa che l'id sia univoco

	int tmp=rand()%50;
	if(tmp < 10) srand48((getpid())^(time(NULL)));
	else if((tmp >= 10) && (tmp < 20))srand48((getpid())*(time(NULL)));
	else if((tmp >= 20) && (tmp < 30))srand48((getpid())^(time(NULL))*56783);
	else if((tmp >= 30) && (tmp < 40))srand48((getpid())*(time(NULL))*56783);
	else srand48((getpid())^(time(NULL))*getpid());

	//devo combinare più volte di seguito lrand48 per poi ottenere un num a 64 bit	
	long long int uno=lrand48();
	long long int due=lrand48();
	long long int tre=lrand48();

	int tmp2=rand()%30;
	//genero id
	if(tmp2<10)id=(uno << 42) + (due << 21) + (tre);
	else if(tmp2>20)id=(due << 42) + (tre << 21) + (uno);
	else id=(tre << 42) + (uno << 21) + (due);
	

	srand((getpid())^(time(NULL)));
	secret=rand()%3000+1;//genero secret

	printf("CLIENT %llx SECRET %d\n",id,secret);
	fflush(stdout);

	//messaggio che voglio inviare al server
	char mex[16]={};

	//breve procedimento trovato su stackoverflow per passare da host byte order a network byte order
	//chiamo la funzione htonl sulle due parti dell'id(32 bit ciascuna)
	long int prima = htonl((long int)(id >> 32));
	long int poi = htonl((long int)(id & 0xFFFFFFFFLL));

	//messaggio deve contenere l'id in network byte order 
	sprintf(mex,"%llx",((((long long int)poi) << 32) | prima));


	//scelgo p server distinti con cui comunicare
	//serverScelti[i]=1 se voglio comunicare con il server i, altrimenti sarà zero

	int i,j;
	int serverScelti[k];//array che utilizzo per capire quali server ho scelto
	
		for(i=0; i<k; i++){//inizializzo a zero
			serverScelti[i]=0;
		}


			for(i=0;i<p;i++){
				j=rand()%k;
				while(serverScelti[j] ==1){//controllo se il server è già stato selezionato
					j= rand()%k;
				}
				serverScelti[j]=1;

			}

	//preparo la connessione
	char SOCKNAME[15] = "OOB-server-";
	struct sockaddr_un sa;
	bzero(&sa,sizeof(sa));
	sa.sun_family=AF_UNIX;
	int connessioni[k];

	//mi collego ai p server scelti prima
	for(i=0; i<k; i++){
		if(serverScelti[i]==1){//se quel server era stato scelto
			sprintf((SOCKNAME+11), "%d", i);
			strncpy(sa.sun_path,SOCKNAME,MAX);
			connessioni[i]=socket(AF_UNIX,SOCK_STREAM,0);
			if((connect(connessioni[i],(struct sockaddr*)&sa,sizeof(sa))) <0){
				perror("Error->connect\n");
				exit(EXIT_FAILURE);
			}

		}
	}

//scelgo casualmente w server dei p selezionati prima(questa volta possono esserci ripetizioni) a cui invierò i messaggi e li salvo in mexTot. Lo faccio adesso per essere sicura che l'attesa tra un messaggio e il successivo sia di secret millisecondi e non venga ritardato dalla scelta del server a cui inviarlo
	int mexTot[w];//server a cui inviare i w messaggi in ordine di invio
	i=0;
		while(i<w){
			j=rand()%k;
			if(serverScelti[j]==1){//controllo che sia uno dei p selezionati in precedenza
				mexTot[i]=j;
				i++;
			}
		}

//mi serve per poter fare la nanosleep
	struct timespec attesa={};
	attesa.tv_sec=secret/1000; //secondi
	attesa.tv_nsec = (secret%1000) * 1000000;//nanosecondi

//adesso posso inviare i w messaggi ai server selezionati
		for(i=0;i<w; i++){
			write(connessioni[mexTot[i]],mex,MAX);//mando il messaggio
			if((nanosleep(&attesa,NULL))==-1){ //attesa 
				perror("Error->nanosleep\n");
			}
		}



//chiudo le connessioni
	for(i=0; i< p; i++){
		close(connessioni[i]);
	}
	


		printf("CLIENT %llx DONE\n", id);
		fflush(stdout);

return 0;

}
