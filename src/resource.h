#ifndef RESOURCE_H
#define RESOURCE_H

#include "resource.c"

typedef struct _resource* resourcesList;
typedef struct _resource_list resource_list;
resourcesList add_resource(resource_list* list, char name[MAX_RES_NAME_LENGTH], int a, int c/*, resourcesList* nextNode*/);

#endif // RESOURCE_H
