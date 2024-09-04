#include "threads_manager.h"

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
