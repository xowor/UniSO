#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "resource.h"
#include "so_log.h"
#include "messages/introduction.h"
#include "messages/tao_opening.h"
#include "messages/tao_info_to_agent.h"


int msqid;
int client_num;
int pid;
int ppid;
// char* required_resources[MAX_REQUIRED_RESOURCES];
int required_resources_length = 0;
int budget;

resource_list* req_resources;       /* The list containing all the available resources */

/**
 *  Creates agent's process.
 */
int create_agent_process(){
	char str_msqid [32];
    sprintf(str_msqid, "%d", msqid);
    
    //char str_client_num [6];
    //sprintf(str_client_num, "%d", client_num);
    
	int agent_pid = fork();
	
	if ( agent_pid == -1 ){
        printf("[main] Error: agent not created.");
        fprintf(stderr, "\t%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if ( agent_pid == 0 ) {
        /*  Child code */
        // per coda di messaggi modificare argv
        char *envp[] = { NULL };
        char *argv[] = { "./agent", "-m", str_msqid, NULL };
        /* Run the client process. */
        int agent_execve_err = execve("./agent", argv, envp);
        if (agent_execve_err == -1) {
            /* Cannot find the client binary in the working directory */
            fprintf(stderr, "[agent] Error: cannot start agent process (Try running main from ./bin).\n");
            /* strerror interprets the value of errnum, generating a string with a message that describes the error */
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
 * Client communicates to agents resource informations.
 */
void notify_tao_info(int availability, int cost, int shmid, int semid, int basebid, char res[MAX_RES_NAME_LENGTH], int budget){
	/* Allocates the simple_message message */
	tao_info_to_agent* msg = (tao_info_to_agent*) malloc(sizeof(tao_info_to_agent));
	msg->mtype = TAO_INFO_TO_AGENT_MTYPE;
	strcpy(msg->res, res);
	msg->availability = availability;
	msg->cost = cost;
	msg->shmid = shmid;
	msg->semid = semid;
	msg->basebid = basebid;
	msg->budget = budget;
	
	msgsnd(msqid, msg, sizeof(tao_info_to_agent) - sizeof(long), 0600);
    free(msg);
}

/**
 * Creates a single agent for each tao call 
 */
void create_agent(char* resource, int shmid, int semid, int basebid){
	create_agent_process();
	
	/* Communicates to agent*/
	while(req_resources->list){
        if(strcmp(req_resources->list->name, resource) == 0)
			notify_tao_info(req_resources->list->availability, req_resources->list->cost, shmid, semid, basebid, req_resources->list->name, budget);
		req_resources->list = req_resources->list->next;
	}

    //// msgrcv CLIENTE DEVE RIMANERE IN ATTESA DI ALTRI MESSAGGI DA PARTE DEL BANDITORE

}

/**
 * Loads the resources required by the client from file.
 */
void load_resources(){
    FILE* resources;
    char line[64];
    char* name;
    char* tmp;
    char* token;
    int avail = 0, cost = 0, i = 0, resourcesNumber = 0, number_line = 0;
    char filename[1024];
    req_resources = (resource_list*) malloc(sizeof(resource_list));
    /* Reads the resource list according to the client number. */
    sprintf(filename, "../resources/clients/%d.txt", client_num);
    resources = fopen(filename, "r");
    if( resources != NULL ){
        /* Reads each line from file */
        token = strtok(line, ";");
        if(number_line == 0){
			budget = (int)token;
			number_line++;
		}else{
			while( fgets(line, 64, resources) != NULL ){
				i = 0;
				name = (char*) malloc(64);
				while( token ){
					/* In each line there are 4 tokens: name, available, cost and \n */
					switch(i%4){
						case 0:
							strcat(token, "\0");
							strcpy(name, token);
							break;
						case 1:
							tmp = token;
							avail = atoi(tmp);
							break;
						case 2:
							tmp = token;
							cost = atoi(tmp);
							break;
					}
					i++;
					token = strtok(NULL, ";");
				}
				//printf("[client][%d][%d] Resource required: %s %d %d \n", client_num, pid, name, avail, cost);

				// resourcesNumber++;
				// required_resources[required_resources_length++] = name;

				/* Adds the read resources inside the required resources list */
				add_resource(req_resources, name, avail, cost);
			}
		}
    }else{
        fprintf(stderr, "[client][%d][%d] Error: Unable to open resource's file. %s\n", client_num, pid, strerror(errno));
    }

    fflush(stdout);
    fclose(resources);
}

/**
 *  Sends an introduction message to the auctioneer, telling it which resources
 *  this client requires.
 */
void send_introduction(){
    // so_log('m');
    // [TODO] SEMAFORO PER LA SCRITTURA
    /* Allocates the introduction message */
    introduction* intr = (introduction*) malloc(sizeof(introduction));
    /* Initializes PID */
    intr->mtype = INTRODUCTION_MTYPE;
    intr->pid = pid;
    intr->resources_length = 0;

    /* For each resource required adds its name in the introduction message. */
    resource* res = req_resources->list;
    int i = 0;
    do {
        strcpy(intr->resources[i], res->name);
        intr->resources_length++;
        i++;
        res = res->next;
    }
    while(res != NULL);

    /**
     * Sends a message to the auctioneer containing the client pid and the
     * required resources.
     */
    msgsnd(msqid, intr, sizeof(introduction) - sizeof(long), 0600);
	free(intr);
}

void listen_auction_start(){
    // [TODO] SEMAFORO PER LA LETTURA
    tao_opening* msg = (tao_opening*) malloc(sizeof(tao_opening));
    if ( msgrcv(msqid, msg, sizeof(tao_opening) - sizeof(long), TAO_OPENING_MTYPE, 0) != -1 ) {
        so_log_is('m', pid, "started_tao");
        /* Passes informations included in message queue to agent */
        create_agent(msg->resource, msg->shmid, msg->semid, msg->base_bid);
    }

	free(msg);
}


/**
* Collects all the IPC garbage
*/
void ipc_gc(){

}

/**
* Cleans the heap after quitting (heard is a good pratice...)
*/
void gc(){
	free(req_resources);
}

int main(int argc, char** argv){
    pid = getpid();
    ppid = getppid();
    printf("[client] Started client.\tPid: %d\tPPid: %d\n", pid, ppid);

    if (argc >= 4 && strcmp(argv[1], "-m") == 0 && strcmp(argv[3], "-c") == 0){
        msqid = atoi(argv[2]);
        client_num = atoi(argv[4]);
        //fprintf(stdout, "[client][%d][%d] Using message queue %d\n", client_num, pid, msqid);
    } else {
        fprintf(stderr, "[client][%d] Error: msqid (-m) or client number (-c) argument not valid.\n", pid);
        return -1;
    }

    load_resources();
    send_introduction();
    listen_auction_start();


	fprintf(stdout, "[client][%d][%d] \x1b[31mRemoving all the IPC structures... \x1b[0m \n", client_num, pid);
	ipc_gc();

	fprintf(stdout, "[client][%d][%d] \x1b[31mCleaning heap... \x1b[0m \n", client_num, pid);
	gc();


    fprintf(stdout, "[client][%d][%d] \x1b[31mQuitting... \x1b[0m \n", client_num, pid);
    fflush(stdout);

    // se banditore comunica acquisizione di risorsa --> scrivere su file di log : risorsa acquisita; quantità; prezzoComplessivo
    // lo stato di terminazione del figlio è  restituito nell'argomento status della wait --> processo padre viene svegliato

    return 0;
}
