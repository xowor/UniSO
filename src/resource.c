#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "config.h"
#include "so_log.h"

/**
* Information related to each resource. Has a pointer to use it as a list.
* @param name The name of the resource.
* @param availability The availability of the resource.
* @param cost The cost of the resource.
* @param next The next element of the resource list.
*/
typedef struct _resource {
    char name[MAX_REQUIRED_RESOURCES];
    int availability;
    int cost;
    int agent_pid;
    struct _resource* next;
} resource;


/**
* List of general resources available to be sell.
* @param list The pointer to the first element of the list;
* @param resources_count The number of resources (elements) in the list.
*/
typedef struct _resource_list {
    resource* list;
    int resources_count;    /* List's size */
} resource_list;


/**
* Creates a resource list.
* @return a resource_list.
*/
resource_list* create_resource_list(){
    resource_list* list = (resource_list*) malloc(sizeof(resource_list));
    list->list = 0;
    list->resources_count = 0;
    return list;
}


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
    strcpy(new_resource->name, name);
    new_resource->availability = avail;
    new_resource->cost = cost;
    new_resource->next = list->list;
    new_resource->agent_pid = -1;
    list->list = new_resource;
    list->resources_count++;
    //printf("[[add_resource]] resource_list* list : %p , char name[MAX_RES_NAME_LENGTH] : %s , int avail %d , int cost %d \n", list, name, avail, cost);
    return new_resource;
}


/**
* Loads the resources from the given file, and appends them to the given
* resource list.
* @param filename The resources file name;
* @param list The resources list;
*/
resource* get_resource(char* name, resource_list* list){
    resource* tmp_resource = list->list;
    /* adds name's resource and common informations to each tao */
    while(tmp_resource){
        if(strcmp(tmp_resource->name, name) == 0){
            return tmp_resource;
        }
        tmp_resource = tmp_resource->next;
    }
}



void get_resource_from_line(char* line, resource_list* list) {
    char* tok;
    int i;
    char* name = (char*) malloc(MAX_RES_NAME_LENGTH);
    int avail = 0;
    int cost = 0;
    tok = strtok(line, ";");
    for (i = 0; i < 3; i++){
        if (i == 0){
            strcat(tok, "\0");
            strcpy(name, tok);
        } else if (i == 1) {
            avail = atoi(tok);
        } else if (i == 2) {
            cost = atoi(tok);
        }
        tok = strtok(NULL, ";\n");
    }
    add_resource(list, name, avail, cost);
    // printf("Resource available: %s %d %d \n", name, avail, cost);
}

void load_resources(char filename[256], resource_list* list) {
    FILE* resources;
    resources = fopen(filename, "r");
    if( resources != NULL ){
        char line[64];
        while (fgets(line, 64, resources)){
            get_resource_from_line(line, list);
        }
    }
}
