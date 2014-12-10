#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "config.h"

// #include "log.c"


// typedef resource* resourcesList;

/* Information related to each resource. */
typedef struct _resource {
    char* name;
    int availability;
    int cost;
    struct _resource* next;
} resource;

/* List of general resources available to be sell. */
typedef struct _resource_list {
    resource* list;
    int resources_count;	// cos'è???? quantità di risorse totali??
} resource_list;


/**
 * Creates a node and adds it at the head of the list.
 * @param list The resource list pointer
 * @param name Resource's name
 * @param avail Resource's availability
 * @param cost Resource's cost
 * @return node of resourcesList
 */
resource* add_resource(resource_list* list, char name[MAX_RES_NAME_LENGTH], int avail, int cost){
    resource* new_resource = (resource*) malloc(sizeof(resource));
    new_resource->name = malloc(MAX_RES_NAME_LENGTH);
    strcpy(new_resource->name, name);
    new_resource->availability = avail;
    new_resource->cost = cost;
    new_resource->next = list->list;
    list->list = new_resource;
    list->resources_count++;
    // printf("[[add_resource]] resource_list* list : %p , char name[MAX_RES_NAME_LENGTH] : %s , int avail %d , int cost %d \n", list, name, avail, cost);
    return new_resource;
}
