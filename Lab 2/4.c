#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

int main()
{

	int a,b;
	printf("Enter a and b\n");
	scanf("%d",&a);
	scanf("%d",&b);

	int id;
	id = fork();

	if(id < 0)
	{
		printf("bailing out\n");
		exit(-10);
	}

	if(id == 0)
	{
		a++;b++;
		int t = a*a - b*b;
		printf("Child 1: <%d>, <%d>\n",getpid(),t);
		//exit(0);
	}
	else
	{
		//wait(NULL);
		int t = a*a - b*b;
		printf("Parent: <%d>, <%d>\n",getpid(),t);
	}
	return 0;
}
		
