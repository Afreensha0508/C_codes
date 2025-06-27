#include<stdio.h>

int main(){
	unsigned int a=0x12345678;
	unsigned int b;

	b=((a>>24)&0x000000ff)|((a>>8)&0x0000ff00)|((a<<8)&0x00ff0000)|((a<<24)&0xff000000);
	printf("%x\n",b);
	return 0;
}

