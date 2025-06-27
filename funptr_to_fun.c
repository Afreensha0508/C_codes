#include<stdio.h>

void f1(){
	printf("hello from mirafra\n");
}

void(*getfun())(){
	return f1;
}

int main(){
	void(*fp)()=getfun();
	fp();
	return 0;
}

