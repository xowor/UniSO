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

void alarm_handler() {
    if (lifetime_counter){
        canexit = 1;
        simple_message* msg = (simple_message*) malloc(sizeof(simple_message));
        msg->mtype = SIMPLE_MESSAGE_MTYPE;
        msg_content content;
        msg->content = content;

        /* Initializes PID */
        msg->pid = getpid();
        strcpy(msg->msg, TAO_PROCESS_END_MSG);

        msgsnd(tao_processes_msqid, msg, sizeof(simple_message) - sizeof(long), 0600);

        free(msg);
        fprintf(stdout, "[Tao process] [%d] \x1b[31mQuitting... \x1b[0m \n", getpid());
        _exit(EXIT_SUCCESS);
    } else {
        // timer lifetime
        signal(SIGALRM, alarm_handler);
        // if(signal(SIGALRM, alarm_handler) == SIG_ERR)
        // 	printf("[Auctioneer] [%d] Error in alarm signal (timer lifetime).\n", getpid());
        // recuperare la durata di questo tao
        //alarm(lifetime del tao);

        // start_tao(current_tao);
        // messaggio di avvio asta ai clienti
        // notify_tao_start(current_tao);
        // printf("[Tao process] [%d] signal (timer lifetime).\n", getpid());

        simple_message* msg = (simple_message*) malloc(sizeof(simple_message));
        msg->mtype = SIMPLE_MESSAGE_MTYPE;
        msg_content content;
        msg->content = content;

        /* Initializes PID */
        msg->pid = getpid();
        strcpy(msg->msg, TAO_PROCESS_END_THREESEC);

        msgsnd(tao_processes_msqid, msg, sizeof(simple_message) - sizeof(long), 0600);

        free(msg);

        lifetime_counter = 1;
        alarm(lifetime);
    }
}


/**
* Generates the auctioneer and some clients.
*/
int main(int argc, char** argv) {
    printf("[Tao process] \x1b[32mTao process.\t\tPid: %d\x1b[0m\n", getpid());

    /**
    * Loads the process lifetime.
    */
    if (argc >= 4 && strcmp(argv[1], "-t")  == 0 && strcmp(argv[3], "-m") == 0){
        lifetime = atoi(argv[2]);
        tao_processes_msqid = atoi(argv[4]);
    } else {
        fprintf(stderr, "[Tao process] Error: lifetime (-t) argument not valid.\n");
        return -1;
    }
    /* timer of 3 seconds before the start of auction */
    if(signal(SIGALRM, alarm_handler) == SIG_ERR)
        printf("[Tao process] [%d] Error in alarm signal.\n", getpid());
    lifetime_counter = 0;
    canexit = 0;
    alarm(3);
    while(1){};
}
