#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include "semaphore.c"

/**
 * Decrements semaphore.
 * @param semid The id of the semaphore pool.
 * @param semnum The number of the semaphore.
 */
int sem_p (int semid, int semnum);


/**
 * Increments semaphore.
 * @param semid The id of the semaphore pool.
 * @param semnum The number of the semaphore.
 */
int sem_v (int semid, int semnum);

#endif // SEMAPHORE_H
