#include <stdlib.h>
#include "config.h"

typedef struct _simple_message {
    pid_t pid;
    char msg[MAX_SIMPLE_MESS_LENGTH];
} simple_message;
