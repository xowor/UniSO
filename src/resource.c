#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>

#define N 20

typedef struct _resource* resourcesList;
typedef struct _resource {
    char* name;
    int availability;
    int cost;
    resourcesList next;
}resource;

resourcesList node_creation(char name[N], int a, int c, resourcesList n){
    resourcesList rl = (resourcesList) malloc(sizeof(resource));
    strcpy(rl->name, name);
    rl->availability = a;
    rl->cost = c;
    rl->next = n;
    return rl;
}
