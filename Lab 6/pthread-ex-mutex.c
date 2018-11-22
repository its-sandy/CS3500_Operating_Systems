#include <pthread.h> 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define COUNTER 10
int sum;		    /* this data is shared by the thread(s) */
void *runner1(void *param), *runner2(void *param); 
/* threads call this function */ 
pthread_mutex_t lock;

int main(int argc, char *argv[])
{
  pthread_t tid1, tid2;		/* thread identifier */
  pthread_attr_t attr; 		/* set of thread attributes */

  sum = 0;
  /* get the default attributes */
  pthread_attr_init(&attr);	

  /* create the mutex lock */ 
  pthread_mutex_init(&lock,NULL);

  /* create the threads */
  if (pthread_create(&tid1, &attr, runner1, NULL) < 0)
    {
      perror("\nUnable to create thread\n"); 
      exit(-1);
    }

  if (pthread_create(&tid2, &attr, runner2, NULL) < 0)
    {
      printf("\nUnable to create thread\n");
      exit(-1);
    }

  /* wait for the threads to exit */
  pthread_join(tid1, NULL); 
  pthread_join(tid2, NULL); 

  printf("sum = %d\n",sum);
}

/* The thread will begin control in this function */ 

void *runner1(void *param)
{
  for (int i = 0; i < COUNTER; i++)
    {
      pthread_mutex_lock(&lock);
      sum++;
      fprintf(stdout,"Child Increment Loop No: %d; sum: %d\n", i, sum); 
      pthread_mutex_unlock(&lock);
      sleep(random()%3);
    }
  
  pthread_exit(0);
}

void *runner2(void *param)
{
  for (int i = 0; i < COUNTER; i++)
    {
      pthread_mutex_lock(&lock);  
      sum--;
      fprintf(stdout,"\t\t\t\tChild Decrement Loop No: %d; sum: %d\n", i, sum); 
      pthread_mutex_unlock(&lock);
      sleep(random()%4);      

    }
  
  pthread_exit(0);
}

