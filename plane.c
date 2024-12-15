#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>


#define MAX_LUGGAGE_WEIGHT 25
#define MAX_BODY_WEIGHT 100
#define PASSENEGER_CREW_MEMBERS 7
#define CARGO_CREW_MEMBERS 2

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
	int terminate;
} Message;


int** create_passenger_processes(int passengers, int plane_id) {
    

    int i, fd[2];
    pid_t pid;

	int **weight = (int**)malloc(passengers * sizeof(int*));

	if (weight == NULL){
		printf("Memory allocation failed\n");
		exit(1);
	}

	for(int j=0; j< passengers; ++j)
	{
		weight[j] = (int*)malloc(2 * sizeof(int));
		if(weight[j] == NULL)
		{
			printf("Memory allocation failed.\n");
			exit(EXIT_FAILURE);
		}
	}
		
    for (i = 0; i < passengers; i++) {
        if (pipe(fd) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
        pid = fork();

        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) {
        	int l_weight, b_weight;

        	printf("Enter Weight of Your Luggage: ");
        	scanf("%d", &l_weight);
        	printf("Enter Your Body Weight: ");
        	scanf("%d", &b_weight);
        	
            close(fd[0]); // close read end
            write(fd[1], &l_weight, sizeof(int));
            write(fd[1], &b_weight, sizeof(int));
            close(fd[1]);

            //printf("Passenger on Plane %d: Luggage Weight: %d kgs, Body Weight: %d kgs\n", plane_id, l_weight, b_weight);
            exit(0);
        } else {
			wait(NULL);
            close(fd[1]); // close write end
            read(fd[0], weight[i], sizeof(int));
            for(int j=0; j<2; j++)
            {
            	read(fd[0], &weight[i][j+1], sizeof(int));
        	}

		}
	}
        return weight;
    
}

void passenger_plane_process(Plane *plane) {
    
    int i, num_passengers;
    

    //printf("Plane ID: %d\n", plane->plane_id);
    printf("Enter Number of Occupied Seats: ");
    scanf("%d", &num_passengers);

    if (num_passengers < 1 || num_passengers > 10) {
        printf("Invalid number of passengers.\n");
        exit(EXIT_FAILURE);
    }

    plane->passengers = num_passengers;

    

    int **weights = create_passenger_processes(num_passengers, plane->plane_id);
	int total_weight=0;
    for (i = 0; i < num_passengers; i++) {
    	for(int j=0; j<2; j++)
            {
            	//printf("%d ", weights[i][j]);
            	total_weight+= weights[i][j];
        	}
    }
    
    plane->plane_weight = total_weight + (PASSENEGER_CREW_MEMBERS * 75);
    printf("\n");
    
    //printf("Plane Weight: %d \n", plane->plane_weight);
    
    printf("Enter Airport Number for Departure: ");
    scanf("%d", &plane->departure_airport);

    printf("Enter Airport Number for Arrival: ");
    scanf("%d", &plane->arrival_airport);

    //printf("Plane %d ready for departure.\n", plane.plane_id);
    //sleep(30); // Journey duration
    //printf("Plane %d arrived at Airport %d.\n", plane.plane_id, plane.arrival_airport);
    //printf("Deboarding/unloading passengers...\n");
    //sleep(3); // Unloading process
    //printf("Plane %d has successfully traveled from Airport %d to Airport %d!\n", plane.plane_id, plane.departure_airport, plane.arrival_airport);
    
    for (int i = 0; i < num_passengers; i++) {
        free(weights[i]);
    }
    free(weights);
}

void cargo_plane_process(Plane *plane) {
    int num_cargo_items, avg_cargo_weight;

    printf("Plane ID: %d\n", plane->plane_id);
    printf("Enter Number of Cargo Items: ");
    scanf("%d", &num_cargo_items);

    if (num_cargo_items < 1 || num_cargo_items > 100) {
        printf("Invalid number of cargo items.\n");
        exit(EXIT_FAILURE);
    }

    printf("Enter Average Weight of Cargo Items: ");
    scanf("%d", &avg_cargo_weight);
    
    if (avg_cargo_weight < 1 || avg_cargo_weight > 100) {
        printf("Invalid average weight of cargo items.\n");
        exit(EXIT_FAILURE);
    }

    printf("Enter Airport Number for Departure: ");
    scanf("%d", &plane->departure_airport);

    printf("Enter Airport Number for Arrival: ");
    scanf("%d", &plane->arrival_airport);

    plane->plane_weight = (num_cargo_items * avg_cargo_weight) + (CARGO_CREW_MEMBERS * 75);
	
	//printf("Plane Weight: %d \n", plane->plane_weight);
	
    //printf("Plane %d ready for departure.\n", plane.plane_id);
    //sleep(30); // Journey duration
    //printf("Plane %d arrived at Airport %d.\n", plane.plane_id, plane.arrival_airport);
    //printf("Unloading cargo...\n");
    //sleep(3); // Unloading process
    //printf("Plane %d has successfully traveled from Airport %d to Airport %d!\n", plane.plane_id, plane.departure_airport, plane.arrival_airport);
}

int send_to_atc(Plane plane) {
    key_t key;
    int msgid;
    Plane message;
	
    // Generate a key for the message queue
    if ((key = ftok("airtrafficcontroller.c", 'A')) == -1) {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    // Join a message queue
    if ((msgid = msgget(key, 0644)) == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    // Message Type
    message.mtype = 21;
    message.plane_id = plane.plane_id;
    message.plane_weight = plane.plane_weight;
    message.type = plane.type;
    message.passengers = plane.passengers;
    message.departure_airport = plane.departure_airport;
    message.arrival_airport = plane.arrival_airport;
    message.dora = 0;

    // Send the message
    if (msgsnd(msgid, (void *)&message, sizeof(Plane), 0) == -1) {
        perror("msgsnd");
        exit(EXIT_FAILURE);
    }

    //printf("Plane data sent to ATC.\n");
    
    return msgid;
}

int main() {
    Plane plane;
    int choice;

    printf("Enter Plane ID: ");
    scanf("%d", &plane.plane_id);

    if (plane.plane_id < 1 || plane.plane_id > 10) {
        printf("Invalid plane ID.\n");
        exit(EXIT_FAILURE);
    }

    printf("Enter Type of Plane:"); // (1 for passenger, 0 for cargo)
    scanf("%d", &choice);

    if (choice != 0 && choice != 1) {
        printf("Invalid choice.\n");
        exit(EXIT_FAILURE);
    }

    plane.type = choice;
    plane.passengers = 0;

    if (plane.type == 1) {
        passenger_plane_process(&plane);
    } else {
        cargo_plane_process(&plane);
    }
    
    int msgid;
    
    // Send the plane details to ATC
    msgid = send_to_atc(plane);
    
    sleep(3); // Boarding/Loading
    //sleep(2); // Take-off
    sleep(30); // Journey
    //sleep(2); // Landing
    sleep(3); // Deboarding/Unloading
    
    long type;
    type= (long)(plane.plane_id + 10);
    Message msg;
    do {
        msg.terminate=0;
        // Receive a message from ATC
        
        if (msgrcv(msgid, (void *)&msg, sizeof(Message), type, IPC_NOWAIT) == -1) {
        	if (errno != ENOMSG) {
            perror("msgrcv");
            exit(EXIT_FAILURE);
        }
        }
        if(msg.terminate == 1) // 1 if deboarding is complete ATC
        {
			printf("Plane %d has successfully traveled from Airport %d to Airport %d!\n", plane.plane_id, plane.departure_airport, plane.arrival_airport);
		}
		} while(msg.terminate != 1);
		
		
    return 0;
}

