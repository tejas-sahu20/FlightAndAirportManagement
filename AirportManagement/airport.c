#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/ipc.h>
#include <sys/msg.h>
// #include "common.h"
struct planeDetails {
    int totalPlaneWeight;
    int arrival;
    int departure;
    int planeId;
    int typeOfPlane;
    int numOfItems;
    bool arrived;
    bool takeoffRequest;
    bool reqFromPlane;
};
struct mesg_buffer {
    long mesg_type;
    struct planeDetails planeDetailsObject;
} message;

#define MAX_RUNWAYS 10
#define MAX_AIRPORTS 10

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


typedef struct {
    struct planeDetails planeDetailsObject;
    int *weights;
    bool *runwayAvailable;
    int numberOfRunways;
    int msgid;
} ThreadArgs;

void *threadFunctionDeparture(void *args) {
    printf("waiting for departure\n");
    fflush(stdout);
    ThreadArgs threadArgs = *((ThreadArgs *)args);
    
            printf("third %d\n",threadArgs.planeDetailsObject.planeId);
            fflush(stdout);
    bool found = false;
    int ind = -1;
    // printf("waiting for departure\n");
    // fflush(stdout);
    while (!found) {
        pthread_mutex_lock(&mutex);
        for (int i = 0; i < threadArgs.numberOfRunways; i++) {
            if (threadArgs.runwayAvailable[i] && threadArgs.weights[i] >= threadArgs.planeDetailsObject.totalPlaneWeight) {
                if (ind == -1 || threadArgs.weights[ind] >= threadArgs.weights[i]) {
                    ind = i;
                }
            }
        }
        if (ind != -1) {
            threadArgs.runwayAvailable[ind] = false;
            found = true;
        }
        pthread_mutex_unlock(&mutex);
    }

    printf("recived runway\n");
    fflush(stdout);
    sleep(30); // Boarding/loading process
    threadArgs.runwayAvailable[ind] = true;
    printf("Plane %d has completed boarding/loading and taken off from Runway No. %d of Airport No. %d.\n",
           threadArgs.planeDetailsObject.planeId, ind, threadArgs.planeDetailsObject.departure);
    fflush(stdout);
    // Send message to air traffic controller
    message.planeDetailsObject = threadArgs.planeDetailsObject;
    message.mesg_type = 22;
    
    message.planeDetailsObject.reqFromPlane = false;
    
            printf("fourth %d and %d\n",message.planeDetailsObject.planeId,threadArgs.planeDetailsObject.planeId);
            fflush(stdout);
    if (msgsnd(threadArgs.msgid, &message, sizeof(message), 0) == -1) {
        perror("msgsnd");
        exit(EXIT_FAILURE);
    }
    pthread_exit(NULL);
}

void *threadFunctionArrival(void *args) {
    ThreadArgs *threadArgs = (ThreadArgs *)args;
    bool found = false;
    int ind = -1;
    while (!found) {
        pthread_mutex_lock(&mutex);
        for (int i = 1; i < threadArgs->numberOfRunways; i++) {
            if (threadArgs->runwayAvailable[i] && threadArgs->weights[i] >= threadArgs->planeDetailsObject.totalPlaneWeight) {
                if (ind == -1 || threadArgs->weights[ind] >= threadArgs->weights[i]) {
                    ind = i;
                }
            }
        }
        if (ind == -1 && threadArgs->weights[0] >= threadArgs->planeDetailsObject.totalPlaneWeight && threadArgs->runwayAvailable[0]) {
            ind = 0;
        }
        if (ind != -1) {
            threadArgs->runwayAvailable[ind] = false;
            found = true;
        }
        pthread_mutex_unlock(&mutex);
    }
    sleep(20); // Boarding/loading process
    printf("Woke up\n");
    fflush(stdout);
    pthread_mutex_lock(&mutex);
    threadArgs->runwayAvailable[ind] = true;
    printf("Plane %d has landed on Runway No. %d of Airport No. %d and has completed deboarding/unloading.\n",
           threadArgs->planeDetailsObject.planeId, ind, threadArgs->planeDetailsObject.arrival);
    fflush(stdout);
    pthread_mutex_unlock(&mutex);
    // Send message to air traffic controller
    message.planeDetailsObject = threadArgs->planeDetailsObject;
    message.mesg_type = 22;
    
    message.planeDetailsObject.reqFromPlane = false;
    if (msgsnd(threadArgs->msgid, &message, sizeof(message), 0) == -1) {
        perror("msgsnd");
        exit(EXIT_FAILURE);
    }
    pthread_exit(NULL);
}

