#include<stdio.h>
#include<sys/ipc.h>
#include<sys/shm.h>

int main(){
	key_t key =ftok("shmfile",65);
	int shmid=shmget(key,1024,0666|IPC_CREAT);
	int *shared=(int*)shmat(shmid,NULL,0);

	shared[0]=2;
	shared[1]=3;
	shared[2]=shared[0]+shared[1];

	printf("%d\n",shared[2]);
	shmdt(shared);
	return 0;
}

