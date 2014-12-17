#include <stdlib.h>
#include "config.h"

#define INTRODUCTION_MTYPE 2
#define MAX_REQUIRE_RESOURCES 32    /* The maximum number of resource a client can require. */


typedef struct _introduction {
    long int mtype;
    pid_t pid;
    char resources[MAX_REQUIRE_RESOURCES][MAX_RES_NAME_LENGTH];
    int resources_length;
} introduction;
