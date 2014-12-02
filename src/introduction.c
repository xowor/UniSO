
#define MAX_REQUIRE_RESOURCES 32    /* The maximum number of resource a client can require. */


typedef struct _introduction {
    int pid;
    char resources[MAX_REQUIRE_RESOURCES][MAX_RES_NAME_LENGTH];
    int resources_length;
} introduction;
