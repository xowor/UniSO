#ifndef SO_LOG_H
#define SO_LOG_H

#include "so_log.c"

void so_log(char c);
void so_log_i(char c, int i);
void so_log_s(char c, char* s);
void so_log_p(char c, void* p);
void so_log_is(char c, int i, char* s);

// void so_deb(char* file, int line, )

#endif // SO_LOG_H
