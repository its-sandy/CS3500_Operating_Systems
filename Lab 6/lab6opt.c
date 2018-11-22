#include <pthread.h> 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

int p,c,s,np,nc,in,out,counter, delete_left;		    /* this data is shared by the thread(s) */
int *buffer;
bool possible;

void *runner_p(void *param);
void *runner_c(void *param); 

pthread_mutex_t lock;

int main(int argc, char *argv[])
{
  if(argc != 9)
  {
    fprintf(stderr, "Invalid Number of Inputs\n");
    exit(-1);
  }
  
  srand(time(0));
  int i;
  p = atoi(argv[2]);
  c = atoi(argv[4]);
  s = atoi(argv[6]);
  np = atoi(argv[8]);
  nc = 0.7*np*p/c;
  in = out = counter = 0;
  delete_left = nc*c;
  possible = true;

  buffer = (int *)malloc(s*sizeof(int));

  pthread_t tid_p[p], tid_c[c];
  pthread_attr_t attr;

  pthread_attr_init(&attr);	
  pthread_mutex_init(&lock,NULL); /* create the mutex lock */ 

  int pss[p];
  for(i=0; i<p; i++)
  {
    pss[i] = i+1;
    if (pthread_create(&tid_p[i], &attr, runner_p, &pss[i]) < 0)
    {
      perror("\nUnable to create thread\n"); 
      exit(-1);
    }
  }

  int css[c];
  for(i=0; i<c; i++)
  {
    css[i] = i+1;
    if (pthread_create(&tid_c[i], &attr, runner_c, &css[i]) < 0)
    {
      perror("\nUnable to create thread\n"); 
      exit(-1);
    }
  }

  for(i=0; i<p; i++)
  pthread_join(tid_p[i], NULL);

  for(i=0; i<c; i++)
  pthread_join(tid_c[i], NULL);

  if(!possible)
    printf("Buffer Overflow\n");
  printf("Program over...\n");
  return 0;
}

/* The thread will begin control in this function */ 

void *runner_p(void *param)
{
  int thread_no = *(int *)param, val;
  for(int i = 1; i <= np; i++)
  {
    val = rand()%9000 + 1000;
    while(counter == s)
    {
      if(delete_left == 0 || !possible)
      {
        possible = false;
        pthread_exit(0);
      }
    }    
    
    pthread_mutex_lock(&lock);

      if(counter == s)
      {
        if(delete_left == 0 || !possible)
        {
          possible = false;
          pthread_exit(0);
        }
        i--;
      }
      else
      {
        counter++;
        buffer[in] = val;
        fprintf(stdout,"Producer Thread #: %d; Item #: %d; in value: %d; Added Number: %d\n", thread_no, i, in, val); 
        in = (in+1)%s;
      }

    pthread_mutex_unlock(&lock);
    sleep(rand()%4);
  }
  pthread_exit(0);
}

void *runner_c(void *param)
{
  int thread_no = *(int *)param, val;
  for(int i = 1; i <= nc; i++)
  {
    while(counter == 0);
    
    pthread_mutex_lock(&lock);

      if(counter == 0)
        i--;
      else
      {
        counter--;
        val = buffer[out];
        fprintf(stdout,"Consumer Thread #: %d; Item #: %d; out value: %d; Read Number: %d\n", thread_no, i, out, val); 
        out = (out+1)%s;
        delete_left--;
      }

    pthread_mutex_unlock(&lock);
    sleep(rand()%5);
  }
  pthread_exit(0);
}

