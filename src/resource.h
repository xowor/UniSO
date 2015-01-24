#ifndef RESOURCE_H
#define RESOURCE_H

#include "resource.c"

resource_list* create_resource_list();
resource* add_resource(resource_list* list, char name[MAX_RES_NAME_LENGTH], int avail, int cost);
resource* get_resource(char* name, resource_list* list);
void get_resource_from_line(char* line, resource_list* list);
void load_resources(char filename[256], resource_list* list);

#endif // RESOURCE_H
