#include <bits/stdc++.h>
#include <stdio.h>
#include <fstream>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#define all(a) (a).begin(), (a).end()
#define pb push_back
#define mp make_pair
#define pii pair<int,int>

using namespace std;

map<int, int> arrivalTime, completionTime;
map<int, int> initLevel;

queue<pii> level1, level4;
priority_queue <pii, vector<pii>, greater<pii> > level2, level3;
struct timeval startTime, curTime;
int Q,T,t1,t2;
pii temp;
bool done;
struct timespec waitTime;


void promoteProcesses()
{
	gettimeofday(&curTime,NULL);
	t2 = (curTime.tv_sec *1000000 + curTime.tv_usec)/1000;

	if(!level2.empty() && t2 - arrivalTime[level2.top().second] > T)
	{
		arrivalTime[level2.top().second] = t2;
		level1.push(level2.top());
		printf("Milliseconds Elapsed : %6d | Process %3d promoted from level 2 to level 1\n", t2-t1, level2.top().second);
		level2.pop();
	}

	if(!level3.empty() && t2 - arrivalTime[level3.top().second] > T)
	{
		arrivalTime[level3.top().second] = t2;
		level2.push(level3.top());
		printf("Milliseconds Elapsed : %6d | Process %3d promoted from level 3 to level 2\n", t2-t1, level3.top().second);
		level3.pop();
	}

	if(!level4.empty() && t2 - arrivalTime[level4.front().second] > T)
	{
		arrivalTime[level4.front().second] = t2;
		level3.push(level4.front());
		printf("Milliseconds Elapsed : %6d | Process %3d promoted from level 4 to level 3\n", t2-t1, level4.front().second);
		level4.pop();
	}
}

bool scheduleAndRunProcess()
{
	gettimeofday(&curTime,NULL);
	t2 = (curTime.tv_sec *1000000 + curTime.tv_usec)/1000;
	int id, burst;

	if(!level1.empty())
	{
		temp = level1.front();
		level1.pop();

		id = temp.second;
		burst = min(temp.first, Q);
		waitTime.tv_nsec = burst*(1000000L);
		printf("Milliseconds Elapsed : %6d | Process %3d scheduled from Level 1 for Burst Time = %4dms\n", t2-t1, id, burst);
		nanosleep(&waitTime, NULL);
		if(burst == temp.first)
		{
			gettimeofday(&curTime,NULL);
			t2 = (curTime.tv_sec *1000000 + curTime.tv_usec)/1000;	
			printf("Milliseconds Elapsed : %6d | Process %3d completed\n", t2-t1, id);
		}
		else
		{
			gettimeofday(&curTime,NULL);
			t2 = (curTime.tv_sec *1000000 + curTime.tv_usec)/1000;	
			printf("Milliseconds Elapsed : %6d | Process %3d swithced out\n", t2-t1, id);
			level1.push(mp(temp.first - burst, id));	
		}
	}
	else if(!level2.empty())
	{
		temp = level2.top();
		level2.pop();

		id = temp.second;
		burst = temp.first;
		waitTime.tv_nsec = burst*(1000000L);
		printf("Milliseconds Elapsed : %6d | Process %3d scheduled from Level 2 for Burst Time = %4dms\n", t2-t1, id, burst);
		
		nanosleep(&waitTime, NULL);

		gettimeofday(&curTime,NULL);
		t2 = (curTime.tv_sec *1000000 + curTime.tv_usec)/1000;	
		printf("Milliseconds Elapsed : %6d | Process %3d completed\n", t2-t1, id);
	}
	else if(!level3.empty())
	{
		temp = level3.top();
		level3.pop();

		id = temp.second;
		burst = temp.first;
		waitTime.tv_nsec = burst*(1000000L);
		printf("Milliseconds Elapsed : %6d | Process %3d scheduled from Level 3 for Burst Time = %4dms\n", t2-t1, id, burst);
		
		nanosleep(&waitTime, NULL);

		gettimeofday(&curTime,NULL);
		t2 = (curTime.tv_sec *1000000 + curTime.tv_usec)/1000;	
		printf("Milliseconds Elapsed : %6d | Process %3d completed\n", t2-t1, id);
	}
	else if(!level4.empty())
	{
		temp = level4.front();
		level4.pop();

		id = temp.second;
		burst = temp.first;
		waitTime.tv_nsec = burst*(1000000L);
		printf("Milliseconds Elapsed : %6d | Process %3d scheduled from Level 4 for Burst Time = %4dms\n", t2-t1, id, burst);
		
		nanosleep(&waitTime, NULL);

		gettimeofday(&curTime,NULL);
		t2 = (curTime.tv_sec *1000000 + curTime.tv_usec)/1000;	
		printf("Milliseconds Elapsed : %6d | Process %3d completed\n", t2-t1, id);
	}
	else
		done = true;
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

	gettimeofday(&startTime,NULL);
	t1 = (startTime.tv_sec *1000000 + startTime.tv_usec)/1000;
	for (auto it = arrivalTime.begin(); it != arrivalTime.end(); it++)
		arrivalTime[it->first] = t1;

	done = false;
	waitTime.tv_sec = 0;
	while(!done)
	{
		promoteProcesses();
		scheduleAndRunProcess();	
	}
	printf("All processes completed, program terminated\n");
	ifile.close();
	ofile.close();
	return 0;
}