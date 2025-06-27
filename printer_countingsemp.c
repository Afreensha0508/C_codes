#include<stdio.h>
#include<pthread.h>
#include<semaphore.h>
#include<unistd.h>

#define NUM_USERS 5
#define NUM_PRINT 2

sem_t printer;

void* userthread(void* arg){
	int userId=*((int*)arg);

	printf("user %d: waiting to use a printer\n",userId);

	sem_wait(&printer);
	printf("user %d: printing any data\n",userId);

	sleep(2);

	printf("user %d: complete task\n",userId);
	sem_post(&printer);
	return NULL;
}

int main(){
	pthread_t users[NUM_USERS];
	int userIds[NUM_USERS];

	sem_init(&printer,0,NUM_PRINT);

	for(int i=0;i<NUM_USERS;i++){
		userIds[i]=i+1;
		pthread_create(&users[i],NULL,userthread,&userIds[i]);
		sleep(1);
	}

	for(int i=0;i<NUM_USERS;i++){
		pthread_join(users[i],NULL);
	}

	sem_destroy(&printer);
	return 0;
}

