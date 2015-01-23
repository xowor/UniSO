#include <stdlib.h>
#include "../config.h"

#define AUCTION_STATUS_MTYPE 6



enum auction_status_type {
    AUCTION_CREATED = 0,
    AUCTION_STARTED = 1,
    AUCTION_ENDED = 2,
    AUCTION_RESULT = 3,
    UNREGISTRATION = 4
};



typedef struct _auction_status {
    long int mtype;
    char resource[MAX_RES_NAME_LENGTH];
    enum auction_status_type type;
    int shm_id;
    int sem_id;
    int base_bid;
    int quantity;
    int unit_bid;
} auction_status;
