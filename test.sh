 #!/bin/bash

 
 #per prima cosa eseguo il supervisor che lancerà 8 server. scrivo lo stdout di sup in sout
 echo "esecuzione supervisor"
 ./sup 8 >&sout &

 #attendo due secondi prima di lanciare i client
 sleep 2

 #se cout esiste già, lo elimino altrimenti andrei ad aggiungere dati in coda altri dati derivanti da esecuzioni passate
 #(è un controllo in più, poichè elimino cout alla fine)
 #non faccio la stessa cosa con sout perchè in quel caso sovrascrivo
 if [ -f cout ]; then
	rm cout
 fi

 #lancio 20 client a coppie di due e a distanza di un secondo tra una coppia e l'altra
 echo "start client"
 for((i = 0; i < 10; i++)); do
	./myclient 5 8 20 1>>cout & #aggiungo in coda al file(non sovrascrivo)
	./myclient 5 8 20 1>>cout &
	echo "coppia di client numero $i in esecuzione" 
	sleep 1
 done

 #dopo aver lanciato l'ultima coppia di client invio un sigint al supervisor ogni 10 secondi. E dopo 60 secondi invio un doppio SIGINT
 echo "start segnali"
 for((i = 0; i < 70; i+=10)); do
	if (( $i == 60 )); then
	   kill -2 `pidof sup`
	   kill -2 `pidof sup`
	   echo "inviati ultimi due SIGINT"
	else
	    kill -2 `pidof sup` 
	    echo "inviato SIGINT"
	    sleep 10
	fi
 done


 #lancio lo script misura.sh sui risultati raccolti
 echo "statistiche raccolte"
 ./misura.sh sout cout 

 #elimino i file. se si vuole controllare il contenuto dei file dopo l'esecuzione basta commentare le due righe successive.
 #è molto importante che cout venga eliminato perchè altrimenti nell'esecuzione successiva aggiunge dati a quelli già esistenti,
 #non sovrascrive!
 rm sout
 rm cout
