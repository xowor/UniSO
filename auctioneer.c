#include <stdio.h>

// un tao per risorsa
// area di memoria condivisa
// max 5 offerte per tao
typedef struct{
    // pid client
    // quantita richiesta
    // offerta per unità
}tao;

// banditore può creare al max 3 tao contemporaneamente

// lista di clienti registrati
typedef struct _node{
    // pid del cliente
    // risorse interessate (array)
    struct _node* next;
}client;
typedef client* clientList;


// mai in sleep

int main(){
    printf("[auctioneer] Started auctioneer.\n");
    return 0;
}
