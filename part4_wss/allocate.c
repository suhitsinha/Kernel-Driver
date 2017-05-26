#include<stdio.h>
#include<unistd.h>
#include<malloc.h>

int main()
{
	int i,k;
	printf("My process ID : %d\n", getpid());

	int *ptr = (int *) malloc(sizeof(int)* 1024*10);
	while(1)
	{
		for(i=0; i<1024*10; i++)
		{
			*(ptr+i) = *(ptr+i) * 1;
		}
		printf("Enter a int value to proceed:\n");
		scanf("%d",&k);
	}
}
