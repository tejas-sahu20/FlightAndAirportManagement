#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdbool.h>
// #include "common.h"

#define READ_END 0
#define WRITE_END 1
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
void getPlaneDetails(int *planeID, int *planeType) {
    printf("Enter Plane ID:\n");
    fflush(stdout);
    scanf("%d", planeID);
    printf("Enter Plane Type:\n");
    fflush(stdout);
    scanf("%d", planeType);
}

int getPassengerLuggageWeight() {
    int weight = 0;
    printf("Enter Weight of Your Luggage:\n");
    fflush(stdout);
    scanf("%d", &weight);
    return weight;
}

int getPassengerBodyWeight() {
    int weight = 0;
    printf("Enter Your Body Weight:\n");
    fflush(stdout);
    scanf("%d", &weight);
    return weight;
}

void getPassengerInfo(struct planeDetails *planeDetailsObject) {
    int numOfSeats = 0;
    int totalPlaneWeight = 525;
    printf("Enter Number of Occupied Seats:\n");
    fflush(stdout);
    scanf("%d", &numOfSeats);
    planeDetailsObject->numOfItems = numOfSeats;

    // Start Here
    for (int i = 0; i < numOfSeats; i++) {
        int fd[2];
        if (pipe(fd) == -1) {
            fprintf(stderr, "PIPE failed");
            return;
        }
        pid_t pid = fork();
        if (pid < 0) {
            perror("Fork failed");
            return;
        } else if (pid == 0) { // Child process
            close(fd[READ_END]);
            int weightLuggage = getPassengerLuggageWeight();
            int bodyWeight = getPassengerBodyWeight();
            write(fd[WRITE_END], &weightLuggage, sizeof(int));
            write(fd[WRITE_END], &bodyWeight, sizeof(int));
            close(fd[WRITE_END]);
            exit(EXIT_SUCCESS);
        }
        wait(NULL);
        close(fd[WRITE_END]);
        int read_msg;
        while (read(fd[READ_END], &read_msg, sizeof(int)) > 0) {
            totalPlaneWeight += read_msg;
        }
        close(fd[READ_END]);
        wait(NULL);
    }

    planeDetailsObject->totalPlaneWeight = totalPlaneWeight;
}

void getCargoInfo(struct planeDetails *planeDetailsObject) {
    int numOfCargoItems=0;
    int averageWeight=0;
    printf("Enter Number of Cargo Items:\n");
    fflush(stdout);
    scanf("%d", &numOfCargoItems);
    planeDetailsObject->numOfItems = numOfCargoItems;

    printf("Enter the Average Weight of Cargo Items\n");
    fflush(stdout);
    scanf("%d", &averageWeight);
    planeDetailsObject->totalPlaneWeight = 150 + (numOfCargoItems * averageWeight);
}

void printSuccessLanding(struct planeDetails *planeDetailsObject){
    printf("Plane %d has successfully traveled from Airport %d to Airport %d!\n", planeDetailsObject->planeId, planeDetailsObject->departure, planeDetailsObject->arrival);
}


void getArrivalAndDeparture(struct planeDetails *planeDetailsObject) {
    printf("Enter Airport Number for Departure:\n");
    fflush(stdout);
    scanf("%d", &planeDetailsObject->departure);
    printf("Enter Airport Number for Arrival:\n");
    fflush(stdout);
    scanf("%d", &planeDetailsObject->arrival);
}

int main() {
    struct planeDetails planeDetailsObject;

    getPlaneDetails(&planeDetailsObject.planeId, &planeDetailsObject.typeOfPlane);
    printf("%d",planeDetailsObject.planeId);
    if (planeDetailsObject.typeOfPlane == 1) {
        getPassengerInfo(&planeDetailsObject);
    } else {
        getCargoInfo(&planeDetailsObject);
    }

    getArrivalAndDeparture(&planeDetailsObject);
    printf("It is here");

    // Sending message to air traffic controller with details in message queue
    key_t key;
    int msgid;
    key = ftok("airTrafficController.c", 'A'); // Change 'A' to any unique character

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

    message.mesg_type = 22; // Assuming message type 22 is meaningful in your application
    message.planeDetailsObject = planeDetailsObject;
    message.planeDetailsObject.arrived = false;
    message.planeDetailsObject.reqFromPlane = true;
    // Sending message using msgsnd
    sleep(10);
    printf("Here9 %d\n",message.planeDetailsObject.arrived);
            fflush(stdout);
    if (msgsnd(msgid, &message, sizeof(message), 0) == -1) {
        perror("msgsnd");
        exit(EXIT_FAILURE);
    }
    msgrcv(msgid, &message, sizeof(message), planeDetailsObject.planeId, 0);
    message.planeDetailsObject.arrived = true;
    message.planeDetailsObject.reqFromPlane = true;
    message.mesg_type=22;
    printf("Here9 %d\n",message.planeDetailsObject.arrived);
            fflush(stdout);
            sleep(30);
    if (msgsnd(msgid, &message, sizeof(message), 0) == -1) {
        perror("msgsnd");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Second message sent\n");
        fflush(stdout);
    }
    // Landing request wait
    
    msgrcv(msgid, &message, sizeof(message), planeDetailsObject.planeId, 0);
    // Print Successfully traveled
    printSuccessLanding(&planeDetailsObject);
    printf("Total plane weight: %d\n", planeDetailsObject.totalPlaneWeight);
    printf("Number of items: %d\n", planeDetailsObject.numOfItems);

    return 0;
}
