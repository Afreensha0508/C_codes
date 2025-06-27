#include<stdio.h>

int add(int a,int b){
	return a+b;
}
int sub(int a,int b){
	return a-b;
}
int mul(int a,int b){
	return a*b;
}
int div(int a,int b){
	return b?a/b:0;
}

int main(){
        int x=2,y=3;
	int (*fp[4])(int,int)={add,sub,mul,div};
       const char *names[]={"addition","sub","mul","div"};
       for(int i=0;i<4;i++){
	       printf("%s:%d\n",names[i],fp[i](x,y));
       }
       return 0;
}

