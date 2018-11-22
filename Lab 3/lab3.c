#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAXCOM 1000
#define MAXARG 100

int main()
{
	printf("Welcome to Mini-Shell!\n");
	char com[MAXCOM];
	char* ptr;
	char* arg[MAXARG];

	int i, pid, fg;

	while(1)
	{
		printf("Command> ");
		fgets(com, MAXCOM, stdin);

		ptr = strtok(com, " \n");
		i = 0;
		while(ptr != NULL)
		{
			arg[i++] = ptr;
			ptr = strtok(NULL, " \n");
		}
		arg[i] = NULL;

		if(i == 0)
			continue;

		if(i != 0 && strcmp(arg[i-1], "&") == 0)
		{
			fg = 0;
			arg[i-1] = NULL;
		}
		else
			fg = 1;

		if(strcmp(arg[0], "exit") == 0 || strcmp(arg[0], "quit") == 0)
		{
			printf("Mini-Shell Terminating...Goodbye!\n");
			break;
		}

		pid = fork();
		if(pid < 0)
		{
			printf("Error: child not created\nMini-Shell Terminating...Goodbye!\n");
			break;
		}
		else if(pid == 0)
		{
			//child
			if (i != 0 && execvp(arg[0], arg) < 0)
            	printf("Could not execute command..\n");
			exit(0);
		}
		else
		{
			//parent
			if(fg)
				waitpid(pid, NULL, 0);
		}
	}
	return 0;
}