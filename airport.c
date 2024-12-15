#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <errno.h>

#define MAX_RUNWAYS 10
#define MAX_RUNWAY_LOADCAPACITY 12000
#define BACKUP_LOADCAPACITY 15000

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

typedef struct {
    int number;
    int load_capacity;
    pthread_mutex_t mutex;
} Runway;

Runway runways[MAX_RUNWAYS + 1]; // Including backup runway

int num_runways;
int msgid;

void *handle_departure(void *arg) {
    Plane *plane = (Plane *)arg;
    int selected_runway = -1;
    
    // Best-fit selection of runway
    for (int i = 0; i < num_runways; ++i) {
        pthread_mutex_lock(&runways[i].mutex);
        if (plane->plane_weight <= runways[i].load_capacity) {
            if (selected_runway == -1 || runways[i].load_capacity < runways[selected_runway].load_capacity) {
                selected_runway = i;
            }
        }
        pthread_mutex_unlock(&runways[i].mutex);
    }

    // If no suitable runway found, use backup runway
    if (selected_runway == -1) {
        selected_runway = num_runways;
    }
	sleep(3); //Boarding
    pthread_mutex_lock(&runways[selected_runway].mutex);
    // Takeoff
    sleep(2);
    printf("Plane %d has completed boarding/loading and taken off from Runway No. %d of Airport No. %d.\n", plane->plane_id, runways[selected_runway].number, plane->departure_airport);
    pthread_mutex_unlock(&runways[selected_runway].mutex);

    

    // Inform air traffic controller about successful takeoff
    
    plane->mtype = 23;
    if (msgsnd(msgid, (void *)plane, sizeof(Plane), 0) == -1) {
        perror("msgsnd");
        exit(EXIT_FAILURE);
    }
    
    free(plane);
    return NULL;
}

void *handle_arrival(void *arg) {
    Plane *plane = (Plane *)arg;
    int selected_runway = -1;

    // Best-fit selection of runway
    for (int i = 0; i < num_runways; ++i) {
        pthread_mutex_lock(&runways[i].mutex);
        if (plane->plane_weight <= runways[i].load_capacity) {
            if (selected_runway == -1 || runways[i].load_capacity < runways[selected_runway].load_capacity) {
                selected_runway = i;
            }
        }
        pthread_mutex_unlock(&runways[i].mutex);
    }

    // If no suitable runway found, use backup runway
    if (selected_runway == -1) {
        selected_runway = num_runways;
    }
	sleep(30); //Journey
	
    pthread_mutex_lock(&runways[selected_runway].mutex);
    sleep(2); //Landing
    printf("Plane %d has landed on Runway No. %d of Airport No. %d and has completed deboarding/unloading.\n", plane->plane_id, runways[selected_runway].number, plane->arrival_airport);
    pthread_mutex_unlock(&runways[selected_runway].mutex);

    // Deboarding/unloading
    sleep(3);

    // Inform air traffic controller about successful landing and deboarding
    plane->mtype = 25;
    if (msgsnd(msgid, (void *)plane, sizeof(Plane), 0) == -1) {
        perror("msgsnd");
        exit(EXIT_FAILURE);
    }
    free(plane);
    return NULL;
}

pthread_t handle_message(Plane *plane) {
    pthread_t tid;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    
    // Allocate memory for the plane
    Plane *plane_copy = (Plane *)malloc(sizeof(Plane));
    if (plane_copy == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    memcpy(plane_copy, plane, sizeof(Plane));
    
    //printf("Depart");
    if (plane->dora == 1) { // Departure
    	//printf("Depart");
        pthread_create(&tid, &attr, handle_departure, (void *)plane_copy);
    } else if (plane->dora == 2){ // Arrival
        pthread_create(&tid, &attr, handle_arrival, (void *)plane_copy);
    }
    return tid;
	//pthread_attr_destroy(&attr);
    //pthread_detach(tid);
}

int main() {
    int airport_number, backup_runway_number;
    
    printf("Enter Airport Number: ");
    scanf("%d", &airport_number);
    if (airport_number < 1 || airport_number > 10) {
        printf("Invalid airport number.\n");
        exit(EXIT_FAILURE);
    }
	
	
    printf("Enter number of Runways: ");
    scanf("%d", &num_runways);
    if (num_runways < 1 || num_runways > MAX_RUNWAYS || num_runways % 2 != 0) {
        printf("Invalid number of runways. It should be an even number between 1 and 10.\n");
        exit(EXIT_FAILURE);
    }
    backup_runway_number = num_runways + 1;
    
    key_t key;
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
    

	
  // Initialize runways
    printf("Enter loadCapacity of Runways (give as a space separated list in a single line):\n");
    char input_buffer[1024]; // Assuming input won't exceed 1024 characters
    fgets(input_buffer, sizeof(input_buffer), stdin); // Clear stdin
    fgets(input_buffer, sizeof(input_buffer), stdin);
    char *token = strtok(input_buffer, " ");
    for (int i = 0; i < num_runways; ++i) {
        if (token == NULL) {
            printf("Not enough load capacities provided.\n");
            exit(EXIT_FAILURE);
        }
        runways[i].load_capacity = atoi(token);
        if (runways[i].load_capacity < 1000 || runways[i].load_capacity > 12000) {
            printf("Invalid load capacity. It should be an integer between 1000 and 12000.\n");
            exit(EXIT_FAILURE);
        }
        runways[i].number = i + 1;
        if (pthread_mutex_init(&runways[i].mutex, NULL) != 0) {
            perror("pthread_mutex_init");
            exit(EXIT_FAILURE);
        }
        token = strtok(NULL, " ");
    }

    // Initialize backup runway
    runways[backup_runway_number - 1].number = backup_runway_number;
    runways[backup_runway_number - 1].load_capacity = BACKUP_LOADCAPACITY;
    if (pthread_mutex_init(&runways[backup_runway_number - 1].mutex, NULL) != 0) {
        perror("pthread_mutex_init");
        exit(EXIT_FAILURE);
    }
    //fflush(stdout);
	//printf("Hello");
	
    	long a;
	a= (long)airport_number;
	Message msg;
	msg.m = 0;
	pthread_t tid;
	do{
		//printf("Anmol");
		
        if (msgrcv(msgid, (void *)&msg, sizeof(Message), 28, IPC_NOWAIT) == -1) {
        	if (errno != ENOMSG) {
            perror("msgrcv");
            exit(EXIT_FAILURE);
        }
        }
    
    
        Plane plane;
        if (msgrcv(msgid, (void *)&plane, sizeof(Plane), a, IPC_NOWAIT) == -1) {
        	if (errno != ENOMSG) {
            perror("msgrcv");
            exit(EXIT_FAILURE);
        }
        
        }
        else {
        tid = handle_message(&plane);}
    
} while(msg.m !=1);


	pthread_join(tid,NULL);
	
	// Termination Confirmation
	Message msg1;
	msg1.mtype = 29;
	msg1.m = 1;
    if (msgsnd(msgid, (void *)&msg1, sizeof(Message), 0) == -1) {
        perror("msgsnd");
        exit(EXIT_FAILURE);
    }
	
    // Clean up
    for (int i = 0; i <= backup_runway_number; ++i) {
        pthread_mutex_destroy(&runways[i].mutex);
    }

    return 0;
    
}

