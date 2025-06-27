#include <stdio.h>
#include <pthread.h>

#define NUM_ITERATIONS 100000  // Number of increments per thread
#define NUM_THREADS 2          // Number of threads

int sharedCounter = 0;              // Shared global variable
pthread_mutex_t lock;               // Mutex declaration

void* increment(void* arg) {
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        pthread_mutex_lock(&lock);     // Lock before accessing shared data
        sharedCounter++;
        pthread_mutex_unlock(&lock);   // Unlock after done
    }
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];

    pthread_mutex_init(&lock, NULL); // Initialize the mutex

    // Create threads
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, increment, NULL);
    }

    // Wait for all threads to finish
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Final value (with mutex): %d\n", sharedCounter);

    pthread_mutex_destroy(&lock); // Clean up
    return 0;
}


