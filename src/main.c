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
    printf("[main] Started main.\n");
    // creazione clienti
    // registrazione clienti al banditore (pid, risorsa interessata)

    int auctioneer_pid = fork();
    if ( auctioneer_pid == -1 ){
        printf("[main] Error: auctioneer not created.\n");
    }
    //Non è stato creato il processo figlio
    if ( auctioneer_pid == 0 ) {
        /*  Child code */
        char *envp[] = { NULL };
        char *argv[] = { NULL };
        int auct_execve_err = execve("./auctioneer", argv, envp);
        if (auct_execve_err == -1)
            /* Cannot find the auctioneer binary in the working directory */
            fprintf(stderr, "[auctioneer] Error: cannot start auctioneer process (Try running main from ./bin).\n");

    } else {
        /* Parent code */
        int i;
        int client_pid;
        for (i = 0; i < 5; i++){
            client_pid = fork();
            if ( client_pid == -1 ){
                printf("[main] Error: auctioneer not created.");
            }
            if ( client_pid == 0 ) {
                /*  Child code */
                char *envp[] = { NULL };
                char *argv[] = { NULL };
                int auct_execve_err = execve("./client", argv, envp);
                if (auct_execve_err == -1)
                    /* Cannot find the auctioneer binary in the working directory */
                    fprintf(stderr, "[client] Error: cannot start client process (Try running main from ./bin).\n");

            } else {
                /* Parent code */
            }
        }

        sleep(1);
        return (EXIT_SUCCESS);
    }


    return (-1);
}
