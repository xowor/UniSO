#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "config.h"

// #include "log.c"


// typedef resource* resourcesList;


typedef struct _resource {
    char name[MAX_REQUIRED_RESOURCES];
    int availability;
    int cost;
    struct _resource* next;
} resource;


typedef struct _resource_list {
    resource* list;
    int resources_count;
} resource_list;


resource_list* create_resource_list(){
    resource_list* list = (resource_list*) malloc(sizeof(resource_list));
    list->list = 0;
    list->resources_count = 0;
    return list;
}

resource* add_resource(resource_list* list, char name[MAX_RES_NAME_LENGTH], int avail, int cost){
    resource* new_resource = (resource*) malloc(sizeof(resource));
    // new_resource->name = malloc(MAX_RES_NAME_LENGTH);
    strcpy(new_resource->name, name);
    new_resource->availability = avail;
    new_resource->cost = cost;
    new_resource->next = list->list;
    list->list = new_resource;
    list->resources_count++;
    //printf("[[add_resource]] resource_list* list : %p , char name[MAX_RES_NAME_LENGTH] : %s , int avail %d , int cost %d \n", list, name, avail, cost);

    return new_resource;
}
