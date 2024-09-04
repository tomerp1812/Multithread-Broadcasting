#ifndef UNBOUNDED_H
#define UNBOUNDED_H

#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Struct for unbounded queue nodes
typedef struct unBoundedList {
    struct unBoundedList *next;
    struct unBoundedList *prev;
    char *data;
} unBoundedList;

// Struct for unbounded queue
typedef struct unBounded {
    sem_t full;
    sem_t mutex;
    unBoundedList *head;
    unBoundedList *tail;
} unBounded;

extern unBounded sports;
extern unBounded news;
extern unBounded weather;
extern const char *broadCast[3];

// Function declarations
void initReport();
void insertUnbounded(unBounded *uB, char *article);
char *removeObjUnbounded(unBounded *uB);

#endif
