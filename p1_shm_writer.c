// p1_writer.c
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>

struct SharedData {
    int a;
    int b;
};

int main() {
    key_t key = ftok("shmfile", 65); // generate unique key
    int shmid = shmget(key, sizeof(struct SharedData), 0666 | IPC_CREAT); // create shared memory
    if (shmid == -1) {
        perror("shmget failed");
        exit(1);
    }

    struct SharedData *data = (struct SharedData *)shmat(shmid, NULL, 0); // attach to shared memory
    if (data == (void *) -1) {
        perror("shmat failed");
        exit(1);
    }

    // Take input
    printf("P1 (Writer): Enter value a: ");
    scanf("%d", &data->a);
    printf("P1 (Writer): Enter value b: ");
    scanf("%d", &data->b);

    printf("P1 (Writer): Data written to shared memory.\n");

    shmdt(data); // detach from shared memory
    return 0;
}

