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
		int t = a*a - b*b;
		printf("Child 1: <%d>, <%d>\n",getpid(),t);
		//exit(0);
	}
	else
	{
		id = fork();

		if(id < 0)
		{
			printf("bailing out\n");
			exit(-10);
		}

		if(id == 0)
		{
			int t = (a-b)*(a+b);
			printf("Child 2: <%d>, <%d>\n",getpid(),t);
			//exit(0);
		}
		else
		{
			int status;
			while(wait(&status) > 0);
			printf("Parent exitting\n");
		}
	}
	return 0;
}
		
