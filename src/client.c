#include <stdio.h>

typedef struct _client {
    //pid

} client;

int main(){
    printf("[client] Started client.\n");

    if (argc >= 2 && strcmp(argv[1], "-m") == 0){
        msqid = atoi(argv[2]);
    } else {
        fprintf(stderr, "[auctioneer] Error: msqid (-m) argument not valid.\n");
        return -1;
    }

    return 0;
}
