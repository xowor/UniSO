#ifndef RESOURCE_H
#define RESOURCE_H

#include "resource.c"


/**
 * Information related to each resource. Has a pointer to use it as a list.
 * @param name The name of the resource.
 * @param availability The availability of the resource.
 * @param cost The cost of the resource.
 * @param next The next element of the resource list.
 */
typedef struct _resource resource;


/**
 * List of general resources available to be sell.
 * @param list The pointer to the first element of the list;
 * @param resources_count The number of resources (elements) in the list.
 */
typedef struct _resource_list resource_list;



/**
* Creates a node and adds it at the head of the list.
* @param list The resource list pointer
* @param name Resource's name
* @param avail Resource's availability
* @param cost Resource's cost
* @return node of resourcesList
*/
resource* add_resource(resource_list* list, char name[MAX_RES_NAME_LENGTH], int avail, int cost);

#endif // RESOURCE_H
