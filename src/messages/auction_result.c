#include <stdlib.h>
#include "../config.h"

#define AUCTION_RESULT_MTYPE 5

typedef struct _auction_result {
    long int mtype;
    char resource[MAX_RES_NAME_LENGTH];
    int quantity;
    int unit_bid;
} auction_result;
