#ifndef INTRODUCTION_H
#define INTRODUCTION_H

#include "introduction.c"

/**
* A message that can be used to notice the opening of a TAO.
* @param pid The pid of the (client) process.
* @param msqid The message queue id the process has choosen.
* @param resources The resources required by the client.
* @param base_bid The length of the resources array.
*/
typedef struct _introduction introduction;

#endif // INTRODUCTION_H
