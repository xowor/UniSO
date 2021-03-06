#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <math.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "resource.h"
#include "so_log.h"
#include "messages/introduction.h"
#include "messages/simple_message.h"
#include "messages/tao_info_to_agent.h"
#include "messages/auction_result.h"
#include "messages/auction_status.h"
#include "messages/client_status.h"


int master_msqid;
int msqid;
int client_num;
int pid;
int ppid;
int agent_pid_msqid[MAX_REQUIRED_RESOURCES][2];
int pid_msqid_length = 0;
int budget;
int number_required_resources = 0;				/* n of resources wanted*/

int agent_list[MAX_REQUIRED_RESOURCES];		/* agent's pid*/
int number_of_agents = 0;
int status = CLIENT_OK;				/* The internal status of the client */

resource_list* req_resources;       /* The list containing all the available resources */


int agent_msqid_from_pid(int agent_pid) {
	int j = 0;
	for (; j < pid_msqid_length; j++){
		if (agent_pid_msqid[j][0] == agent_pid){
			return agent_pid_msqid[j][1];
		}
	}
}


/* Creates agent process. */
int create_agent_process(){
	int agent_processes_msqid = msgget(IPC_PRIVATE, 0600 | IPC_CREAT);
	char str_msqid [32];
	sprintf(str_msqid, "%d", agent_processes_msqid);

	int agent_pid = fork();
	if ( agent_pid == -1 ){
        printf("[main] Error: agent not created.");
        fprintf(stderr, "\t%s\n", strerror(errno));
        perror("fork");
        exit(EXIT_FAILURE);
    } else if ( agent_pid == 0 ) {
		fflush(stdout);
        /*  Child code */
        char *envp[] = { NULL };
        char *argv[] = { "./agent", "-m", str_msqid, NULL };
        /* Run agent process. */
        int agent_execve_err = execve("./agent", argv, envp);
        if (agent_execve_err == -1) {
            /* Cannot find the client binary in the working directory */
            fprintf(stderr, "[agent] Error: cannot start agent process (Try running main from ./bin).\n");
            /* strerror interprets the value of errnum, generating a string with a message that describes the error */
            fprintf(stderr, "\t%s\n", strerror(errno));
			exit(EXIT_FAILURE);
        }
    } else {
        /* Parent code */
        agent_pid_msqid[pid_msqid_length][0] = agent_pid;
        agent_pid_msqid[pid_msqid_length][1] = agent_processes_msqid;
        pid_msqid_length++;
        return agent_pid;
    }
}

/**
 * Communicates to agents all the necessary informations.
 */
void notify_tao_info(int pid, int availability, int cost, int shmid, int semid, int base_bid, char resource_name[MAX_RES_NAME_LENGTH], int budget){
	/* Allocates the simple_message message */
	tao_info_to_agent* msg = (tao_info_to_agent*) malloc(sizeof(tao_info_to_agent));
	msg->mtype = TAO_INFO_TO_AGENT_MTYPE;
	strcpy(msg->resource_name, resource_name);
	msg->availability = availability;
	msg->cost = cost;
	msg->shmid = shmid;
	msg->semid = semid;
	msg->base_bid = base_bid;
	msg->budget = budget / req_resources->resources_count;

	int j = 0;
	for (; j < pid_msqid_length; j++){
		if (agent_pid_msqid[j][0] == pid){
			msgsnd(agent_pid_msqid[j][1], msg, sizeof(tao_info_to_agent) - sizeof(long), 0600);
		}
	}
  free(msg);
}

/* Creates a single agent */
void create_agent(char* resource_name, int shmid, int semid, int base_bid){
	int agent_pid = create_agent_process();
	agent_list[number_of_agents] = agent_pid;
	number_of_agents++;

	resource* res;
	res = req_resources->list;
	/* Communicates tao information to agent */
	while(res){
		/* Searches specific resource from list */
    	if(strcmp(res->name, resource_name) == 0){
			notify_tao_info(agent_pid, res->availability, res->cost, shmid, semid, base_bid, res->name, budget);
			res->agent_pid = agent_pid;
		}
		res = res->next;
	}
}

