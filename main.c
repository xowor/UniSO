/* 
 * File:   main.c
 * Author: federica
 *
 * Created on 7 novembre 2014, 10.55
 */

#include <stdio.h>
#include <stdlib.h>

int clientGenerator(){
    return 0;
    // creare e scrivere il file con all'interno le risorse che il cliente deve acquisire e loro qta, budget disponibile
}
/**
 * Il main funge da generatore di clienti e avvia il sistema.
 */
int main(int argc, char** argv) {
    // creazione clienti
    // registrazione clienti al banditore (pid, risorsa interessata)
    
    int auctioneer_pid = fork();
    if ( auctioneer_pid == -1 ){
        printf("Error: auctioneer not created.");
    }
    //Non è stato creato il processo figlio
    if ( auctioneer_pid ) {
    // equivale a "if (pid_child != 0)"
    /*codice del padre*/
        
    } else {
    /*
    codice del figlio*/
        execve("./auctioneer", char *const argv[], char *const envp[]);
    }
    
    
    return (EXIT_SUCCESS);
}

