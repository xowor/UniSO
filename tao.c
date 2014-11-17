#include <stdlib.h>
#include <stdio.h>
#define MAXOFFER 5
// TAO = LISTA DI OFFERTE

// un tao per risorsa / ogni tao contiene max 5 offerte

typedef struct{
    int client_pid;
    int availability;
    int unitOffer;
}offer;

// area di memoria condivisa
typedef struct _elementTao* taoList;
typedef struct _elementTao{
    int id;
    offer singleOffer;
    taoList next;
} tao;

/**
 * Creates a node and adds it at the head of the list.
 * @param id single tao's id
 * @param so client's offer
 * @param nextNode Next node of this tao 
 * @return node of taoList
 */
taoList node_creation(int id, int a, Offer so, taoList* nextNode){
    taoList tl = (taoList) malloc(sizeof(tao));
    tl -> id = id;
    tl->singleOffer = so;
    if ( *nextNode != NULL)
        tl->next = *nextNode;
    return tl;
}
