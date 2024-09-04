#include "broadcaster.h"

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