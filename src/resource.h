#ifndef RESOURCE_H
#define RESOURCE_H

#include "resource.c"

typedef struct _resource* resourcesList;
resourcesList add_resource(char name[MAX_RES_NAME_LENGTH], int a, int c/*, resourcesList* nextNode*/);

#endif // RESOURCE_H
