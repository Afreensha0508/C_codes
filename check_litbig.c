#include<stdio.h>
int main(){
	unsigned int n=1;
	char *ptr=(char *)&n;

	if(*ptr==1)
		printf("little endian\n");
	else
		printf("big endian\n");
	return 0;
}
