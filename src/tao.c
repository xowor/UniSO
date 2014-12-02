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
    //int riempimento = 0;
    // struct _tao* next;
} tao;


int offers_count = 0;

tao* create_tao(){
    tao* new_tao = (tao*) malloc(sizeof(tao));
    int j = 0;
    // new_tao -> id = id;
    // new_tao->singleOffer = so;
    // new_tao->bids = {};
    // if ( *nextNode != NULL)
    //     new_tao->next = *nextNode;
    return new_tao;
}


/**
 * Precondizione: l'agente ha già controllato se la sua offerta è tra le migliori --> non fa l'offerta con make bid
 * Precondizione: l'agente ha già aumentato la unit_offer rispetto alla offerta precedente
 * Precondizione: l'agente si può permettere la unit_offer
 *   * controlla l'offerta minima nel tao
     * l'agente ha già un'offerta nel tao?
     * 	se si --> controlla se l'offerta che vuole fare sia maggiore del minimo
     * 		  se si --> la sovrascrive quella già presente
     * 		  se no --> return
     * 	se no --> controlla se l'offerta che vuole fare sia maggiore del minimo
     * 		  se si --> scrive una nuova offerta a suo nome
     * 			ci sono entry vuote?
     * 				se si --> aggiunge la propria offerta nello spazio vuoto
     * 				se no --> sovrascrive l'offerta di prezzo minore
     * 		  se no --> return
 */
int make_bid(int pid, int quantity, int unit_offer, tao* auction_tao){
    // controlla l'offerta minima nel tao
    int i = 0, index_min_bid = 0;
    int min_bid = auction_tao->bids[0].unit_offer;
    int pid_min_bid = auction_tao->bids[0].client_pid;
    
    for(; i < MAX_OFFER; i++){
      if(auction_tao->bids[i].unit_offer < min_bid){
	min_bid = auction_tao->bids[i].unit_offer;
	index_min_bid = i;
	pid_min_bid = auction_tao->bids[i].client_pid;
      }
    }
    // nuovo bid
    bid* new_bid = (bid*) malloc(sizeof(bid));
    new_bid->client_pid = pid;
    new_bid->quantity = quantity;
    new_bid->unit_offer = unit_offer;
     
    // cerca entry vuota
    int empty_index = -1;
    for(i = 0; i < MAX_OFFER; i++){
      if(auction_tao->bids[i].client_pid < 0){
	empty_index = i;
      }
    }
    
    // l'agente ha già un'offerta nel tao?
    int has_bid = 0, own_bid = NULL;
    for(i = 0; i < MAX_OFFER; i++){
      if(auction_tao->bids[i].client_pid == pid){
	has_bid = 1;
	own_bid = i;
      }
    }
    
    // controlla se l'offerta che vuole fare è maggiore del minimo
    if(unit_offer > min_bid){
      // ha già un'offerta nel tao --> sostituisce la "propria entry"
      if(has_bid){
	/*
	auction_tao->bids[own_bid].client_pid = new_bid->client_pid;
	auction_tao->bids[own_bid].quantity = new_bid->quantity;
	auction_tao->bids[own_bid].unit_offer = new_bid->unit_offer;	
	*/
	replace_bids(own_bid, new_bid, auction_tao);
      // non ha offerte nel tao
      }else{
	// aggiunge la propria offerta nello spazio vuoto
	if(empty_index >= 0){
	  /*
	  auction_tao->bids[empty_index].client_pid = new_bid->client_pid;
	  auction_tao->bids[empty_index].quantity = new_bid->quantity;
	  auction_tao->bids[empty_index].unit_offer = new_bid->unit_offer;*/
	  replace_bids(empty_index, new_bid, auction_tao);
	// sovrascrive l'offerta di prezzo minore
	}else{
	  /*
	  auction_tao->bids[index_min_bid].client_pid = new_bid->client_pid;
	  auction_tao->bids[index_min_bid].quantity = new_bid->quantity;
	  auction_tao->bids[index_min_bid].unit_offer = new_bid->unit_offer;
	  */
	  replace_bids(index_min_bid, new_bid, auction_tao);
	}
      }
    }else
      return -1;    
}

/**
 * Support function to make_bid.
 */
replace_bids(int n, bid* new_bid, tao* auction_tao){
  bid tmp;
  tmp = auction_tao->bids[n];
  tmp.client_pid = new_bid->client_pid;
  tmp.quantity = new_bid->quantity;
  tmp.unit_offer = new_bid->unit_offer;
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
