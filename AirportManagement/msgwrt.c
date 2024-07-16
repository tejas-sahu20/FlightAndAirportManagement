// C Program for Message Queue (Writer Process) 
#include <stdio.h> 
#include <sys/ipc.h> 
#include <sys/msg.h> 
#define MAX 10 
  

struct planeDetails {
    int totalPlaneWeight;
    int arrival;
    int departure;
    int planeId;
    int typeOfPlane;
    int numOfItems;
};

// structure for message queue
struct mesg_buffer { 
    long mesg_type; 
    struct planeDetails; 
} message; 
  
int main() 
{ 
    key_t key; 
    int msgid; 
  
    // ftok to generate unique key 
    key = ftok("progfile", 65); 
  
    // msgget creates a message queue 
    // and returns identifier 
    msgid = msgget(key, 0666 | IPC_CREAT); 
    message.mesg_type = 1; 
}