void getAirportDetails(int *airNumber, int *noOfRunways) {
    printf("Enter Airport Number:\n");
    fflush(stdout);
    scanf("%d", airNumber);
    
        fflush(stdout);
    printf("Enter number of Runways:\n");
    fflush(stdout);
    scanf("%d", noOfRunways);
    
        fflush(stdout);
}

void getLoadCapacities(int loadCapacities[], int numberOfRunways) {
    printf("Enter loadCapacity of Runways (give as a space separated list in a single line):\n");
    fflush(stdout);
    for (int i = 1; i < numberOfRunways; i++) {
        scanf("%d", &loadCapacities[i]);
        fflush(stdout);
    }
    loadCapacities[0]=0;
    // printf("foasfdo");
        fflush(stdout);
}

int main() {
    int airportNumber;
    int numberOfRunways;
    getAirportDetails(&airportNumber, &numberOfRunways);
    numberOfRunways++; // Considering one runway is reserved for backup

    int loadCapacities[MAX_RUNWAYS];
    bool runwayAvailable[MAX_RUNWAYS];

    getLoadCapacities(loadCapacities, numberOfRunways);
    
    printf("JI\n");
        fflush(stdout);

    // Initialize runway availability
    for (int i = 0; i < numberOfRunways; i++) {
        runwayAvailable[i] = true;
    }

    // Initialize message queue
    key_t key;
    int msgid;

    // ftok to generate unique key
    key = ftok("airTrafficController.c", 'A');

    // Check if ftok succeeded
    if (key == -1) {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    // msgget creates a message queue and returns identifier
    msgid = msgget(key, 0666 | IPC_CREAT);

    // Check if msgget succeeded
    if (msgid == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    // Main loop to handle messages
    while (true) {
        
            printf("Here4");
            fflush(stdout);
        if (msgrcv(msgid, &message, sizeof(message), airportNumber+10, 0) == -1) {
            perror("msgrcv");
            exit(EXIT_FAILURE);
        }
        
            printf("first %d\n",message.planeDetailsObject.planeId);
            fflush(stdout);

        // Create thread arguments
        ThreadArgs threadArgs;
        threadArgs.planeDetailsObject = message.planeDetailsObject;
        threadArgs.weights = loadCapacities;
        threadArgs.runwayAvailable = runwayAvailable;
        threadArgs.numberOfRunways = numberOfRunways;
        threadArgs.msgid = msgid;
        
            printf("second %d\n",message.planeDetailsObject.planeId);
            fflush(stdout);
        // Create thread based on departure or arrival
        pthread_t thread;
        // sleep(10);
        if (threadArgs.planeDetailsObject.departure == airportNumber) {
            int threadResult = pthread_create(&thread, NULL, threadFunctionDeparture, (void *)&threadArgs);
            if (threadResult != 0) {
                fprintf(stderr, "Error creating thread: %d\n", threadResult);
                exit(EXIT_FAILURE);
            }
        } else {
            int threadResult = pthread_create(&thread, NULL, threadFunctionArrival, (void *)&threadArgs);
            if (threadResult != 0) {
                fprintf(stderr, "Error creating thread: %d\n", threadResult);
                exit(EXIT_FAILURE);
            }
        }
        fflush(stdout);
    }

    return 0;
}
