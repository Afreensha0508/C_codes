#include<stdio.h>
#include<unistd.h>
#include<sys/wait.h>
int main(){
	int a=10,b=5,c[3];
int pipefd[2];
pipe(pipefd); // create pipe 
pid_t pid=fork(); // create child process 
if(pid==0){
	c[0]=a+b;
	c[1]=a-b;
	c[2]=a*b;

	write(pipefd[1],c,sizeof(c));// send to parent 
	close(pipefd[1]);
}
else{

	wait(NULL); // wait for child
	read(pipefd[0],c,sizeof(c));
	close(pipefd[0]);
        printf("result give from child\n");
	printf("add:%d\nsub;%d\nmul:%d\n",c[0],c[1],c[2]);
}
return 0;
}

