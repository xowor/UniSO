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
    /*
    typedef client* clientList;
    // legge da file le risorse e crea i tao per ciascuno
    FILE* resources = fopen("resource.txt", "r");
    if (resources != NULL){
        int countAuction = 0;
        // LEGGERE DA FILE I VARI TOKEN PER RIGA
        while(1){        //finchè ci sono risorse da leggere
            if(countAuction < MAXTAO){
                //crea il tao corrispondente
                countAuction += 1;
            }else{
                // attesa che un'altra asta termini per farne partire un'altra
            }        
        }
    }else{
        printf("Error: file isn't open.");
    }
    fclose(resources);
    */
    return 0;
}
