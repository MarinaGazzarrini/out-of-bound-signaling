 #!/bin/bash
 
 #ho due array associativi, uno conterrà per ogni client il valore del secret reale e l'altro il valore del secret stimato dal supervisor
 declare -A secretReale
 declare -A secretStimato



 #leggo riga per riga il file che contiene l'output del supervisor e aggiorno l'array secretStimato
 while read line; do
	r=($line)
	case $line in
		"SUPERVISOR "*"BASED "*) #è una linea dove è scritta la stima finale poichè contine "BASED" e "SUPERVISOR"
			secretStimato[${r[4]}]=${r[2]} ;; # r[4]->id del client     r[2]->stima fatta dal supervisor
	esac
 done <$1



 #leggo riga per riga il file che contiene l'output del client e aggiorno l'array secretReale
 while read line; do
	r=($line)
	case $line in
		*"SECRET "*) #linea del file di output del client dove specifica anche il secret
			secretReale[${r[1]}]=${r[3]} ;; # r[1]->id del client      r[3]->secret
	esac
done <$2


 ok=0 #numero di secret correttamente stimati
 ko=0 #numero di secret stimati in modo errato
 tot=0 #numero di stime totali
 sumErr=0 #sommatoria(|secret corretto - secret calcolato|)
 erroreMedioTot=0 #errore medio calcolato sui valori totali
 erroreMedioErr=0 #errore medio calcolato sui valori errati
 tmp=0

 #scorro secretReale, cerco l'id corrispondente in secretStimato e controllo se la stima è corretta o meno
 for i in "${!secretReale[@]}"; do 
	if(( secretStimato[$i] == secretReale[$i] )); then
		ok=$[ok+1]
	elif (( (secretStimato[$i] < secretReale[$i]) && ((secretStimato[$i]+25) >= secretReale[$i]) )); then
		ok=$[ok+1]
	elif (( (secretStimato[$i] > secretReale[$i]) && ((secretStimato[$i]-25) <= secretReale[$i]) )); then
		ok=$[ok+1]
	elif (( $[secretStimato[$i]] < $[secretReale[$i]] )); then
		ko=$[ko+1]
		tmp=$[secretReale[$i]]-$[secretStimato[$i]]
		sumErr=$[sumErr+tmp]
	else
		ko=$[ko+1]
		tmp=$[secretStimato[$i]]-$[secretReale[$i]]
		sumErr=$[sumErr+tmp]
	fi
 done

 tot=$[ko+ok]

 if(($ko==0)); then
	erroreMedioErr=0
 else
	erroreMedioErr=$[sumErr/ko]
 fi


 if(($tot==0)); then
	erroreMedioTot=0
 else
	erroreMedioTot=$[sumErr/tot]
 fi


 echo "numero di stime totali: " $tot
 echo "numero di stime corrette: " $ok
 echo "numero di stime errate: " $ko
 echo "errore medio in proporzione alle stime totali: " $erroreMedioTot
 echo "errore medio in proporzione alle stime errate: " $erroreMedioErr 
