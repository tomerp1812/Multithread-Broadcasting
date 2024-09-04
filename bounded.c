#include "bounded.h"

int numberOfProducers;
bounded coEditor;
int doneCounter = 0;
int originalNumOfProds;

/**
 * initializes all the bounded queues
 * **/
void createBoundedQ(producer *p) {
    p->boundedQ = malloc(sizeof(bounded));
    sem_init(&p->boundedQ->empty, 0, p->queueSize);
    sem_init(&p->boundedQ->full, 0, 0);
    sem_init(&p->boundedQ->mutex, 0, 1);
    p->boundedQ->first = 0;
    p->boundedQ->last = 0;
    p->boundedQ->queueSize = p->queueSize;
    p->boundedQ->numOfProductsLeft = p->numOfProducts;
    if (p->numOfProducts > 0) {
        p->boundedQ->isProdLeft = true;
        //if no products to produce just setting the bool of is there any products to
        // produce to false and decreases the number of producers left
    } else {
        p->boundedQ->isProdLeft = false;
        --numberOfProducers;
    }

    p->boundedQ->data = malloc(sizeof(char *) * p->queueSize);
}

/**
 * insertion to bounded queue either the co-editor or one of the producers
 * **/
void insertBounded(bounded *b, const char *article) {
    sem_wait(&b->empty);
    sem_wait(&b->mutex);
    //the modulo is for the round-robin algorithm
    b->data[b->last % b->queueSize] = malloc(sizeof(char) * (strlen(article) + 1));
    strcpy(b->data[b->last % b->queueSize], article);
    //increases the distance between last and first object
    ++b->last;
    sem_post(&b->mutex);
    sem_post(&b->full);
}

/**
 * function that removes object from one of the producers queue
 * **/
char *removeObjBounded(bounded *b) {
    //lock
    sem_wait(&b->full);
    sem_wait(&b->mutex);
    char *article = strdup(b->data[b->first % b->queueSize]);
    free(b->data[b->first % b->queueSize]);
    b->data[b->first % b->queueSize] = NULL;
    //decreases the distance between last and first object
    ++b->first;
    --b->numOfProductsLeft;
    if (b->numOfProductsLeft == 0) {
        --numberOfProducers;
        b->isProdLeft = false;
    }
    //unlock
    sem_post(&b->mutex);
    sem_post(&b->empty);
    return article;
}

/**
 * initialization of the co-edit queue
 * **/
void createCoEditBounded(int coEditorQSize) {
    //init the semaphores
    sem_init(&coEditor.empty, 0, coEditorQSize);
    sem_init(&coEditor.full, 0, 0);
    sem_init(&coEditor.mutex, 0, 1);
    coEditor.data = malloc(sizeof(char *) * coEditorQSize);
    //init the array to 0
    coEditor.first = 0;
    coEditor.last = 0;
    coEditor.queueSize = coEditorQSize;
}

/**
 * function that removes object from the co-edit queue
 * **/
char *removeCoEditBounded() {
    sem_wait(&coEditor.full);
    sem_wait(&coEditor.mutex);
    char *article = strdup(coEditor.data[coEditor.first % coEditor.queueSize]);
    //we count the doneCounter to 3 , which will mean that all the queues was over
    if (strcmp(article, "DONE") == 0) {
        doneCounter++;
    }
    free(coEditor.data[coEditor.first % coEditor.queueSize]);
    coEditor.data[coEditor.first % coEditor.queueSize] = NULL;
    ++coEditor.first;
    sem_post(&coEditor.mutex);
    sem_post(&coEditor.empty);
    return article;
}

//for each of the producers it initializes its own queue
void createBoundedQueues(producer *prods) {
    for (int i = 0; i < originalNumOfProds; i++) {
        createBoundedQ(&prods[i]);
    }
}

//frees all the allocated space made in the program and was not cleared yet
void clean(producer* prods) {
    for (int i = 0; i < originalNumOfProds; i++) {
        if (prods[i].boundedQ != NULL) {
            if (prods[i].boundedQ->data != NULL) {
                free(prods[i].boundedQ->data);
                prods[i].boundedQ->data = NULL;
            }
            free(prods[i].boundedQ);
            prods[i].boundedQ = NULL;
        }
    }
}