#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include "resource.h"
#include "so_log.h"
#include "config.h"

extern int errno;       /* Externally declared (by kernel) */
int auctioneer_pid = 0;       /* Auctioneer PID */

/**
 * Creates the auctioneer process.
 */
int create_auctioneer(int master_msqid){
    /* puts message queue's id into a char array */
    char str_master_msqid [32];
    sprintf(str_master_msqid, "%d", master_msqid);

    int auctioneer_pid = fork();

    if ( auctioneer_pid == -1 ){
        printf("[main] Error: auctioneer not created.\n");
        fprintf(stderr, "\t%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    } else if ( auctioneer_pid == 0 ) {
        /*  Child code */
        char *envp[] = { NULL };
        //executes auctioneer binary
        char *argv[] = { "./auctioneer", "-m", str_master_msqid, NULL };
        /* Run the auctioneer process. */
        int auct_execve_err = execve("./auctioneer", argv, envp);
        if (auct_execve_err == -1) {
            /* Cannot find the auctioneer binary in the working directory */
            //fprintf prints on stdout default stream
            fprintf(stderr, "[auctioneer] Error: cannot start auctioneer process (Try running main from ./bin).\n");
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
 * Creates a client process.
 */
int create_client(int master_msqid, int client_num){
    char str_master_msqid [32];
    char str_client_num [6];
    sprintf(str_master_msqid, "%d", master_msqid);
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
        char *argv[] = { "./client", "-m", str_master_msqid, "-c", str_client_num, NULL };
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
}


/**
 * Generates the auctioneer and some clients.
 */
int main(int argc, char** argv) {

    printf("[main] Started main.\t\tPid: %d\n", getpid());
    fflush(stdout);

    /* Chreate the master message queue id, with the 0600 permissions. (everything to user) */
    int master_msqid = msgget(IPC_PRIVATE, 0600 | IPC_CREAT);
    int auctnr_exit = create_auctioneer(master_msqid);

    /* Checks the auctioneer creation was successful. */
    if (auctnr_exit == 0){
        int i = 0;
        for (i = 0; i < MAX_CLIENTS; i++){
            int client_exit = create_client(master_msqid, i);
            /* If the client creation fails, will not try to create more clients */
            if (client_exit != 0)
                exit(EXIT_FAILURE);
        }

        int processes = MAX_CLIENTS + 1;
        while(processes > 0){
            //wait for all childs to end
            int p_status = 0;
            waitpid(-1, &p_status, WUNTRACED);
            processes--;
        }
        exit(EXIT_SUCCESS);
    }
    exit(EXIT_FAILURE);
}
