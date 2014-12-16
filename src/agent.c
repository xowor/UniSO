#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

int msqid;

/* Check */
//int check_budget(){

//}

int main(int argc, char** argv){
	
	pid_t pid;
	pid = getpid();
	
	printf("[agent] Started agent.\tPid: %d\tPPid: %d\n", pid, getppid());

	if (argc >= 2 && strcmp(argv[1], "-m") == 0 ){
        msqid = atoi(argv[2]);
        //client_num = atoi(argv[4]);
        //fprintf(stdout, "[agent][%d][%d] Using message queue %d\n", client_num, pid, msqid);
    } else {
        fprintf(stderr, "[agent][%d] Error: msqid (-m) argument not valid.\n", pid);
        return -1;
    }
    
    // da qui richiamo i metodi che fanno le offerte
    
    // chiama un metodo dell'agente che resta in attesa del segnale di avvio dell'asta
	// chiama un metodo nell'agente che inizia a fare le offerte
	// incrementa il semaforo
	// fa l'offerta
	// decrementa il semaforo
	// aspetta di nuovo il suo turno
}

