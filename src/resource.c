#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>

#define MAX_RES_NAME_LENGTH 16


// typedef resource* resourcesList;

typedef struct _resource {
    char* name;
    int availability;
    int cost;
    struct _resource* next;
} resource;


typedef struct _resource_list {
    resource* list;
    int resources_count;
} resource_list;


// resource* resource_list = NULL;
//
// int resources_count = 0;


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
    // if ( list->list != NULL){
        new_resource->next = list->list;
        list->list = new_resource;
        list->resources_count++;
    // }
    return new_resource;
}
