#include<stdio.h>

int main(){
	int a=0x1234;
	a=(a&0xf000)>>12|(a&0x0f00)>>4|(a&0x00f0)<<4|(a&0x000f)<<12;
	printf("%x\n",a);
	return 0;
}

