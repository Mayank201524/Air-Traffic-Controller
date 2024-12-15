# Air Traffic Control System

This project simulates an Air Traffic Control (ATC) system that manages the coordination of planes, airports, and air traffic controllers. It supports both passenger and cargo planes, handles departures and arrivals, and allows for controlled termination of the system.

## Features

- **Plane Management**: Handles passenger and cargo planes with varying weights and capacities.
- **Runway Assignment**: Uses a best-fit approach for assigning runways to planes based on weight.
- **Communication**: Facilitates messaging between planes, airports, and the air traffic controller using a message queue.
- **System Termination**: Provides controlled cleanup and shutdown of the system.

## Components

The project consists of four main modules:

1. **Airport Module (`airport.c`)**:
   - Manages runways and assigns them to departing or arriving planes.
   - Notifies the ATC about takeoff and landing events.

2. **Air Traffic Controller Module (`airtrafficcontroller.c`)**:
   - Coordinates communication between planes and airports.
   - Logs events such as departures, arrivals, and successful landings.
   - Handles termination requests.

3. **Plane Module (`plane.c`)**:
   - Simulates planes, including passenger and cargo planes.
   - Collects passenger and cargo weights.
   - Sends messages to the ATC for departure and arrival.

4. **Cleanup Module (`cleanup.c`)**:
   - Provides an interface for controlled shutdown of the system.
   - Ensures all modules are terminated gracefully.

## Prerequisites

- GCC Compiler
- POSIX-compatible system (Linux or macOS)
- Basic knowledge of C programming

## How to Run

1. Compile the modules:
   ```bash
   gcc -o airport airport.c -lpthread
   gcc -o airtrafficcontroller airtrafficcontroller.c
   gcc -o plane plane.c
   gcc -o cleanup cleanup.c
    ```

2. Run the air traffic controller:
    ```bash
    ./airtrafficcontroller
    ```

3. Run the airport module for each airport:
    ```bash
    ./airport
    ```

4. Run the plane module for each plane:
    ```bash
    ./plane
    ```

5. Run the cleanup module to terminate the system:
    ```bash
    ./cleanup
    ```

# Project Workflow
##1. Initialization:

Start the ATC and airports.
Input the number of runways and their load capacities for each airport.
Plane Simulation:

##2. Simulate passenger or cargo planes by providing weights and airport details.
Planes request permission for takeoff via the ATC.
Flight Operations:

##3. Planes take off and notify the ATC.
After a simulated journey, planes land and notify the ATC.

##4.Cleanup:

Use the cleanup module to terminate the system once all operations are complete.

#File Descriptions
airport.c: Handles runway operations and plane communication at airports.
airtrafficcontroller.c: Manages plane-to-airport communication and logs operations.
plane.c: Simulates plane behavior, including passenger and cargo management.
cleanup.c: Ensures proper shutdown of the ATC system.

#Future Enhancements
Add real-time GUI for better visualization.
Extend the system to handle emergency landings.
Improve runway assignment algorithm for efficiency.
