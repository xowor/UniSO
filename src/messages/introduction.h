#ifndef INTRODUCTION_H
#define INTRODUCTION_H

#include "introduction.c"

/**
* A message that can be used to notice the opening of a TAO.
* @param resource The name of the resource the TAO trades.
* @param shmid The shared memory id.
* @param semid The semaphore which regulates the shared memory access.
* @param base_bid The base bid.
*/
typedef struct _introduction introduction;

#endif // INTRODUCTION_H
