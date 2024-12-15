#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>


typedef struct {
    long mtype;
    int m;
} Message;

int main() {
    key_t key;
    int msgid;
    Message msg;

    // Generate a key
    if ((key = ftok("airtrafficcontroller.c", 'A')) == -1) {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    // Join a message queue for cleanup process
    if ((msgid = msgget(key, 0644)) == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    // Main loop for cleanup process
    while (1) {
        
        char choice;
        printf("Do you want the Air Traffic Control System to terminate? (Y for Yes and N for No): ");
        scanf(" %c", &choice);

        // Send the choice to terminate or not to the Air Traffic Controller
        msg.mtype = 27;
        
        if(choice == 'Y')
        {
        	msg.m = 1;
        }
        else{ 
        msg.m=0;}
        
        if (msgsnd(msgid, (void *)&msg, sizeof(Message), 0) == -1) {
            perror("msgsnd");
            exit(EXIT_FAILURE);
        }

        // If user chooses to terminate, cleanup process terminates
        if (choice == 'Y') {
            //printf("Termination signal sent to Air Traffic Controller. Cleanup process terminated.\n");
            break;
        }
    }

    return 0;
}

