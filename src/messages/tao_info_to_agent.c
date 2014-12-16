#include <stdlib.h>

#define TAO_INFO_TO_AGENT_MTYPE 4


typedef struct _tao_info_to_agent {
    long int mtype;
	int availability; 
	int cost;
	int shmid;
	int semid;
	int basebid;
} tao_info_to_agent;
 
