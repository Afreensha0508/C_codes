#include<stdio.h>
#include<sys/ipc.h>
#include<sys/shm.h>

int main(){
	key_t key=ftok("shmfile",65);

	int shmid=shmget(key,1024,0666);
	int *shared=(int*)shmat(shmid,NULL,0);

	printf("%d\n",shared[2]);
	
	shmdt(shared);
	shmctl(shmid,IPC_RMID,NULL);
	return 0;
}