/* Listen for the msqid with the auctioneer */
void listen_msqid(){
	simple_message* msg = (simple_message*) malloc(sizeof(simple_message));
	//msgflag = 0 -> bloccante
	if ( msgrcv(master_msqid, msg, sizeof(simple_message) - sizeof(long), SIMPLE_MESSAGE_MTYPE, 0) != -1 ) {
		msqid = msg->content.i;
	}

	free(msg);
}

/**
 * Loads the resources required by the client from file.
 */
void load_client_resources(){
    FILE* resources;
    char line[64];
    char* name;
    char* tmp;
    char* token;
    int avail = 0, cost = 0, i = 0, resourcesNumber = 0, number_line = 0;
    char filename[1024];
    req_resources = create_resource_list();
    /* Reads the resource list according to the client number. */
    sprintf(filename, "../resources/clients/%d.txt", client_num);
    resources = fopen(filename, "r");
    if( resources != NULL ){
      /* Reads each line from file */
      while( fgets(line, 64, resources) != NULL ){
				token = strtok(line, ";");
				if(number_line == 0){
					budget = atoi(token);
					number_line++;
				}else{
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
					/* Adds the read resources inside the required resources list */
					add_resource(req_resources, name, avail, cost);
					number_required_resources++;
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
    /* Allocates the introduction message */
    introduction* intr = (introduction*) malloc(sizeof(introduction));
    /* Initializes PID */
    intr->mtype = INTRODUCTION_MTYPE;
		intr->msqid = msqid;
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
    } while(res);

    /**
     * Sends a message to the auctioneer containing the client pid and the
     * required resources.
     */
    msgsnd(msqid, intr, sizeof(introduction) - sizeof(long), 0600);
	free(intr);
}

/* Notifies the start of his auction */
void notify_agent_start(int agent_pid){
  	simple_message* msg = (simple_message*) malloc(sizeof(simple_message));
    msg->mtype = SIMPLE_MESSAGE_MTYPE;
    msg_content content;
    msg->content = content;

    /* Initializes PID */
    msg->pid = getpid();
    strcpy(msg->msg, AUCTION_START_MSG);

    /* Gets the message queue id of the client */
	int agent_msqid = agent_msqid_from_pid(agent_pid);
	msgsnd(agent_msqid, msg, sizeof(simple_message) - sizeof(long), 0600);
    free(msg);
}


/* Notifies the client status to the auctioneer */
void notify_client_status(int status){
	client_status* msg = (client_status*) malloc(sizeof(client_status));
	msg->type = getpid();
	msg->mtype = CLIENT_STATUS_MTYPE;
	msg->type = status;

	msgsnd(msqid, msg, sizeof(client_status) - sizeof(long), 0600);

	free(msg);
}

/* Removes an agent from the agents list */
void remove_agent_from_list(int agent_pid){
	int i = 0;
	for(; i < MAX_REQUIRED_RESOURCES; i++)
		if(agent_list[i] == agent_pid)
			agent_list[i] = 0;
}

/* Lustens for action status messages */
void listen_auction_status(){
	FILE *file;
	char fname[256];
	sprintf(fname, "../results/%d.txt", client_num);
	file = fopen(fname, "w");
	fprintf(file, " ");

	// int _creations = 0;
	// int _starts = 0;
	// int _ends = 0;
	// int _results = 0;


	int i = 0;
	for (; i < req_resources->resources_count * 4; i++){
		/* For each auction, the client should receive 4 messagese (creation,
		 * start, end, results) */
		auction_status* msg = (auction_status*) malloc(sizeof(auction_status));
		if ( msgrcv(msqid, msg, sizeof(auction_status) - sizeof(long), AUCTION_STATUS_MTYPE, 0) != -1 ) {
			if (msg->type == AUCTION_CREATED) {
				/* Auction created, the agent can be created. */
				create_agent(msg->resource, msg->shm_id, msg->sem_id, msg->base_bid);
				notify_client_status(status);
			} else if (msg->type == AUCTION_STARTED) {
				/* Auction started, the agent can be notified of it. */
				resource* res = req_resources->list;
				while(res){
					if(strcmp(res->name, msg->resource) == 0){
						notify_agent_start(res->agent_pid);
					}
					res = res->next;
				}
				notify_client_status(status);
			} else if (msg->type == AUCTION_ENDED) {
				/* Auction ended, the agent can killed. */
				resource* res = req_resources->list;
				while(res){
					if(strcmp(res->name, msg->resource) == 0){
						signal(SIGCHLD, SIG_IGN);
						remove_agent_from_list(res->agent_pid);
						number_of_agents--;
						int p_status;
						kill(res->agent_pid, SIGKILL);
						waitpid(res->agent_pid, &p_status, WUNTRACED);
						/* Removes the queue message */
						int agent_msqid = agent_msqid_from_pid(res->agent_pid);
						msgctl(agent_msqid, IPC_RMID,0);
					}
					res = res->next;
				}
				number_required_resources--;
				notify_client_status(status);
			} else if (msg->type == AUCTION_RESULT){
				/* Auction results received, now can be printed to file */
				if (msg->quantity > 0){
					FILE *file;
					char fname[256];
					sprintf(fname, "../results/%d.txt", client_num);
					file = fopen(fname, "a");
					fprintf(file, "[client] [%d] Won %d units of resource %s. Total amount: %d.\n", pid, msg->quantity, msg->resource, (msg->quantity)*(msg->unit_bid));
					// printf("[client] [%d] Won %d units of resource %s. Total amount: %d.\n", pid, msg->quantity, msg->resource, (msg->quantity)*(msg->unit_bid));
				}
				notify_client_status(status);
			}
		}
		free(msg);
		// printf("creations: %d   starts: %d    ends:%d     results:%d\n", _creations, _starts, _ends, _results);
	}
	free(req_resources);
	fclose(file);
}


/* Handles the SIGINT and kills the agents */
void sigint_signal_handler(){
	// kills agents
	int i = 0;
	for(; i < MAX_REQUIRED_RESOURCES; i++)
		if(agent_list[i] != 0)
			kill(agent_list[i], SIGKILL);
	exit(EXIT_SUCCESS);
}

void listen_sigint_signal(){
	if (signal(SIGINT, sigint_signal_handler)== SIG_ERR)
		perror("signal (SIG_ERR) error");
}

int main(int argc, char** argv){
    pid = getpid();
    ppid = getppid();
    printf("[client] Started client.\tPid: %d\tPPid: %d\n", pid, ppid);

    if (argc >= 4 && strcmp(argv[1], "-m") == 0 && strcmp(argv[3], "-c") == 0){
        master_msqid = atoi(argv[2]);
        client_num = atoi(argv[4]);
        //fprintf(stdout, "[client][%d][%d] Using message queue %d\n", client_num, pid, master_msqid);
    } else {
        fprintf(stderr, "[client][%d] Error: master_msqid (-m) or client number (-c) argument not valid.\n", pid);
        return -1;
    }
	listen_sigint_signal();

	listen_msqid();


	load_client_resources();

	send_introduction();

	listen_auction_status();

	// TO_SEE detacharsi dall'area condivisa

	/* Close message queue */
	msgctl(msqid, IPC_RMID,0);

	fprintf(stdout, "[client][%d][%d] \x1b[31mQuitting... \x1b[0m \n", client_num, pid);
	fflush(stdout);

	// lo stato di terminazione del figlio è  restituito nell'argomento status della wait --> processo padre viene svegliato

	return 0;
}
