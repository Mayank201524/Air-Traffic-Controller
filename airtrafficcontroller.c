#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <errno.h>
//#include <time.h>


#define FILENAME "AirTrafficController.txt"
//#define FILENAME_MAX_LENGTH 100


typedef struct {
	long mtype;
    int plane_id;
    int plane_weight;
    int type; // 0 for cargo, 1 for passenger
    int passengers;
    int departure_airport;
    int arrival_airport;
    int dora;
} Plane;

typedef struct {
    long mtype;
    int m;
} Message;


void handle_departure_request(int msgid, Plane *plane);
void handle_takeoff_complete(int msgid, Plane *plane);
void handle_landing_success(int msgid, Plane *plane);
void handle_cleanup_request(int msgid, int num_airports);

int main() {
    int num_airports;
    int s=0;
    int q=0;
    int r=0;
    //system("touch airtraffic.txt");
    key_t key;
    
    // Generate a key for the message queue
    if ((key = ftok("airtrafficcontroller.c", 'A')) == -1) {
        perror("ftok");
        exit(EXIT_FAILURE);
    }
    
    // Create a message queue
    int msgid = msgget(key, IPC_CREAT | 0644);
    if (msgid == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }
    
    printf("Enter the number of airports to be handled/managed: ");
    scanf("%d", &num_airports);
    
    

    // File pointer for appending plane journey information
    FILE *file = fopen(FILENAME, "a");
    if (file == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    Message msg;
    msg.m=0;
    // Main loop for handling messages
    while (1) {
        
        Plane plane;
        plane.mtype=0;
        int j;
 
        // Receive a message from clean-up
        
        if (msgrcv(msgid, (void *)&msg, sizeof(Message), 27, IPC_NOWAIT) == -1) {
            if (errno != ENOMSG) {
            perror("msgrcv");
            exit(EXIT_FAILURE);
            }
        }
        //else {
        	//i = 1;
        	//}
        	
        
               
        for( j=21; j<=25; j+=2)
        {
        if (msgrcv(msgid, (void *)&plane, sizeof(Plane), j, IPC_NOWAIT) == -1) {
        	if (errno != ENOMSG) {
            perror("msgrcv");
            exit(EXIT_FAILURE);
            }
        }
        else {
        break;}
        }
        
        
	
        // Determine message type and call corresponding handler function
        switch (plane.mtype) {
            
            case 21:
            	if(msg.m !=1)
            	{
            	handle_departure_request(msgid, &plane);
            	s+=1;
            	}//printf("Hello");
        		break;
            case 23: // Take-off complete message type
                handle_takeoff_complete(msgid, &plane);
                q+=1;
                break;
            case 25: // Landing success message type
                handle_landing_success(msgid, &plane);
                r+=1;
                break;
            default:
                break;
        }
        
        if(s == q && s == r)
        {
        	if(msg.m == 1)
        	{
        	handle_cleanup_request(msgid, num_airports);
        	break;
        	}
        }
        
    } 

    // Close file pointer
    fclose(file);
	//system("rm airtraffic.txt");
    return 0;
}

void handle_departure_request(int msgid, Plane *plane) {
    // Extract plane details from the message
    Plane message;
    
    message.mtype = (long)(plane->departure_airport);
    message.plane_id = plane->plane_id;
    message.plane_weight = plane->plane_weight;
    message.type = plane->type;
    message.passengers = plane->passengers;
    message.departure_airport = plane->departure_airport;
    message.arrival_airport = plane->arrival_airport;
    message.dora = 1;

    // Inform departure airport to begin boarding/loading and takeoff process
    
    if (msgsnd(msgid, (void *)&message, sizeof(Plane), 0) == -1) {
        perror("msgsnd");
        exit(EXIT_FAILURE);
    }

    
    // Append plane journey information to the file
    FILE *file = fopen(FILENAME, "a");
    if (file == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    fprintf(file, "Plane %d has departed from Airport %d and will land at Airport %d.\n", message.plane_id, message.departure_airport, message.arrival_airport);
    fclose(file);
}

void handle_takeoff_complete(int msgid, Plane *plane)
{
	Plane message;
    
    message.mtype = (long)plane->arrival_airport;
    message.plane_id = plane->plane_id;
    message.plane_weight = plane->plane_weight;
    message.type = plane->type;
    message.passengers = plane->passengers;
    message.departure_airport = plane->departure_airport;
    message.arrival_airport = plane->arrival_airport;
    message.dora = 2;
    
    if (msgsnd(msgid, &message, sizeof(Plane), 0) == -1) {
        perror("msgsnd");
        exit(EXIT_FAILURE);
    }
}

void handle_landing_success(int msgid, Plane *plane) {
    Message msg;
    
    msg.mtype = (long)(plane->plane_id + 10);
    msg.m = 1;
    if (msgsnd(msgid, (void *)&msg, sizeof(Message), 0) == -1) {
        perror("msgsnd");
        exit(EXIT_FAILURE);
    }
}

void handle_cleanup_request(int msgid, int num_airports) {
    
    Message msg;
    msg.mtype = 28; // Termination message type
    msg.m = 1;

    // Send termination message to all airports
    for (int i = 1; i <= num_airports; i++) {
        if (msgsnd(msgid, (void *)&msg, sizeof(Message), 0) == -1) {
            perror("msgsnd");
            exit(EXIT_FAILURE);
        }
    }

    // Wait for confirmation messages from airports
    int confirmation_count = 0;
    while (confirmation_count < num_airports) {
        Message msg1;
        msg1.m = 0;
        if (msgrcv(msgid, (void *)&msg1, sizeof(Message), 29, IPC_NOWAIT) == -1) {
        	if (errno != ENOMSG) {
            perror("msgrcv");
            exit(EXIT_FAILURE);
        }
        }
        // Check if confirmation message is received from an airport
        if (msg1.m == 1) {
            confirmation_count++;
        }
    }

    // Perform cleanup activities
    //printf("All airports have terminated. Performing cleanup...\n");

    // Remove the message queue
    if (msgctl(msgid, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        exit(EXIT_FAILURE);
    }

    // Terminate the air traffic controller process
    //printf("Air traffic controller process terminated.\n");
    exit(EXIT_SUCCESS);
}

