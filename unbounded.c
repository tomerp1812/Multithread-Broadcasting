#include "unbounded.h"


//the 4 queues that are not the producers
unBounded sports;
unBounded news;
unBounded weather;

//global variables
const char *broadCast[3] = {"SPORTS", "WEATHER", "NEWS"};

//initialize the reports semaphores
void initReport() {
    sem_init(&sports.full, 0, 0);
    sem_init(&sports.mutex, 0, 1);
    sem_init(&weather.full, 0, 0);
    sem_init(&weather.mutex, 0, 1);
    sem_init(&news.full, 0, 0);
    sem_init(&news.mutex, 0, 1);
}

/**
 * function that inserts an object into unbounded queue, which happens in critical section that
 * is being protected by semaphores
 * **/
void insertUnbounded(unBounded *uB, char *article) {
    sem_wait(&uB->mutex);
    unBoundedList *nList = malloc(sizeof(unBoundedList));
    if (uB->head == NULL) {
        //no items in the list
        uB->head = nList;
        uB->tail = uB->head;
        uB->head->next = NULL;
        uB->head->prev = NULL;
        uB->head->data = malloc(sizeof(char) * (strlen(article) + 1));
        strcpy(uB->head->data, article);
    } else {
        //1 or more items in the list
        nList->data = malloc(sizeof(char) * (strlen(article) + 1));
        strcpy(nList->data, article);
        nList->next = uB->head;
        uB->head->prev = nList;
        uB->head = nList;
        uB->head->prev = NULL;
    }
    sem_post(&uB->mutex);
    sem_post(&uB->full);
}

/**
 * function that removes object from an unbounded queue
 * **/
char *removeObjUnbounded(unBounded *uB) {
    sem_wait(&uB->full);
    sem_wait(&uB->mutex);
    char *article = NULL;
    if (uB->head == uB->tail) {
        // Only one node in the list
        article = strdup(uB->tail->data);
        free(uB->tail->data);
        free(uB->tail);
        uB->head = NULL;
        uB->tail = NULL;
    } else {
        // Multiple nodes in the list
        article = strdup(uB->tail->data);
        free(uB->tail->data);
        unBoundedList *temp = uB->tail;
        uB->tail = uB->tail->prev;
        uB->tail->next = NULL;
        free(temp);
    }
    sem_post(&uB->mutex);
    return article;
}