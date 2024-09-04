#ifndef THREADS_MANAGER_H
#define THREADS_MANAGER_H

#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include "bounded.h"
#include "unbounded.h"

// Function declarations for thread management
void *producerThread(void *prod);
void *disPatcherThread(void *prods);
void *coEditThread(void *unbounded);
void *screenManagerThread();
void createThreads(producer *prods);


#endif
