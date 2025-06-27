#include<stdio.h>

void add(int a, int b){
	printf("sum:%d\n",a+b);
}

void op(void(*fun)(int,int),int x,int y){
	fun(x,y);
}

int main(){
	op(add,5,10);
	return 0;
}

