#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

#define NUM_ITERATIONS 100000  // Number of times each thread increments
#define NUM_THREADS 2          // Number of threads

int sharedCounter = 0;         // Shared global variable
sem_t sem;                     // Semaphore

void* increment(void* arg) {
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        sem_wait(&sem);        // Acquire semaphore (lock)
        sharedCounter++;       // Critical section
        sem_post(&sem);        // Release semaphore (unlock)
    }
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];

    // Initialize binary semaphore with value 1
    sem_init(&sem, 0, 1);

    // Create threads
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, increment, NULL);
    }

    // Wait for threads to finish
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Final value (with semaphore): %d\n", sharedCounter);

    // Destroy semaphore
    sem_destroy(&sem);

    return 0;
}

