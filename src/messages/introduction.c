#include <stdlib.h>
#include "../config.h"

#define INTRODUCTION_MTYPE 2


typedef struct _introduction {
    long int mtype;
    pid_t pid;
    int msqid;
    char resources[MAX_REQUIRED_RESOURCES][MAX_RES_NAME_LENGTH];
    int resources_length;
} introduction;
