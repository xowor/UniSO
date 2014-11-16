#ifndef RESOURCE_H
#define RESOURCE_H

#include "resource.c"

typedef struct _resource* resourcesList;
resourcesList node_creation(char name[N], int a, int c, resourcesList n);

#endif // RESOURCE_H
