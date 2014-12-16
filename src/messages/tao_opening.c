#include <stdlib.h>
#include "../config.h"

#define TAO_OPENING_MTYPE 3

typedef struct _tao_opening {
    long int mtype;
    char resource[MAX_RES_NAME_LENGTH];
    int shmid;
    int semid;
    int base_bid;
} tao_opening;
