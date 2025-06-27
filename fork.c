#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>

int x=3;

int main(){
int y=2;

 printf("Before fork:\n");
    printf("x = %d, address of x = %p (global)\n", x, &x);
    printf("y = %d, address of y = %p (local)\n", y, &y);

	pid_t pid;
      pid =fork();

      if(pid<0){
	      perror("failed\n");
      }
      else if(pid==0){
	      printf("child process\n");
	      printf("pid=%d,ppid=%d\n",getpid(),getppid());
	      printf("x=%d\nx=%p\ny=%d\ny=%p\n",x,&x,y,&y);
      }
      else{
	      printf("parent process\n");
	      printf("pid=%d,child pid=%d\n",getpid(),pid);
	      printf("x=%d\nx=%p\ny=%d\ny=%p\n",x,&x,y,&y);
      }
      return 0;
}

	
