#include <bits/stdc++.h>
#include <stdio.h>
#include <fstream>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h> 
#include <unistd.h>
#define all(a) (a).begin(), (a).end()
#define pb push_back
#define mp make_pair
#define pii pair<int,int>

using namespace std;

map<int, int> arrivalTime, completionTime;
map<int, int> initLevel;

queue<pii> level1, level4;
priority_queue <pii, vector<pii>, greater<pii> > level2, level3;
struct timeval startTime;
int Q,T,t1;
bool done;
pthread_mutex_t qlock;


void *promoteProcesses(void* param)
{
	int t2;
	struct timeval curTime;
	struct timespec waitTime;
	waitTime.tv_sec = (T/1000);
	waitTime.tv_nsec = (T%1000)*(1000000L);

	while(!done)
	{
		nanosleep(&waitTime, NULL);

		pthread_mutex_lock(&qlock);
		gettimeofday(&curTime,NULL);
		t2 = (curTime.tv_sec *1000000 + curTime.tv_usec)/1000;

		if(!level2.empty() && (t2 - arrivalTime[level2.top().second] > T))
		{
			arrivalTime[level2.top().second] = t2;
			level1.push(level2.top());
			printf("Milliseconds Elapsed : %6d | Process %3d promoted from level 2 to level 1\n", t2-t1, level2.top().second);
			fflush(stdout);
			level2.pop();
		}

		if(!level3.empty() && (t2 - arrivalTime[level3.top().second] > T))
		{
			arrivalTime[level3.top().second] = t2;
			level2.push(level3.top());
			printf("Milliseconds Elapsed : %6d | Process %3d promoted from level 3 to level 2\n", t2-t1, level3.top().second);
			fflush(stdout);
			level3.pop();
		}

		if(!level4.empty() && (t2 - arrivalTime[level4.front().second] > T))
		{
			arrivalTime[level4.front().second] = t2;
			level3.push(level4.front());
			printf("Milliseconds Elapsed : %6d | Process %3d promoted from level 4 to level 3\n", t2-t1, level4.front().second);
			fflush(stdout);
			level4.pop();
		}
		pthread_mutex_unlock(&qlock);
	}
	pthread_exit(0);
}

void* scheduleAndRunProcess(void* param)
{
	pii temp;
	int id, burst, lvl, t2;
	struct timespec waitTime;
	struct timeval curTime;
	waitTime.tv_sec = 0;

	while(!done)
	{
		// find process to run
		pthread_mutex_lock(&qlock);

			if(!level1.empty())
			{
				temp = level1.front();
				level1.pop();
				lvl = 1;
			}
			else if(!level2.empty())
			{
				temp = level2.top();
				level2.pop();
				lvl = 2;
			}
			else if(!level3.empty())
			{
				temp = level3.top();
				level3.pop();
				lvl = 3;
			}
			else if(!level4.empty())
			{
				temp = level4.front();
				level4.pop();
				lvl = 4;
			}
			else
				done = true;

		pthread_mutex_unlock(&qlock);

		if(done)
			break;

		// run scheduled process
		id = temp.second;
		if(lvl == 1)
			burst = min(temp.first, Q);
		else
			burst = temp.first;

		waitTime.tv_nsec = burst*(1000000L);

		gettimeofday(&curTime,NULL);
		t2 = (curTime.tv_sec *1000000 + curTime.tv_usec)/1000;
		printf("Milliseconds Elapsed : %6d | Process %3d scheduled from Level %d for Burst Time = %4dms\n", t2-t1, id, lvl, burst);
		fflush(stdout);

		nanosleep(&waitTime, NULL);

		gettimeofday(&curTime,NULL);
		t2 = (curTime.tv_sec *1000000 + curTime.tv_usec)/1000;

		if(burst == temp.first)
		{
			printf("Milliseconds Elapsed : %6d | Process %3d completed from Level %d\n", t2-t1, id, lvl);
			fflush(stdout);
			completionTime[id] = t2-t1;
		}
		else
		{
			printf("Milliseconds Elapsed : %6d | Process %3d switched out\n", t2-t1, id);
			fflush(stdout);
			pthread_mutex_lock(&qlock);
			level1.push(mp(temp.first - burst, id));	
			pthread_mutex_unlock(&qlock);
		}
	}
	pthread_exit(0);
}
int main(int argc, char** argv)
{
	int id,lev,burst,timeDiff;

	Q = atoi(argv[2]);
	T = atoi(argv[4]);

	string F(argv[6]);
	string P(argv[8]);	

	ifstream ifile(F);
	ofstream ofile(P);

	while(ifile >> id)
	{
		ifile >> lev;
		ifile >> burst;

		switch(lev)
		{
			case 1:	level1.push(mp(burst, id)); break;
			case 2:	level2.push(mp(burst, id)); break;
			case 3:	level3.push(mp(burst, id)); break;
			case 4:	level4.push(mp(burst, id)); break;
		}

		arrivalTime[id] = 0;
		initLevel[id] = lev;
	}
	ifile.close();

	pthread_t tid1, tid2;		/* thread identifier */
	pthread_attr_t attr; 		/* set of thread attributes */
	pthread_attr_init(&attr);
	pthread_mutex_init(&qlock,NULL);

	gettimeofday(&startTime,NULL);
	t1 = (startTime.tv_sec *1000000 + startTime.tv_usec)/1000;
	for (auto it = arrivalTime.begin(); it != arrivalTime.end(); it++)
		arrivalTime[it->first] = t1;

	done = false;
	if (pthread_create(&tid1, &attr, scheduleAndRunProcess, NULL) < 0)
    {
      perror("\nUnable to create thread\n"); 
      exit(-1);
    }

  	if (pthread_create(&tid2, &attr, promoteProcesses, NULL) < 0)
    {
      printf("\nUnable to create thread\n");
      exit(-1);
    }

	pthread_join(tid1, NULL); 
  	pthread_join(tid2, NULL);
  	printf("All processes completed, program terminated\n");

	ofile.close();
	return 0;
}