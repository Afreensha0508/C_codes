#include<stdio.h>
 
int main(){
	int n=10,k=1;
n|=(1<<k);
printf("set:%d\n",n);
n&=~(1<<k);
printf("clear:%d\n",n);
n^=(1<<k);
printf("toggle:%d\n",n);

return 0;
}

