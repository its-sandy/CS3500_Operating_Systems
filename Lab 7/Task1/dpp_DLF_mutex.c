// compile as gcc -o dpp_DLF dpp_DLF.c -lpthread
#include <pthread.h> 
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>

int n;        /* this data is shared by the thread(s) */
struct timeval startTime;
void *runner(void *param);

sem_t* chopsticks;
sem_t seeman;

int main(int argc, char *argv[])
{
  if(argc != 3)
  {
    fprintf(stderr, "Invalid Number of Inputs\n");
    exit(-1);
  }
  
  srand(time(0));
  int i;

  n = atoi(argv[2]);
  chopsticks = (sem_t *)malloc(n*sizeof(sem_t));

  pthread_t tid[n];
  pthread_attr_t attr;
  pthread_attr_init(&attr);

  for(i=0; i<n; i++)
  sem_init(&chopsticks[i], 0, 1); /* create the mutex lock */
  sem_init(&seeman, 0, 1); 

  int pss[n];
  gettimeofday(&startTime,NULL);
  for(i=0; i<n; i++)
  {
    pss[i] = i;
    if (pthread_create(&tid[i], &attr, runner, &pss[i]) < 0)
    {
      perror("\nUnable to create thread\n"); 
      exit(-1);
    }
  }

  for(i=0; i<n; i++)
  pthread_join(tid[i], NULL);
  
  for(i=0; i<n; i++)
    sem_destroy(&chopsticks[i]);
  sem_destroy(&seeman);

  //printf("Program over...\n");
  return 0;
}

/* The thread will begin control in this function */ 

void *runner(void *param)
{
  struct timeval curTime;
  int thread_no = *(int *)param,i,n1,n2;
  long int val;

  char filename[30];
  sprintf(filename, "dp_out_thread_%d_DLF_mutex.txt", thread_no+1);
  FILE* fptr = fopen(filename, "w");

  for(i = 1; i <= 100; i++)
  {
    val = rand()%51 + 50;
    gettimeofday(&curTime,NULL);
    fprintf(fptr, "milliseconds elapsed:%7ld #  Round: %3d #  entering THINKING state\n", ((curTime.tv_sec-startTime.tv_sec)*1000000 + curTime.tv_usec-startTime.tv_usec)/1000, i);
    fflush(fptr);
    printf("milliseconds elapsed:%7ld #  Round: %3d #  Philosopher %d entering THINKING state\n", ((curTime.tv_sec-startTime.tv_sec)*1000000 + curTime.tv_usec-startTime.tv_usec)/1000, i, thread_no+1);
    fflush(stdout);
    nanosleep((const struct timespec[]){{0, val*(1000000L)}}, NULL);  
    
    gettimeofday(&curTime,NULL);
    fprintf(fptr, "milliseconds elapsed:%7ld #  Round: %3d #  entering HUNGRY state\n", ((curTime.tv_sec-startTime.tv_sec)*1000000 + curTime.tv_usec-startTime.tv_usec)/1000, i);
    fflush(fptr);
    printf("milliseconds elapsed:%7ld #  Round: %3d #  Philosopher %d entering HUNGRY state\n", ((curTime.tv_sec-startTime.tv_sec)*1000000 + curTime.tv_usec-startTime.tv_usec)/1000, i, thread_no+1);
    fflush(stdout);

    while(1)
    {
      sem_wait(&seeman); 
        sem_getvalue(&chopsticks[thread_no], &n1);
        sem_getvalue(&chopsticks[(thread_no+1)%n], &n2);

        if(n1>0 && n2>0)
        {
          sem_wait(&chopsticks[thread_no]);
          // nanosleep((const struct timespec[]){{0, 100*(1000000L)}}, NULL);
          sem_wait(&chopsticks[(thread_no+1)%n]);
          sem_post(&seeman);
          break;
        }
        sem_post(&seeman);
    }

    val = rand()%101 + 100;
    gettimeofday(&curTime,NULL);
    fprintf(fptr, "milliseconds elapsed:%7ld #  Round: %3d #  entering EATING state\n", ((curTime.tv_sec-startTime.tv_sec)*1000000 + curTime.tv_usec-startTime.tv_usec)/1000, i);
    fflush(fptr);
    printf("milliseconds elapsed:%7ld #  Round: %3d #  Philosopher %d entering EATING state\n", ((curTime.tv_sec-startTime.tv_sec)*1000000 + curTime.tv_usec-startTime.tv_usec)/1000, i, thread_no+1);
    fflush(stdout);
    nanosleep((const struct timespec[]){{0, val*(1000000L)}}, NULL);

    sem_post(&chopsticks[thread_no]);
    sem_post(&chopsticks[(thread_no+1)%n]);

    gettimeofday(&curTime,NULL);
    fprintf(fptr, "milliseconds elapsed:%7ld #  Round: %3d #  entering LEAVING state\n", ((curTime.tv_sec-startTime.tv_sec)*1000000 + curTime.tv_usec-startTime.tv_usec)/1000, i);
    fflush(fptr);
    printf("milliseconds elapsed:%7ld #  Round: %3d #  Philosopher %d entering LEAVING state\n", ((curTime.tv_sec-startTime.tv_sec)*1000000 + curTime.tv_usec-startTime.tv_usec)/1000, i, thread_no+1);
    fflush(stdout);
  }

  fclose(fptr);
  pthread_exit(0);
}