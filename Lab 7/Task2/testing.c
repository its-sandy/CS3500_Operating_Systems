#include <pthread.h> 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

int M,S,D,Z,C,B,U,E,H,R,N,num_iterations;
int *shelves;
int makers_left, students_left;
int num_filled_shelves;

void *runner_p(void *param);
void *runner_c(void *param); 

pthread_mutex_t *lock;
pthread_mutex_t maker_lock, student_lock, shelf_lock;

int main(int argc, char *argv[])
{
  if(argc != 23)
  {
    fprintf(stderr, "Invalid Number of Inputs\n");
    exit(-1);
  }
  
  int i;
  M = atoi(argv[2]);  // no. of makers
  S = atoi(argv[4]);  // no. of shelves
  D = atoi(argv[6]);  // shelf capacity
  Z = atoi(argv[8]);  // maker nap time
  C = atoi(argv[10]); // time to make pizza
  B = atoi(argv[12]); // pizzas per batch
  U = atoi(argv[14]); // no. of students
  E = atoi(argv[16]); // time to eat pizza
  H = atoi(argv[18]); // pizzas eaten per session
  R = atoi(argv[20]); // student satisfied time
  N = atoi(argv[22]); // seed
  num_iterations = 20;
  srand(N);
  
  if(M<5 || B<10 || U<7)
  {
    fprintf(stderr, "Variables don't meet required constraints\n");
    exit(-1);
  }

  M = rand()%(M-4) + 5;
  U = rand()%(U-6) + 7;
  printf("Number of Makers (M) = %d\nNumber of Students (U) = %d\n\n",M,U);

  shelves = (int *)malloc(S*sizeof(int));
  lock = (pthread_mutex_t *)malloc(S*sizeof(pthread_mutex_t));
  for(i=0; i<S; i++)
  {
    shelves[i] = 0;
    pthread_mutex_init(&lock[i],NULL);  //initialize locks
  }

  makers_left = M;
  students_left = U;
  num_filled_shelves = 0;
  pthread_t tid_p[M], tid_c[U];
  pthread_attr_t attr;
  pthread_attr_init(&attr);

  int pss[M];
  for(i=0; i<M; i++)
  {
    pss[i] = i+1;
    if (pthread_create(&tid_p[i], &attr, runner_p, &pss[i]) < 0)
    {
      perror("\nUnable to create thread\n"); 
      exit(-1);
    }
  }

  int css[U];
  for(i=0; i<U; i++)
  {
    css[i] = i+1;
    if (pthread_create(&tid_c[i], &attr, runner_c, &css[i]) < 0)
    {
      perror("\nUnable to create thread\n"); 
      exit(-1);
    }
  }

  for(i=0; i<M; i++)
  pthread_join(tid_p[i], NULL);

  for(i=0; i<U; i++)
  pthread_join(tid_c[i], NULL);

  printf("Program over...\n");
  return 0;
}

/* The thread will begin control in this function */ 

