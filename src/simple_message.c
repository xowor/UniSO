#include <stdlib.h>
#include "config.h"

typedef union _msg_content {
    int i;
    char s[MAX_SIMPLE_MESS_LENGTH];
} msg_content;

typedef struct _simple_message {
    pid_t pid;
    char msg[MAX_SIMPLE_MESS_LENGTH];
    msg_content content;
} simple_message;
