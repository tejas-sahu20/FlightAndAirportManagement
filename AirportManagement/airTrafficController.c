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
int getNumberOfAirports() {
    int a=0;
    printf("Enter the number of airports to be handled/managed:");
    scanf(" %d", &a); // Notice the space before %d to clear whitespace
    while (getchar() != '\n'); // Clear input buffer
    // printf("go\n");
    return a;
}

int main() {
    int numberOfAirports = 0;
    numberOfAirports = getNumberOfAirports();
    fflush(stdout); 
    
    key_t key;
    int msgid;

    key = ftok("airTrafficController.c", 'A');
    if (key == -1){
        printf("error in creating unique key\n");
        exit(1);
    }
    msgid = msgget(key, 0644|IPC_CREAT);   
    if (msgid == -1){
        // printf("error in creating message queue\n");
        exit(1);
    }
    else
    {
        printf("go,\t%d\n",key);
    }
    if (msgctl(msgid, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        exit(EXIT_FAILURE);
    }
    msgid = msgget(key, 0644|IPC_CREAT);  
    fflush(stdout);
    while (true) {
        // struct mesg_buffer message;

    fflush(stdout);
        if (msgrcv(msgid, &message, sizeof(message), 22, 0) == -1) {
            perror("msgrcv");
            exit(EXIT_FAILURE);
        }
                printf("%d\n",message.planeDetailsObject.planeId);
                fflush(stdout);
            // printf("%d",message.pla);

            // fflush(stdout);

        if (message.planeDetailsObject.reqFromPlane == true) {
            // If message was from a plane
            // printf("Here9 %d\n",message.planeDetailsObject.arrived);
            fflush(stdout);
            if (message.planeDetailsObject.arrived == false) {
                message.mesg_type = message.planeDetailsObject.departure + 10;
            } else {
                printf("Here7");
                fflush(stdout);
                message.mesg_type = message.planeDetailsObject.arrival + 10;    
            }
        } else {
            printf("Here8");
            fflush(stdout);
            // If message was not from a plane
            message.mesg_type = message.planeDetailsObject.planeId;
        }
        // Send message
            printf("message type =%ld\n",message.mesg_type);
            fflush(stdout);
            // sleep(3);
        if (msgsnd(msgid, &message, sizeof(message), 0) == -1) {
            perror("msgsnd");
            exit(EXIT_FAILURE);
        }
    }
    fflush(stdout);
    return 0;
}
