# Multithread-Broadcasting
Multithread Broadcasting is a C-based multithreaded application designed to simulate a broadcasting system using the producer-consumer model.
This project demonstrates efficient synchronization and communication between multiple threads using POSIX threads (pthread) and semaphores. using concurrent programming concepts, thread synchronization, and inter-thread communication in C.

## C Concepts and Techniques Used
This project demonstrates several key C programming concepts, particularly in the context of multithreading and synchronization:

1. **Multithreading with pthread**:
Utilizes the POSIX pthread library to create and manage multiple threads (e.g., producers, dispatcher, co-editors, screen manager).

2. **Semaphores (sem_t)**:
Employs semaphores for synchronizing access to shared resources, ensuring thread safety across bounded and unbounded queues.

3. **Dynamic Memory Allocation**:
Uses malloc and free to manage memory for queues and articles, ensuring efficient memory usage.

4. **Data Structures (struct)**:
Structures like bounded, producer, and unBounded organize and manage the data for queues and producers.

5. **Producer-Consumer Pattern**:
Implements this pattern to manage the flow of articles from producers to consumers (dispatcher and co-editors).

6. **Round-Robin Scheduling**:
The dispatcher uses a round-robin algorithm to fairly distribute tasks among producers.

7. **File I/O and String Manipulation**:
Handles configuration files and dynamically manages article strings.

## Project Flow
1. **Producers (P1 to Pn)**:
Each producer is responsible for generating articles of different types: SPORTS (S), NEWS (N), and WEATHER (W).
Each producer has its own bounded queue (1 to n) to store the generated articles temporarily.
2. **Bounded Queues (1 to n)**:
Each queue is associated with a specific producer. It stores articles produced by that producer until the Dispatcher thread can process them.
The bounded nature of these queues ensures that the producers cannot overflow the queue, maintaining a controlled flow of data.
3. **Dispatcher Thread**:
The Dispatcher retrieves articles from each producer’s bounded queue in a round-robin manner.
After retrieving an article, the Dispatcher categorizes it into one of three unbounded queues based on the article type: SPORTS (S), NEWS (N), and WEATHER (W).
4. **Unbounded Queues (S, N, W)**:
These queues receive articles from the Dispatcher. Unlike the bounded queues, these queues can grow indefinitely as they store articles until the corresponding Co-Editor processes them.
There are three separate queues for each article type: SPORTS, NEWS, and WEATHER.
5. **Co-Editor Threads**:
Each Co-Editor thread is responsible for processing articles from one specific unbounded queue.
The Co-Editor retrieves articles from its corresponding queue (e.g., the SPORTS Co-Editor handles articles from the SPORTS queue) and forwards them to the Screen Manager.
6. **Screen Manager Thread**:
The Screen Manager receives processed articles from all three Co-Editors.
The articles are then displayed on the Screen in the correct order, ensuring that all content is presented accurately and completely.
7. **Screen**:
The final destination where the processed articles are displayed.
The Screen Manager sends the articles to be printed or visualized on the Screen, concluding the broadcasting flow.

### Summary:
The project operates in a multithreaded environment where producers generate content, and each type of content is handled by a dedicated flow through bounded and unbounded queues, with Co-Editors and a Screen Manager ensuring synchronized and orderly output on the Screen. This setup simulates a real-world broadcasting system with multiple producers and categorized content management.

![image](https://github.com/user-attachments/assets/126987e5-ca39-44b4-b326-49251d210841)

## Getting Started:
### Clone the Repository:
```bash
git clone https://github.com/tomerp1812/Multithread-Broadcasting.git
cd multithread-broadcasting
```

### Compile the Code with makefile:
```bash
make
```

### Prepare Configuration File:
Create a configuration file specifying the index of each producer (1,2,3...), number of products each producer should generate (10,8,12...), their respective queue size(5,7,9...)
and Co-Editor queue size(15).
Example configuration:
```bash
1
10
5

2
8
7

3
12
9

15
```

### Run the Application:

```bash
./broadcaster config.txt
```
