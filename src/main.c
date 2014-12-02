#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include "resource.h"

#define MAX_CLIENTS 5

extern int errno;       /* Externally declared (by kernel) */

/**
 * Creates the auctioneer process.
 */
int create_auctioneer(int msqid){
    // TODO: registrazione clienti al banditore (pid, risorsa interessata)
    char str_msqid [32];
    sprintf(str_msqid, "%d", msqid);

    int auctioneer_pid = fork();
    //int status = 0;

    if ( auctioneer_pid == -1 ){
        printf("[main] Error: auctioneer not created.\n");
        fprintf(stderr, "\t%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if ( auctioneer_pid == 0 ) {
        /*  Child code */
        char *envp[] = { NULL };
        char *argv[] = { "./auctioneer", "-m", str_msqid, NULL };
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
        return EXIT_SUCCESS;        /* Success */
    }

    perror("fork");
    exit(EXIT_FAILURE);
}

/**
 * Creates a client process.
 */
int create_client(int msqid, int client_num){

    char str_msqid [32];
    char str_client_num [6];
    sprintf(str_msqid, "%d", msqid);
    sprintf(str_client_num, "%d", client_num);

    int client_pid = fork();

    if ( client_pid == -1 ){
        printf("[main] Error: client not created.");
        fprintf(stderr, "\t%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if ( client_pid == 0 ) {
        /*  Child code */
        char *envp[] = { NULL };
        char *argv[] = { "./client", "-m", str_msqid, "-c", str_client_num, NULL };
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
        return EXIT_SUCCESS;
    }

    perror("fork");
    exit(EXIT_FAILURE);
}


/**
 * Generates the auctioneer and some clients.
 */
int main(int argc, char** argv) {

    printf("[main] Started main.\t\tPid: %d\n", getpid());
    fflush(stdout);

    // /* Creates the unique message queue key with system call ftok() */
    // key_t key = ;
    // if (key == -1) fprintf(stderr, "\t%s\n", strerror(errno));
    //
    // printf("%d", key);

    /* Get the message queue id, with the 0666 permissions. */
    int msqid = msgget(IPC_PRIVATE, 0666 | IPC_CREAT);
    int i = 0;
    int auctnr_exit = create_auctioneer(msqid);

    /* Checks the auctioneer creation was successful. */
    if (auctnr_exit == 0){

        for (i = 0; i < MAX_CLIENTS; i++){
            int clnt_exit = create_client(msqid, i);
            /* If the client creation fails, will not try to create more clients */     /* Perchè non prova a crearne altri?? */
            if (clnt_exit != 0)
                exit(EXIT_FAILURE);
        }

        sleep(1);
        exit(EXIT_SUCCESS);
    }
    exit(EXIT_FAILURE);
}
