#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>

void handler(int sig){
	printf("signal %d received\n",aig);
}

int main(){
	signal(SIGINT.handler);
	sigset_t block_Set;
	sigemptyset(&block_Set);
	sigaddset(&block_Set,SIGINT);

	printf("blockng SIGINT for 5sec\n"):
		sigprocmask(SIG_BLOCK,&block_Set,NULL);
	sleep(5);

	printf("unblocking try to cntrl+c \n");
	 sigprocmask(SIG_UNBLOCK,&unblock_Set,NULL);

	 while(1){
		 pause();
	 }
}




