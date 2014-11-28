#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>

#define N 20


// typedef resource* resourcesList;

typedef struct _resource {
    char* name;
    int availability;
    int cost;
    struct _resource* next;
} resource;


resource* resource_list = NULL;

int resources_count = 0;


/**
 * Creates a node and adds it at the head of the list.
 * @param name Resource's name
 * @param avail Resource's availability
 * @param cost Resource's cost
 * @return node of resourcesList
 */
resource* add_resource(char name[N], int avail, int cost){
    resource* new_resource = (resource*) malloc(sizeof(resource));
    strcpy(new_resource->name, name);
    new_resource->availability = avail;
    new_resource->cost = cost;
    if ( resource_list != NULL){
        new_resource->next = resource_list;
        resource_list = new_resource;
        resources_count++;
    }
    return new_resource;
}
