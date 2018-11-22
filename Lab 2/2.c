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
	else if(id > 0)
	{
		wait(NULL);
		printf("Parent exitting\n");
	}
	else
	{
		a = a*a;
		id = fork();
		
		if(id < 0)
		{
			printf("bailing out\n");
			exit(-10);
		}

		if(id == 0)
		{
			b = b*b;
			FILE* fout = fopen("blabla.bin","w");
			fwrite(&b, sizeof(int), 1, fout);
			fclose(fout);		
		}
		else
		{
			wait(NULL);
			FILE* fin = fopen("blabla.bin","r");
			fread(&b, sizeof(int), 1, fin);
			fclose(fin);
			remove("blabla.bin");
			int t = a - b;
		printf("Child 1: <%d>, <%d>\n",getpid(),t);
		}
	}
	return 0;
}
		
