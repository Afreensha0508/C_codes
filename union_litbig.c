#include<stdio.h>

union check{
	int x;
	char bytes[4];
};

int main(){
	union check a;
	a.x=1;

	if(a.bytes[0]==1)
		printf("little endian\n");
	else
		printf("big endian\n");
	return 0;
}

