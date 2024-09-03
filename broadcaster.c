#include <pthread.h>
#include "stdio.h"
#include <unistd.h>
#include <fcntl.h>
#include "stdlib.h"
#include <string.h>
#include "semaphore.h"
#include "stdbool.h"

//bounded queue for the producers and co-Editor
struct bounded {
    sem_t empty;
    sem_t full;
    sem_t mutex;
    char **data;
    int first;
    int last;
    int queueSize;
    int numOfProductsLeft;
    bool isProdLeft;
} typedef bounded;

struct producer {
    int numOfProducts;
    int queueSize;
    int index;
    bounded *boundedQ;
} typedef producer;

//unbounded queue for Dispatcher
struct unBoundedList {
    struct unBoundedList *next;
    struct unBoundedList *prev;
    char *data;
} typedef unBoundedList;

struct unBounded {
    sem_t full;
    sem_t mutex;
    unBoundedList *head;
    unBoundedList *tail;
} typedef unBounded;

//global variables
const char *broadCast[3] = {"SPORTS", "WEATHER", "NEWS"};
int numberOfProducers;
int doneCounter = 0;
int originalNumOfProds;
//the 4 queues that are not the producers
unBounded sports;
unBounded news;
unBounded weather;
bounded coEditor;

////////////////////////////BOUNDED SECTION !!!!!

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



////////////////////////////UNBOUNDED SECTION !!!!!

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


/**
 * opens the configuration file
 * **/
int openFile(char *configuration) {
    int fd;
    //check if able to access and open the configuration file
    if (access(configuration, F_OK) == 0 && access(configuration, R_OK) == 0) {
        fd = open(configuration, O_RDONLY);
        if (fd == -1) {
            printf("Error in: open\n");
            exit(-1);
        }
    } else {
        printf("Error in: access\n");
        exit(-1);
    }

    return fd;
}

/**
 * reads the configuration file and counts the number of producer assuming
 * a specific configuration of the file
 * **/
int countNumOfProd(int fd) {
    int rd;
    char c;
    int count = 0;
    while (1) {
        rd = read(fd, &c, 1);
        if (rd < 0) {             //fail in reading the file
            printf("Error in: read\n");
            exit(-1);
        } else if (rd == 0) {     //finished reading the file
            break;
        } else {
            //count number of lines to figure number of producer
            if (c == '\n') {
                count++;
            }
        }
    }
    //each 4 lines there is new producer
    return (count - 1) / 4;
}

//reads the lines from configuration
char *readLine(int fd) {
    char *line = malloc(sizeof(char) * 100);
    int i = 0;
    char c = ' ';
    while (c != '\n') {
        //reads char char until \n
        if (read(fd, &c, 1) == -1) {
            printf("Error in: read\n");
            exit(-1);
        }
        line[i] = c;
        i++;
    }
    i--;
    line[i] = '\0';
    char *result = strdup(line); // Create a duplicate of the line
    free(line);
    line = NULL;
    return result;
}

/**
 * creates all the producer with their own queue size and number
 * of products.
 * **/
int createProducersStruct(int fd, producer *prods) {
    // Set the file offset to the beginning of the file
    if (lseek(fd, 0, SEEK_SET) == -1) {
        printf("Error in: lseek\n");
        exit(-1);
    }
    for (int i = 0; i < numberOfProducers; i++) {
        //number of producer
        char* empty;
        empty = readLine(fd);
        //free the allocation of empty
        if(empty != NULL){
            free(empty);
            empty = NULL;
        }
        prods[i].index = i + 1;
        //number of product
        char* numOfProductsString = readLine(fd);
        prods[i].numOfProducts = atoi(numOfProductsString);
        //free the allocation of the numOfProductString
        if(numOfProductsString != NULL){
            free(numOfProductsString);
            numOfProductsString = NULL;
        }
        //queue size
        char* queueSizeString = readLine(fd);
        prods[i].queueSize = atoi(queueSizeString);
        //free the allocation of the queueSizeString
        if(queueSizeString != NULL){
            free(queueSizeString);
            queueSizeString = NULL;
        }

        if (prods[i].queueSize <= 0 && prods[i].numOfProducts > 0) {
            printf("The producer's queue size is 0 or less, set a different size!\n");
            exit(0);
        }
        //empty line
        empty = readLine(fd);
        //free the allocation of empty
        if(empty != NULL){
            free(empty);
            empty = NULL;
        }
    }
    char* coEditorQSizeString = readLine(fd);
    int coEditorQSize = atoi(coEditorQSizeString);
    if(coEditorQSizeString != NULL){
        free(coEditorQSizeString);
        coEditorQSizeString = NULL;
    }
    if (coEditorQSize <= 0) {
        printf("The co-editor queue size is 0 or less, set a different size!\n");
        exit(0);
    }
    return coEditorQSize;
}

/**
 * this is a producer thread function, which being called N times (the number of producers)
 * for each producer it creates a known amount of articles (depends on how many he asked for in the conf file)
 * it creates either SPORTS, NEWS or WEATHER articles and insert them into their own bounded queue
 * each producer has its own queue
 * **/
void *producerThread(void *prod) {
    producer *pI = (producer *) prod;
    //counters for the amount of articles the producer already produced
    int sportsCounter = 0, newsCounter = 0, weatherCounter = 0;
    for (int i = 0; i < pI->numOfProducts; ++i) {
        //get a random article from the 3 possible options
        const char *article = broadCast[rand() % 3];
        char message[200];
        if (strcmp(article, "SPORTS") == 0) {//create sports article
            snprintf(message, 200, "Producer %d %s %d\n", pI->index, article, sportsCounter);
            ++sportsCounter;
        } else if (strcmp(article, "NEWS") == 0) {//create news article
            snprintf(message, 200, "Producer %d %s %d\n", pI->index, article, newsCounter);
            ++newsCounter;
        } else {//create weather article
            snprintf(message, 200, "Producer %d %s %d\n", pI->index, article, weatherCounter);
            ++weatherCounter;
        }
        insertBounded(pI->boundedQ, message);
    }
    return NULL;
}

