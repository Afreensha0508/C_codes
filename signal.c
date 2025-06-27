#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

void handle_sigint(int sig){
	printf("\n %d\n",sig);
	exit(0);
}
int main(){
	 signal(SIGINT,handle_sigint);
	while(1){
		printf("woeking\n");
		sleep(1);

	}
	return 0;
}

