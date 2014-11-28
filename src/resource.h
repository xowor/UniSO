#ifndef RESOURCE_H
#define RESOURCE_H

#include "resource.c"

typedef struct _resource* resourcesList;
resourcesList add_resource(char name[N], int a, int c/*, resourcesList* nextNode*/);

#endif // RESOURCE_H
