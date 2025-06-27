// p2_reader.c
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>

struct SharedData {
    int a;
    int b;
};

int main() {
    key_t key = ftok("shmfile", 65); // generate the same key
    int shmid = shmget(key, sizeof(struct SharedData), 0666); // get existing shared memory
    if (shmid == -1) {
        perror("shmget failed");
        exit(1);
    }

    struct SharedData *data = (struct SharedData *)shmat(shmid, NULL, 0); // attach
    if (data == (void *) -1) {
        perror("shmat failed");
        exit(1);
    }

    int sum = data->a + data->b;
    printf("P2 (Reader): Sum of %d + %d = %d\n", data->a, data->b, sum);

    shmdt(data); // detach
    shmctl(shmid, IPC_RMID, NULL); // remove shared memory
    return 0;
}


