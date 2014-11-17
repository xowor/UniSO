#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include "resource.h"

#define NCLIENT 5

extern int errno;       /* Externally declared (by kernel) */

/**
 * Creates the auctioneer process.
 */
int create_auctioneer(){
    // TODO: registrazione clienti al banditore (pid, risorsa interessata)

    int auctioneer_pid = fork();
    //int status = 0;
    
    if ( auctioneer_pid == -1 ){
        printf("[main] Error: auctioneer not created.\n");
        fprintf(stderr, "\t%s\n", strerror(errno));
        //return -1;
        exit(EXIT_FAILURE);                                                         // ########## EXIT FAILURE/SUCCESS MORE PORTABLE THAN 0 e -1
    }
    if ( auctioneer_pid == 0 ) {
        /*  Child code */
        char *envp[] = { NULL };
        char *argv[] = { NULL };
        /* Run the auctioneer process. */
        int auct_execve_err = execve("./auctioneer", argv, envp);
        if (auct_execve_err == -1) {
            /* Cannot find the auctioneer binary in the working directory */
            fprintf(stderr, "[auctioneer] Error: cannot start auctioneer process (Try running main from ./bin).\n");
            /* strerror nterprets the value of errnum, generating a string with a message that describes the error */
            fprintf(stderr, "\t%s\n", strerror(errno));
        }
    } else {
        /* Parent code */
        //return 0;   /* Success */                                                 /* In questo punto, la funzione exit porta a visualizzare nel terminale il path del progetto -  lasciare così che il risultato è come il tuo!!*/
        return EXIT_SUCCESS;                                                        // ########## EXIT FAILURE/SUCCESS MORE PORTABLE THAN 0 e -1
    }

    //return -1;
    perror("fork");
    exit(EXIT_FAILURE);                                                             // ########## EXIT FAILURE/SUCCESS MORE PORTABLE THAN 0 e -1
}

/**
 * Creates a client process.
 */
int create_client(){
    // TODO: creare e scrivere il file con all'interno le risorse che il cliente deve acquisire e loro qta, budget disponibile
    
    int client_pid = fork();
    
    if ( client_pid == -1 ){
        printf("[main] Error: client not created.");
        fprintf(stderr, "\t%s\n", strerror(errno));
        //return -1;
        exit(EXIT_FAILURE);                                                         // ########## EXIT FAILURE/SUCCESS MORE PORTABLE THAN 0 e -1
    }
    if ( client_pid == 0 ) {
        /*  Child code */
        char *envp[] = { NULL };
        char *argv[] = { NULL };
        /* Run the client process. */
        int clnt_execve_err = execve("./client", argv, envp);
        if (clnt_execve_err == -1) {
            /* Cannot find the auctioneer binary in the working directory */
            fprintf(stderr, "[client] Error: cannot start client process (Try running main from ./bin).\n");
            /* strerror nterprets the value of errnum, generating a string with a message that describes the error */
            fprintf(stderr, "\t%s\n", strerror(errno));
        }
    } else {
        /* Parent code */
        //return 0;   /* Success */
                                                                                    /* In questo punto, la funzione exit porta a visualizzare nel terminale il path del progetto -  lasciare così che il risultato è come il tuo!!*/
        return EXIT_SUCCESS;                                                        // ########## EXIT FAILURE/SUCCESS MORE PORTABLE THAN 0 e -1
    }

    //return -1;
    perror("fork");
    exit(EXIT_FAILURE);                                                             // ########## EXIT FAILURE/SUCCESS MORE PORTABLE THAN 0 e -1
}


/**
 * Generates the auctioneer and some clients.
 */
int main(int argc, char** argv) {
    
    printf("[main] Started main.\t\tPid: %d\n", getpid());
    fflush(stdout);

    int i = 0;
    int auctnr_exit = create_auctioneer();

    /* Checks the auctioneer creation was successful. */
    if (auctnr_exit == 0){
        
        for (i = 0; i < NCLIENT; i++){
            int clnt_exit = create_client();
            /* If the client creation fails, will not try to create more clients */     /* Perchè non prova a crearne altri?? */
            if (clnt_exit != 0)
                exit(EXIT_FAILURE);                                                     // ########## EXIT FAILURE/SUCCESS MORE PORTABLE THAN 0 e -1
                //return -1;
        }

        sleep(1);
        exit(EXIT_SUCCESS);
    }
    exit(EXIT_FAILURE);
    //return -1;                                                                        // ########## EXIT FAILURE/SUCCESS MORE PORTABLE THAN 0 e -1
}