/**
 * this function is the dispatcher thread function, it is being called once,
 * and it takes in a round-robin algorithm style from each of the producers queues.
 * it then inserts it to one of the 3 unbounded queues of the reports.
 * at the end it sends DONE to each of the 3 queues to acknowledge them
 * that there are no more items to be send
 * **/
void *disPatcherThread(void *prods) {
    producer *pS = prods;
    int i = 0;
    while (numberOfProducers > 0) {
        if (pS[i % originalNumOfProds].boundedQ->isProdLeft) {
            char *message = removeObjBounded(pS[i % originalNumOfProds].boundedQ);

            if (strstr(message, "SPORTS") != NULL) {//check if the substring of the is sports
                insertUnbounded(&sports, message);
            } else if (strstr(message, "WEATHER") != NULL) {//check if the substring of the article is weather
                insertUnbounded(&weather, message);
            } else if (strstr(message, "NEWS") != NULL) {//check if the substring of the article is news
                insertUnbounded(&news, message);
            }

            // Free the memory for the message
            if (message != NULL) {
                free(message);
                message = NULL;
            }
        }
        i++;
    }

    // Insert "DONE" into the unbounded queues
    char* DONE = "DONE";
    insertUnbounded(&sports, DONE);
    insertUnbounded(&weather, DONE);
    insertUnbounded(&news, DONE);

    return NULL;
}

/**
 * the co-edit thread is being called 3 times, by each of the articles.
 * it removes the articles from its own queue and inserts it into the screenManager queue.
 * **/
void *coEditThread(void *unbounded) {
    unBounded *reports = (unBounded *) unbounded;
    char *report = NULL;
    do {
        if (report != NULL) {
            free(report); // Free the memory allocated for the previous report
            report = NULL; // Reset the pointer to avoid potential issues
        }

        report = removeObjUnbounded(reports);
        insertBounded(&coEditor, report);

    } while (strcmp(report, "DONE") != 0);
    if (report != NULL) {
        free(report);
        report = NULL;
    }
    return NULL;
}

/**
 * this function is also a different thread from the main, it is being called once
 * and what it does is take an object from its CO-edit queue and print it.
 * at the end it will print DONE
 * **/
void *screenManagerThread() {
    char *report = NULL;
    do {
        if (report != NULL) {
            free(report); // Free the memory allocated for the previous report
            report = NULL; // Reset the pointer to avoid potential issues
        }
        report = removeCoEditBounded();
        //sleep for 0.1 seconds
        usleep(100000);
        if (strcmp(report, "DONE") == 0) {
        } else {
            fflush(stdout);
            printf("%s", report);
        }

    } while (doneCounter < 3);//waits for all the 3 DONE to be sent to it
    fflush(stdout);
    printf("DONE\n");
    if (report != NULL) {
        free(report); // Free the memory allocated for the previous report
        report = NULL; // Reset the pointer to avoid potential issues
    }
    return NULL;
}

/**
 * this function creates N threads for the producers,
 * 1 thread for the dispatcher
 * 3 threads for the Co-editors
 * and 1 thread for the screenManager.
 * it then waits until all the threads is finished
 * **/
void createThreads(producer *prods) {
    srand(time(NULL)); // Seed the random number generator with current time
    unsigned long tid[originalNumOfProds];
    for (int i = 0; i < originalNumOfProds; i++) {//create N producer threads
        pthread_create(&tid[i], NULL, producerThread, (void *) &prods[i]);
    }

    unsigned long tidDisPatcher;
    pthread_create(&tidDisPatcher, NULL, disPatcherThread, prods);//dispatcher thread
    unsigned long tidNews;
    unsigned long tidSports;
    unsigned long tidWeather;
    //co-Editor threads
    pthread_create(&tidNews, NULL, coEditThread, &news);
    pthread_create(&tidSports, NULL, coEditThread, &sports);
    pthread_create(&tidWeather, NULL, coEditThread, &weather);
    unsigned long tidScreenManager;
    pthread_create(&tidScreenManager, NULL, screenManagerThread, NULL);//screen screenManager thread
    for (int i = 0; i < originalNumOfProds; i++) {//waits for all the producers threads to finish
        pthread_join(tid[i], NULL);
    }
    pthread_join(tidNews, NULL);//wait for news thread to finish
    pthread_join(tidSports, NULL);//wait for sports thread to finish
    pthread_join(tidWeather, NULL);//wait for weather thread to finish
    pthread_join(tidScreenManager, NULL);//wait for screenManager thread to finish
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


int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Did not match number of arguments!\n");
        exit(0);
    }
    //open configuration file
    int fd = openFile(argv[1]);
    //count the number of producer to make an array of producer
    numberOfProducers = countNumOfProd(fd);
    //save the number of producers so we can modify them
    originalNumOfProds = numberOfProducers;
    //created array of producer
    producer prods[originalNumOfProds];
    //create all the q sizes of all producer and return the q size of the CoEditor
    int coEditorQSize = createProducersStruct(fd, prods);
    createBoundedQueues(prods);
    initReport();
    createCoEditBounded(coEditorQSize);
    createThreads(prods);
    clean(prods);
    return 0;
}