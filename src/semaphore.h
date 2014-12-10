#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include "semaphore.c"

int sem_p (int semid, int semnum);
int sem_v (int semid, int semnum);

#endif // SEMAPHORE_H
