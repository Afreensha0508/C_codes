#include<stdio.h>
#include<stdlib.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<unistd.h>


#defined SHM_KEY 12345

// Shared structure
struct SharedData {
    int a,b,c; // 1 = data ready, 0 = data consumed
};

int main() {
    int shmid;
    struct SharedData *data;

    // Create shared memory
    shmid = shmget(SHM_KEY, sizeof(struct SharedData), IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget");
        exit(1);
    }

    // Attach shared memory
    data = (struct SharedData *)shmat(shmid, NULL, 0);
    if (data == (void *) -1) {
        perror("shmat");
        exit(1);
    }

    while (1) {
        if (data->c == 0) {
            printf("Enter a: ");
            scanf("%d", &data->a);
            printf("Enter b: ");
            scanf("%d", &data->b);

            data->c = 1; // signal that data is ready
        }
        sleep(1); // avoid busy waiting
    }

    shmdt(data);
    return 0;
}




