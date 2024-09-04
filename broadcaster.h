#ifndef BROADCASTER_H
#define BROADCASTER_H

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>

#include "bounded.h"
#include "unbounded.h"
#include "threads_manager.h"

// Global variables
extern const char *broadCast[3];
extern int numberOfProducers;
extern int doneCounter;
extern int originalNumOfProds;
extern unBounded sports;
extern unBounded news;
extern unBounded weather;
extern bounded coEditor;

// File-related functions
int openFile(char *configuration);
int countNumOfProd(int fd);
char *readLine(int fd);
int createProducersStruct(int fd, producer *prods);

#endif
