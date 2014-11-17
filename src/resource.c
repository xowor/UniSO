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

/**
 * Creates a node and adds it at the head of the list.
 * @param name Resource's name
 * @param a Resource's availability
 * @param c Resource's cost
 * @param nextNode Next node of this resource 
 * @return 
 */
resourcesList node_creation(char name[N], int a, int c, resourcesList* nextNode){
    resourcesList rl = (resourcesList) malloc(sizeof(resource));
    strcpy(rl->name, name);
    rl->availability = a;
    rl->cost = c;
    if ( *nextNode != NULL)
        rl->next = *nextNode;
    return rl;
}
