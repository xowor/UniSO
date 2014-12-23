#include <stdlib.h>
#include "../config.h"

#define CLIENT_STATUS_MTYPE 7



enum client_status_type {
    CLIENT_OK = 0,
    CLIENT_UNREGISTERED = 1
};



typedef struct _client_status {
    long int mtype;
    enum client_status_type type;
} client_status;
