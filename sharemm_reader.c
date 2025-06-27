#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SHM_KEY 1234
#define SHM_SIZE 1024

int main() {
    // Get existing shared memory segment
    int shmid = shmget(SHM_KEY, SHM_SIZE, 0666);
    if (shmid == -1) {
        perror("shmget failed");
        return 1;
    }

    // Attach
    char *data = (char *)shmat(shmid, NULL, 0);
    if (data == (char *)-1) {
        perror("shmat failed");
        return 1;
    }

    printf("Reader: Shared memory contains: %s\n", data);

    // Detach and optionally remove
    shmdt(data);
    shmctl(shmid, IPC_RMID, NULL);  // remove shared memory
    return 0;
}

