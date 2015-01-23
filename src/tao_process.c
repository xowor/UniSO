#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include "so_log.h"
#include "resource.h"
#include "config.h"
#include "tao.h"
#include "semaphore.h"
#include "messages/simple_message.h"

int canexit = 0;

int tao_processes_msqid = 0;

int lifetime = 0;
int lifetime_counter;
int id_tao;

void alarm_handler() {
    if (lifetime_counter == 1){
        /* Lifetime timer */
        // printf("[tao_process][%d] Lifetime elapsed.\n", getpid());
        simple_message* msg = (simple_message*) malloc(sizeof(simple_message));
        msg->mtype = SIMPLE_MESSAGE_MTYPE;
        msg_content content;
        content.i = id_tao;
        msg->content = content;

        /* Initializes PID */
        msg->pid = getpid();
        strcpy(msg->msg, TAO_PROCESS_END_MSG);

        msgsnd(tao_processes_msqid, msg, sizeof(simple_message) - sizeof(long), 0600);

        free(msg);
        canexit = 1;
        //fprintf(stdout, "[tao_process] [%d] \x1b[31mQuitting... \x1b[0m \n", getpid());
    } else {
        /* Three seconds timer */
        // printf("[tao_process][%d] Three seconds elapsed, now waiting for %d seconds.\n", getpid(), lifetime);

        simple_message* msg = (simple_message*) malloc(sizeof(simple_message));
        msg->mtype = SIMPLE_MESSAGE_MTYPE;
        msg_content content;
        content.i = id_tao;
        msg->content = content;

        /* Initializes PID */
        msg->pid = getpid();
        strcpy(msg->msg, TAO_PROCESS_END_THREESEC);

        msgsnd(tao_processes_msqid, msg, sizeof(simple_message) - sizeof(long), 0600);

        free(msg);

        lifetime_counter = 1;
        signal(SIGALRM, alarm_handler);
        alarm(lifetime);
    }
    // exit(EXIT_SUCCESS);
}


/**
* Generates the auctioneer and some clients.
*/
int main(int argc, char** argv) {
    // printf("[tao_process] \x1b[32mTao process.\t\tPid: %d\x1b[0m\n", getpid());

    /**
    * Loads the process lifetime.
    */
    if (argc >= 4 && strcmp(argv[1], "-t")  == 0 && strcmp(argv[3], "-m") == 0 && strcmp(argv[5], "-i") == 0){
        lifetime = atoi(argv[2]);
        tao_processes_msqid = atoi(argv[4]);
        id_tao = atoi(argv[6]);
    } else {
        fprintf(stderr, "[tao_process] Error: lifetime (-t), id coda (-m) o id tao (-i) argument not valid.\n");
        return -1;
    }
    /* timer of 3 seconds before the start of auction */
    if(signal(SIGALRM, alarm_handler) == SIG_ERR)
        printf("[tao_process] [%d] Error in alarm signal.\n", getpid());
    lifetime_counter = 0;
    canexit = 0;
    // così non aspetta 3 secondi ogni volta che viene invocato??
    alarm(3);
    while(canexit != 1){};
    exit(EXIT_SUCCESS);
}
