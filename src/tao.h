#ifndef TAO_H
#define TAO_H

#include "tao.c"

void init_taos_array(int number);
void create_tao(char name[MAX_RES_NAME_LENGTH], int cost);
tao* get_tao(int i);
tao* get_tao_by_name(char name[MAX_RES_NAME_LENGTH]);
tao* get_tao_by_id(int id);
void register_client_to_tao(pid_t pid, char name[MAX_RES_NAME_LENGTH]);
void start_tao(tao* current_tao);
void init_tao(tao* current_tao);

#endif // TAO_H
