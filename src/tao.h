#ifndef TAO_H
#define TAO_H

#include "tao.c"

typedef struct _bid bid;
typedef struct _tao tao;

tao* create_tao();
int make_bid(int pid, int quantity, int unit_offer, tao* auction_tao);

#endif // TAO_H
