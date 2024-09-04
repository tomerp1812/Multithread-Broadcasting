#ifndef BOUNDED_H
#define BOUNDED_H

#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Struct for bounded queue
typedef struct bounded {
    sem_t empty;
    sem_t full;
    sem_t mutex;
    char **data;
    int first;
    int last;
    int queueSize;
    int numOfProductsLeft;
    bool isProdLeft;
} bounded;

// Struct for producer
typedef struct producer {
    int numOfProducts;
    int queueSize;
    int index;
    bounded *boundedQ;
} producer;

extern int numberOfProducers;
extern bounded coEditor;
extern int doneCounter;
extern int originalNumOfProds;

// Function declarations
void createBoundedQ(producer *p);
void insertBounded(bounded *b, const char *article);
char *removeObjBounded(bounded *b);
void createCoEditBounded(int coEditorQSize);
char *removeCoEditBounded();
void createBoundedQueues(producer *prods);
void clean(producer *prods);

#endif
