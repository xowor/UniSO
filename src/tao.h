#ifndef TAO_H
#define TAO_H

#include "tao.c"

typedef struct _bid bid;
typedef struct _tao tao;

void init_taos(int number);
void create_tao(char name[MAX_RES_NAME_LENGTH]);
tao* get_tao(int i);
void sign_to_tao(pid_t pid, char name[MAX_RES_NAME_LENGTH]);
void replace_bids(int n, bid* new_bid, tao* auction_tao);
void start_tao();
int make_bid(int pid, int quantity, int unit_offer, tao* auction_tao);

#endif // TAO_H