void *runner_p(void *param)
{
  bool possible = true;
  int thread_no = *(int *)param, i, j;
  char filename[30];
  char pizzaid[30];
  sprintf(filename, "out_maker_%d.txt", thread_no);
  FILE* fptr = fopen(filename, "w");
  fprintf(fptr, "Maker %d Log\n", thread_no);
  fflush(fptr);

  int state = rand()%2, shelf, curcount, batchno = 0;
  for(i = 1; i <= 2*num_iterations && possible; i++)
  {
    printf("0\n");
    fflush(stdout);

    if(state == 1)
    {
      fprintf(fptr, "maker %d entering NAP state\n", thread_no);
      fflush(fptr);
      nanosleep((const struct timespec[]){{0, Z*(1000000L)}}, NULL);
      state = 0;
    }
    else if(state == 0)
    {
      fprintf(fptr, "maker %d entering MAKING PIZZA BATCH state\n", thread_no);
      fflush(fptr);
      state = 1;
      shelf = -1;
      curcount = 0;
      batchno++;
      for(j=1; j<=B && possible; j++)
      {
        sprintf(pizzaid, "maker_%d_batch_%d_pizza_%d", thread_no, batchno, j);
        nanosleep((const struct timespec[]){{0, C*(1000000L)}}, NULL);
        fprintf(fptr, "Pizza %s produced\n",pizzaid);
        fflush(fptr);

        while(possible)
        {
          if(students_left == 0 && num_filled_shelves == S)
          {
            fprintf(fptr, "maker %d exitting as no more space left\n",thread_no);
            fflush(fptr);
            possible = false;
            break;
          }
          if(shelf == -1 || curcount == 10)
          {
            shelf = rand()%S;
            curcount = 0;
            fprintf(fptr, "maker %d Waiting for shelf %d to be free\n",thread_no,shelf+1);
            fflush(fptr);
          }
          pthread_mutex_lock(&lock[shelf]);

          if(shelves[shelf] < D)
          {
            shelves[shelf]++;
            curcount++;
            fprintf(fptr, "Pizza %s placed on shelf %d\n",pizzaid,shelf+1);
            fflush(fptr);

            if(shelves[shelf] == D)
            {
              pthread_mutex_lock(&shelf_lock);
              num_filled_shelves++;
              pthread_mutex_unlock(&shelf_lock);
            }

            pthread_mutex_unlock(&lock[shelf]);
            break;
          }
          else
            curcount = 10;

          pthread_mutex_unlock(&lock[shelf]);
        }
      }
    }
  }

  fprintf(fptr, "Maker %d exitting\n", thread_no);
  fflush(fptr);

  fclose(fptr);
  pthread_mutex_lock(&maker_lock);
    makers_left--;
  pthread_mutex_unlock(&maker_lock);
  pthread_exit(0);
}

void *runner_c(void *param)
{
  bool possible = true;
  int thread_no = *(int *)param, i, j, k;
  char filename[30];
  char pizzaid[30];
  sprintf(filename, "out_student_%d.txt", thread_no);
  FILE* fptr = fopen(filename, "w");
  fprintf(fptr, "Student %d Log\n", thread_no);
  fflush(fptr);

  int state = 0, shelf, curcount=0;
  for(i = 1; i <= 2*num_iterations && possible; i++)
  {
    printf("1\n");
    fflush(stdout);

    if(state == 1)
    {
      fprintf(fptr, "student %d entering STUDYING state\n", thread_no);
      fflush(fptr);
      nanosleep((const struct timespec[]){{0, R*(1000000L)}}, NULL);
      state = 0;
    }
    else if(state == 0)
    {
      fprintf(fptr, "student %d entering HUNGRY state\n", thread_no);
      fflush(fptr);
      state = 1;
      for(j=1; j<=H && possible; j++)
      {
        curcount++;
        sprintf(pizzaid, "student_%d_pizza_%d", thread_no, curcount);

        k=1;
        while(possible)
        {
          if(k<=3)
          {
            shelf = rand()%S;
            fprintf(fptr, "student %d Waiting for pizza to be added to shelf %d\n",thread_no, shelf+1);
            fflush(fptr);
          }
          else
          {
            if(makers_left == 0)
            {
              possible = false;
              fprintf(fptr, "student %d exitting as no more makers left and shelf %d is empty\n",thread_no, shelf+1);
              break;
            }
          }

          pthread_mutex_lock(&lock[shelf]);
          if(shelves[shelf] > 0)
          {
            if(shelves[shelf] == D)
            {
              pthread_mutex_lock(&shelf_lock);
              num_filled_shelves--;
              pthread_mutex_unlock(&shelf_lock);
            }
            shelves[shelf]--;
            fprintf(fptr, "Pizza %s removed from shelf %d\n", pizzaid, shelf+1);
            fflush(fptr);
            pthread_mutex_unlock(&lock[shelf]);
            break;
          }
          pthread_mutex_unlock(&lock[shelf]);
          if(k<=3)
            k++;
        }
        if(!possible)
          break;
        nanosleep((const struct timespec[]){{0, E*(1000000L)}}, NULL);
        fprintf(fptr, "Pizza %s eaten\n", pizzaid);
        fflush(fptr);
      }
    }
  }
  fprintf(fptr, "Student %d exitting\n", thread_no);
  fflush(fptr);

  fclose(fptr);
  pthread_mutex_lock(&student_lock);
    students_left--;
  pthread_mutex_unlock(&student_lock);

  pthread_exit(0);
}