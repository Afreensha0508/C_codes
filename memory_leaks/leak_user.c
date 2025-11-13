#include<stdio.h>
#include<stdlib.h>


int *leaky_func(size_t n){
	int *p=malloc(n * sizeof(int));
	if(!p)
		return NULL;
	p[0]=42;
	return p;  // forgot free-> memory leak
}

// memory leak and here we get errors 
/*int main(){
	int *buf1=leaky_func(1000),*buf2=malloc(256);
	if(buf1)
		printf("buf1[0]=%d\n",buf1[0]);
	return 0;  // forgot free(buf) and free(buf2) 
}*/



int main(){
	int *buf1=leaky_func(1000),*buf2=malloc(256);
	if(buf1)
		printf("buf1[0]=%d\n",buf1[0]);

	// here we r freeing this 2 so that 2 errors r clear 
	free(buf1);
	free(buf2);

	return 0;
}
