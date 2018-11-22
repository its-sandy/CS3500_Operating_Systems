#include <pthread.h> 
#include <semaphore.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>

int n;
int elves, reindeer, counter;
sem_t santaSem, reindeerSem, elfTex, mutex, elfSem;

void *runner_santa(void *param);
void *runner_elf(void *param);
void *runner_reindeer(void *param);

int main(int argc, char *argv[])
{
	if(argc != 3)
	{
		fprintf(stderr, "Invalid Number of Inputs\n");
		exit(-1);
	}
  
	srand(time(0));
	int i, ind;

	n = atoi(argv[2]);

	// input permutation of 9 reindeer and n elfs
	int perm[n+9];
	for(i=0; i<n+9; i++)
		perm[i] = 0;

	for(i=1; i<=9; i++)
	{
		while(true)
		{
			ind = rand()%(n+9);
			if(perm[ind] == 0)
			{
				perm[ind] = 1;
				break;
			}
		}
	}

	//semaphore initialization
	sem_init(&santaSem, 0, 0);
	sem_init(&reindeerSem, 0, 0);
	sem_init(&elfSem, 0, 0);
	sem_init(&elfTex, 0, 1);
	sem_init(&mutex, 0, 1); 

	pthread_t tid[n+12];
	int tot_elf_count = 0, tot_reindeer_count = 0;
	elves = reindeer = counter = 0;	// current counts

	pthread_attr_t attr;
	pthread_attr_init(&attr); 

	int pss[n+12];
	// create santa
	if (pthread_create(&tid[n+11], &attr, runner_santa, NULL) < 0)
	{
		perror("\nUnable to create thread\n");
		exit(-1);
	}
	nanosleep((const struct timespec[]){{0, 500*(1000000L)}}, NULL);

	for(i=0; i<n+9; i++)
	{
		if(perm[i] == 0)
		{
			//create elf
			tot_elf_count++;
			pss[i] = tot_elf_count;
			if (pthread_create(&tid[i], &attr, runner_elf, &pss[i]) < 0)
		    {
		      perror("\nUnable to create thread\n"); 
		      exit(-1);
		    }
		}	
		else
		{
			//create reindeer
			tot_reindeer_count++;
			pss[i] = tot_reindeer_count;
			if (pthread_create(&tid[i], &attr, runner_reindeer, &pss[i]) < 0)
		    {
		      perror("\nUnable to create thread\n"); 
		      exit(-1);
		    }
		}
	}

	//join santa
	pthread_join(tid[n+11], NULL);

	for(i=0; i<n+9; i++)
	{
		pthread_cancel(tid[i]);
		pthread_join(tid[i], NULL);
	}

	sem_destroy(&santaSem);
	sem_destroy(&reindeerSem);
	sem_destroy(&mutex);
	sem_destroy(&elfTex);

	printf("Program over...\n");
	printf("left out elves = %d\n", elves);
	fflush(stdout);
	return 0;
}

void *runner_santa(void *param)
{
	while(true)
	{
		sem_wait(&santaSem);
		
		sem_wait(&mutex);

			if(reindeer >= 9)
			{
				printf("SANTA prepares sleigh\n");
				fflush(stdout);

				for(int i=1; i<=9; i++)
					sem_post(&reindeerSem);
				reindeer-=9;
			}
			else if(elves >= 3)
			{
				printf("SANTA helps elves\n");
				fflush(stdout);
				for(int i=1; i<=3; i++)
					sem_post(&elfSem);
			}

			if(counter == n+9)
			{
				sem_post(&mutex);
				pthread_exit(0);
			}

		sem_post(&mutex);
	}
	pthread_exit(0);
}

void *runner_reindeer(void *param)
{
	int reindeer_id = *(int *)param;

	sem_wait(&mutex);

		reindeer += 1;
		counter++;
		printf("REINDEER with ID = %d added. Current waiting reindeer count = %d\n", reindeer_id, reindeer);
		fflush(stdout);
		if(reindeer == 9 || counter == n+9)
			sem_post(&santaSem);

	sem_post(&mutex);

	sem_wait(&reindeerSem);

	printf("REINDEER with ID = %d hitched\n", reindeer_id);
	fflush(stdout);

	pthread_exit(0);
}

void *runner_elf(void *param)
{
	int elf_id = *(int *)param;

	sem_wait(&elfTex);
	sem_wait(&mutex);

		elves += 1;
		counter++;
		printf("ELF with ID = %d added. Current waiting elf count = %d\n", elf_id, elves);
		fflush(stdout);

		if(elves == 3 || counter == n+9)
			sem_post(&santaSem);
		else
			sem_post(&elfTex);

	sem_post(&mutex);

	sem_wait(&elfSem);
	printf("ELF with ID = %d gets help\n", elf_id);
	fflush(stdout);

	sem_wait(&mutex);
		elves -=1;
		if(elves == 0)
			sem_post(&elfTex);

	sem_post(&mutex);

	pthread_exit(0);
}