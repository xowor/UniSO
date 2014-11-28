#include <stdlib.h>
#include <stdio.h>
#define MAX_OFFER 5
// TAO = LISTA DI OFFERTE

// un tao per risorsa / ogni tao contiene max 5 offerte

typedef struct _bid{
    int client_pid;
    int quantity;
    int unit_offer;
} bid;

// area di memoria condivisa
// typedef struct _tao* taoList;
typedef struct _tao{
    // char* id;
    // bid singleOffer;
    bid bids[MAX_OFFER];
    // struct _tao* next;
} tao;


int offers_count = 0;

tao* create_tao(){
    tao* new_tao = (tao*) malloc(sizeof(tao));
    // new_tao -> id = id;
    // new_tao->singleOffer = so;
    // new_tao->bids = {};
    // if ( *nextNode != NULL)
    //     new_tao->next = *nextNode;
    return new_tao;
}

int make_bid(int pid, int quantity, int unit_offer){
    bid* new_bid = (bid*) malloc(sizeof(bid));
    return -1;
}

// /**
//  * Creates a node and adds it at the head of the list.
//  * @param id single tao's id
//  * @param so client's bid
//  * @param nextNode Next node of this tao
//  * @return node of taoList
//  */
// tao* node_creation(int id, bid so){
//     tao* new_tao = (tao*) malloc(sizeof(tao));
//     new_tao -> id = id;
//     new_tao->singleOffer = so;
//     if ( *nextNode != NULL)
//         new_tao->next = *nextNode;
//     return new_tao;
// }
